package space.nuvio.nativelegacy

import kotlin.math.abs
import kotlin.math.log10
import kotlin.math.roundToInt
import kotlin.math.tanh

// F07 (1.8): VOLUME 0-200% PER SESSION, the math. Pure Kotlin (JVM test in
// tests/cacheboost_android.py runs this exact file).
//
// 0..100% is the player's own volume (ExoPlayer.volume, the AudioTrack gain,
// works for every output). 100..200% is a real gain on the decoded PCM, in
// GanhoAudioProcessor: 200% of amplitude = +6.02 dB. It only exists while the
// audio is PCM; with passthrough/bitstream the samples never reach the app and
// the boost is unavailable (the C side caps the row at 100% and says why).
//
// CLIPPING: multiplying loud material by up to 2 would clip hard. Above the
// knee (-1 dBFS) the boosted signal goes through a tanh soft limiter: it is
// continuous and has slope 1 at the knee, it is monotonic, and it approaches
// full scale asymptotically (never beyond it: no wrap, no hard clip). Below
// the knee the gain is exactly linear, and at 100% the samples pass untouched
// (no limiter, no rounding).
object GanhoMath {
    const val MAX_PCT = 200
    const val JOELHO = 0.8912509f          // -1 dBFS

    fun pct(p: Int) = p.coerceIn(0, MAX_PCT)
    // ExoPlayer.volume (0..1).
    fun volumePlayer(p: Int): Float = minOf(100, pct(p)) / 100f
    // The processor's gain (1..2).
    fun reforco(p: Int): Float = maxOf(100, pct(p)) / 100f
    // 20*log10(p/100) in dB (LoudnessEnhancer would take millibels = dB*100).
    fun decibeis(p: Int): Double = if (pct(p) <= 0) -120.0 else 20.0 * log10(pct(p) / 100.0)

    // One normalized sample (-1..1) times g >= 1, soft-limited above the knee.
    fun amostra(x: Float, g: Float): Float {
        if (g <= 1f) return x
        val y = x * g
        val a = abs(y)
        if (a <= JOELHO) return y
        val faixa = 1f - JOELHO
        val c = JOELHO + faixa * tanh((a - JOELHO) / faixa)
        return if (y < 0f) -c else c
    }

    fun pcm16(s: Short, g: Float): Short {
        if (g <= 1f) return s
        val v = (amostra(s / 32768f, g) * 32768f).roundToInt()
        return v.coerceIn(-32768, 32767).toShort()
    }
}
