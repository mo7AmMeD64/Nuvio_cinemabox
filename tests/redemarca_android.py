"""Compile the actual Android callback block with SDK 35; replay its lifecycle
with JVM network doubles. No Gradle/app build, downloads, device or network I/O.
Requires the locally cached Kotlin compiler used by this project's Android build.
"""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parent.parent
src = (root / "android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt").read_text()
block = src[src.index("    private external fun nativeRedeAlterou"):src.index("    // Chamado pelo C (android_pedir_superficie)")]
cache = Path.home() / ".gradle/caches/modules-2/files-2.1"
compiler = list(cache.glob("org.jetbrains.kotlin/*/2.0.21/*/*.jar"))
compiler += list(cache.glob("org.jetbrains.intellij.deps/trove4j/*/*/*.jar"))
compiler += list(cache.glob("org.jetbrains/annotations/13.0/*/*.jar"))
compiler += list(cache.glob("org.jetbrains.kotlinx/kotlinx-coroutines-core-jvm/*/*/*.jar"))
stdlib = next(cache.glob("org.jetbrains.kotlin/kotlin-stdlib/2.0.21/*/*.jar"))
sdk = Path(os.environ.get("ANDROID_HOME", str(Path.home() / "Library/Android/sdk"))) / "platforms/android-35/android.jar"
assert sdk.is_file() and any("compiler-embeddable" in str(x) for x in compiler)
cp = os.pathsep.join(map(str, compiler))
java = os.environ.get("NUVIO_TEST_JAVA") or str(next((Path.home() / ".local/jdks").glob("jdk-17*/Contents/Home/bin/java")))

def compile_kotlin(path, out, classpath):
    subprocess.run([java, "-cp", cp, "org.jetbrains.kotlin.cli.jvm.K2JVMCompiler",
                    "-no-reflect", "-no-stdlib", "-classpath", classpath,
                    "-d", str(out), str(path)], check=True)

with tempfile.TemporaryDirectory(prefix="nuvio-network-epoch-") as tmp:
    tmp = Path(tmp)
    sdk_src = tmp / "Sdk.kt"
    sdk_src.write_text("""import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.LinkProperties
object PassivoMedidor { @JvmStatic @Volatile var redeGlobal = 0L }
class NuvioActivity {
    fun getSystemService(name: String): Any? = null
""" + block + "}\n")
    compile_kotlin(sdk_src, tmp / "sdk", os.pathsep.join((str(sdk), str(stdlib))))
    test_block = block.replace("private external fun nativeRedeAlterou(sequencia: Long, conhecida: Boolean)",
                               "private fun nativeRedeAlterou(sequencia: Long, conhecida: Boolean) { saida.add(sequencia to conhecida) }")
    replay = tmp / "Replay.kt"
    replay.write_text("""
object Context { const val CONNECTIVITY_SERVICE = "connectivity" }
object PassivoMedidor { @JvmStatic @Volatile var redeGlobal = 0L }
data class Network(val id: Int)
data class LinkProperties(val key: Int) { constructor(other: LinkProperties): this(other.key) }
class NetworkCapabilities(val mask: Int, val transport: Int = 1, val bandwidth: Int = 10) {
    companion object {
        const val NET_CAPABILITY_INTERNET = 0
        const val NET_CAPABILITY_VALIDATED = 1
        const val NET_CAPABILITY_NOT_METERED = 2
        const val NET_CAPABILITY_NOT_VPN = 3
    }
    fun hasCapability(i: Int) = mask and (1 shl i) != 0
    fun hasTransport(i: Int) = transport and (1 shl i) != 0
}
class ConnectivityManager {
    open class NetworkCallback {
        open fun onAvailable(network: Network) {}
        open fun onCapabilitiesChanged(network: Network, cap: NetworkCapabilities) {}
        open fun onLinkPropertiesChanged(network: Network, link: LinkProperties) {}
        open fun onBlockedStatusChanged(network: Network, blocked: Boolean) {}
        open fun onLost(network: Network) {}
    }
    var callback: NetworkCallback? = null
    var removals = 0
    var fail = false
    fun registerDefaultNetworkCallback(cb: NetworkCallback) {
        if (fail) throw SecurityException()
        callback = cb
    }
    fun unregisterNetworkCallback(cb: NetworkCallback) { check(cb === callback); removals++ }
}
class NuvioActivity {
    val cm = ConnectivityManager()
    val saida = mutableListOf<Pair<Long,Boolean>>()
    fun getSystemService(name: String): Any? = cm
    fun start() = observarRede()
    fun close() = fecharRede()
""" + test_block + """
}
fun main() {
    val a = NuvioActivity(); a.start()
    val cb = a.cm.callback!!; val first = Network(1); val second = Network(2)
    cb.onAvailable(first); check(!a.saida.last().second)
    cb.onCapabilitiesChanged(first, NetworkCapabilities(15)); check(!a.saida.last().second)
    cb.onLinkPropertiesChanged(first, LinkProperties(1)); check(a.saida.last().second)
    val ready = a.saida.last().first
    check(PassivoMedidor.redeGlobal == ready) // passive meter shares the epoch
    cb.onCapabilitiesChanged(first, NetworkCapabilities(15, bandwidth=9999))
    cb.onLinkPropertiesChanged(first, LinkProperties(1)); check(a.saida.last().first == ready)
    cb.onLinkPropertiesChanged(first, LinkProperties(2)); check(a.saida.last().first > ready)
    cb.onBlockedStatusChanged(first, true); check(!a.saida.last().second)
    cb.onBlockedStatusChanged(first, false); check(a.saida.last().second)
    cb.onCapabilitiesChanged(first, NetworkCapabilities(1)); check(!a.saida.last().second)
    cb.onCapabilitiesChanged(first, NetworkCapabilities(15)); check(a.saida.last().second)
    cb.onAvailable(second); check(!a.saida.last().second)
    val switched = a.saida.size
    cb.onLost(first); cb.onLinkPropertiesChanged(first, LinkProperties(3))
    cb.onCapabilitiesChanged(first, NetworkCapabilities(15)); check(a.saida.size == switched)
    cb.onLinkPropertiesChanged(second, LinkProperties(2)); check(!a.saida.last().second)
    cb.onCapabilitiesChanged(second, NetworkCapabilities(15)); check(a.saida.last().second)
    cb.onLost(second); check(!a.saida.last().second && PassivoMedidor.redeGlobal == 0L)
    cb.onAvailable(first); cb.onCapabilitiesChanged(first, NetworkCapabilities(15))
    cb.onLinkPropertiesChanged(first, LinkProperties(1)); check(a.saida.last().first > ready)
    a.close(); check(!a.saida.last().second && a.cm.removals == 1)
    val closed = a.saida.size
    cb.onAvailable(first); cb.onCapabilitiesChanged(first, NetworkCapabilities(15))
    cb.onLinkPropertiesChanged(first, LinkProperties(1)); check(a.saida.size == closed)
    val failed = NuvioActivity(); failed.cm.fail = true; failed.start()
    check(!failed.saida.last().second)
    println("redemarca_android: PASS (actual callback SDK syntax, route/link/caps/blocked changes, old network, unregister, failure)")
}
""")
    compile_kotlin(replay, tmp / "replay", str(stdlib))
    subprocess.run([java, "-cp", os.pathsep.join((str(tmp / "replay"), str(stdlib))), "ReplayKt"], check=True)
