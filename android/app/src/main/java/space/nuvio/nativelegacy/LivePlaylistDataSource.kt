package space.nuvio.nativelegacy

import android.net.Uri
import android.os.SystemClock
import androidx.media3.common.C
import androidx.media3.common.util.UnstableApi
import androidx.media3.datasource.DataSource
import androidx.media3.datasource.DataSpec
import androidx.media3.datasource.TransferListener
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.io.IOException

/** Revalidate stale ordinary HLS manifests without touching the player or its sample queues.
 * Never rewrite sequence numbers, segment URLs or timestamps. Media3 remains responsible for
 * parsing, ordering and genuine origin resets. LL-HLS, ranges, VOD and media bytes pass through.
 */
@androidx.annotation.OptIn(UnstableApi::class)
internal class LivePlaylistDataSource private constructor(
    private val upstream: DataSource,
    private val factory: Factory,
) : DataSource {
    internal data class Snapshot(val sequence: Long, val count: Int, val targetMs: Long) {
        val end: Long get() = sequence + count
    }

    class Factory(
        private val upstream: DataSource.Factory,
        // Separate short-timeout HTTP factory; keep the same authentication/user-agent.
        private val revalidation: DataSource.Factory,
        private val clock: () -> Long = { SystemClock.elapsedRealtime() },
        private val report: (String) -> Unit = {},
    ) : DataSource.Factory {
        private data class Seen(var snapshot: Snapshot, var progressAt: Long, var retryAt: Long = -60_000L)
        private val seen = LinkedHashMap<String, Seen>()
        private var budgetAt = -60_000L
        private var retries = 0
        override fun createDataSource(): DataSource = LivePlaylistDataSource(upstream.createDataSource(), this)

        @Synchronized private fun needsRefresh(key: String, next: Snapshot): Boolean {
            val now = clock()
            val old = seen[key]
            if (old == null) {
                if (seen.size >= 16) seen.remove(seen.keys.first())
                seen[key] = Seen(next, now)
                return false
            }
            if (next.sequence >= old.snapshot.sequence && next.end > old.snapshot.end) {
                old.snapshot = next
                old.progressAt = now
                return false
            }
            // One unchanged poll is normal. Allow at least 1.5 target durations.
            val reset = next.end < old.snapshot.sequence
            val stale = now - old.progressAt >= (old.snapshot.targetMs * 3 / 2).coerceIn(6_000L, 60_000L)
            if (!reset && !stale) return false
            if (now - old.retryAt < 15_000L) return false
            if (now - budgetAt >= 60_000L) { budgetAt = now; retries = 0 }
            if (retries >= 2) return false
            old.retryAt = now
            retries++
            return true
        }

        private fun refresh(spec: DataSpec, original: ByteArray, snapshot: Snapshot, resolved: Uri?): ByteArray {
            if (!needsRefresh(spec.uri.toString(), snapshot)) return original
            val requestHeaders = spec.httpRequestHeaders.filterKeys {
                !it.equals("If-None-Match", true) && !it.equals("If-Modified-Since", true) &&
                    !it.equals("Cache-Control", true) && !it.equals("Pragma", true)
            } + mapOf("Cache-Control" to "no-cache, max-age=0", "Pragma" to "no-cache")
            val source = revalidation.createDataSource()
            val started = clock()
            try {
                // Keep the exact URI (including signatures), headers and request context.
                source.open(spec.buildUpon().setHttpRequestHeaders(requestHeaders).build())
                val bytes = readBounded(source) {
                    if (clock() - started > 3_000L) throw IOException("Playlist revalidation deadline")
                }
                val fresh = parse(bytes)
                if (source.uri == resolved && fresh != null && fresh.sequence >= snapshot.sequence && fresh.end > snapshot.end) {
                    synchronized(this) {
                        seen[spec.uri.toString()]?.let {
                            if (fresh.sequence >= it.snapshot.sequence && fresh.end > it.snapshot.end) {
                                it.snapshot = fresh
                                it.progressAt = clock()
                            }
                        }
                    }
                    report("[live-playlist] revalidated advanced=true elapsedMs=${clock() - started}")
                    return bytes
                }
                report("[live-playlist] revalidated advanced=false elapsedMs=${clock() - started}")
            } catch (_: IOException) {
                // A failed optimization must not turn a successful normal request into an error.
                report("[live-playlist] revalidation unavailable elapsedMs=${clock() - started}")
            } finally {
                try { source.close() } catch (_: IOException) { }
            }
            return original
        }

        internal fun process(spec: DataSpec, bytes: ByteArray, resolved: Uri?): ByteArray {
            val snapshot = parse(bytes) ?: return bytes
            return refresh(spec, bytes, snapshot, resolved)
        }
    }

    private var prefix: ByteArrayInputStream? = null
    private var upstreamOpen = false
    private var resolvedUri: Uri? = null
    private var headers: Map<String, List<String>> = emptyMap()

    override fun addTransferListener(listener: TransferListener) = upstream.addTransferListener(listener)
    override fun getUri(): Uri? = resolvedUri
    override fun getResponseHeaders(): Map<String, List<String>> = headers
    override fun open(dataSpec: DataSpec): Long {
        upstreamOpen = true
        val length = upstream.open(dataSpec)
        resolvedUri = upstream.uri
        headers = upstream.responseHeaders
        val http = dataSpec.uri.scheme.equals("http", true) || dataSpec.uri.scheme.equals("https", true)
        val manifest = dataSpec.uri.path?.endsWith(".m3u8", true) == true || headers.any { (key, values) ->
            key.equals("Content-Type", true) && values.any {
                it.substringBefore(';').trim().lowercase() in setOf("application/vnd.apple.mpegurl", "application/x-mpegurl", "audio/mpegurl", "audio/x-mpegurl")
            }
        }
        if (!http || !manifest || dataSpec.httpMethod != DataSpec.HTTP_METHOD_GET ||
            dataSpec.position != 0L || dataSpec.length != C.LENGTH_UNSET.toLong()) return length
        val bytes = readBounded(upstream)
        if (bytes.size > MAX_MANIFEST_BYTES) {
            // Oversize manifests remain streaming; neither truncate nor keep an unbounded copy.
            prefix = ByteArrayInputStream(bytes)
            return length
        }
        upstream.close()
        upstreamOpen = false
        val result = factory.process(dataSpec, bytes, resolvedUri)
        prefix = ByteArrayInputStream(result)
        return result.size.toLong()
    }

    override fun read(buffer: ByteArray, offset: Int, length: Int): Int {
        if (length == 0) return 0
        prefix?.let {
            val read = it.read(buffer, offset, length)
            if (read >= 0) return read
            prefix = null
        }
        return if (upstreamOpen) upstream.read(buffer, offset, length) else C.RESULT_END_OF_INPUT
    }

    override fun close() {
        prefix = null
        resolvedUri = null
        headers = emptyMap()
        if (upstreamOpen) { upstreamOpen = false; upstream.close() }
    }

    companion object {
        private const val MAX_MANIFEST_BYTES = 256 * 1024
        private fun readBounded(source: DataSource, check: () -> Unit = {}): ByteArray {
            val out = ByteArrayOutputStream()
            val chunk = ByteArray(4096)
            while (out.size() <= MAX_MANIFEST_BYTES) {
                check()
                val count = source.read(chunk, 0, minOf(chunk.size, MAX_MANIFEST_BYTES + 1 - out.size()))
                if (count == C.RESULT_END_OF_INPUT) break
                out.write(chunk, 0, count)
            }
            return out.toByteArray()
        }

        // Only a refresh hint; the bytes are still parsed and validated by Media3 unchanged.
        private fun parse(bytes: ByteArray): Snapshot? {
            if (bytes.size > MAX_MANIFEST_BYTES) return null
            val text = bytes.toString(Charsets.UTF_8)
            if (!text.startsWith("#EXTM3U") || text.contains("#EXT-X-ENDLIST") ||
                text.contains("#EXT-X-PLAYLIST-TYPE:VOD") || text.contains("#EXT-X-PART") ||
                text.contains("#EXT-X-SKIP") || text.contains("CAN-BLOCK-RELOAD=YES")) return null
            var sequence = 0L
            var target = 0L
            var count = 0
            for (raw in text.lineSequence()) {
                val line = raw.trim()
                when {
                    line.startsWith("#EXT-X-MEDIA-SEQUENCE:") -> sequence = line.substringAfter(':').toLongOrNull() ?: return null
                    line.startsWith("#EXT-X-TARGETDURATION:") -> target = line.substringAfter(':').toLongOrNull() ?: return null
                    line.startsWith("#EXTINF:") -> count++
                }
            }
            if (sequence < 0 || target !in 1..60 || count == 0 || sequence > Long.MAX_VALUE - count) return null
            return Snapshot(sequence, count, target * 1000)
        }
    }
}
