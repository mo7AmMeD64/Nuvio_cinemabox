package space.nuvio.nativelegacy

import androidx.media3.common.C
import androidx.media3.common.Format
import androidx.media3.common.MimeTypes
import androidx.media3.common.util.UnstableApi
import androidx.media3.exoplayer.audio.AudioSink
import androidx.media3.exoplayer.audio.ForwardingAudioSink
import java.nio.ByteBuffer

// F06: o AudioSink de sempre (DefaultAudioSink do DefaultRenderersFactory) com
// uma escuta na ENTRADA. handleBuffer traz o tempo de midia de cada buffer,
// coisa que o AudioProcessor/TeeAudioProcessor do Media3 1.8 nao da.
// Nao muda formato, capacidades, passthrough, offload nem tunneling: so le.
@androidx.annotation.OptIn(UnstableApi::class)
class AudioSyncSink(sink: AudioSink, private val tap: AudioSyncTap) : ForwardingAudioSink(sink) {
    override fun configure(inputFormat: Format, specifiedBufferSize: Int, outputChannels: IntArray?) {
        val pcm = MimeTypes.AUDIO_RAW == inputFormat.sampleMimeType
        val enc = when (inputFormat.pcmEncoding) {
            C.ENCODING_PCM_16BIT -> AudioSyncTap.ENC_PCM16
            C.ENCODING_PCM_FLOAT -> AudioSyncTap.ENC_FLOAT
            C.ENCODING_PCM_8BIT -> AudioSyncTap.ENC_PCM8
            C.ENCODING_PCM_24BIT -> AudioSyncTap.ENC_PCM24
            C.ENCODING_PCM_32BIT -> AudioSyncTap.ENC_PCM32
            else -> 0
        }
        tap.configurar(pcm, enc, inputFormat.channelCount, inputFormat.sampleRate)
        super.configure(inputFormat, specifiedBufferSize, outputChannels)
    }

    override fun handleBuffer(buffer: ByteBuffer, presentationTimeUs: Long, encodedAccessUnitCount: Int): Boolean {
        tap.buffer(buffer, presentationTimeUs)
        return super.handleBuffer(buffer, presentationTimeUs, encodedAccessUnitCount)
    }

    override fun flush() {
        tap.descontinuidade()
        super.flush()
    }
}
