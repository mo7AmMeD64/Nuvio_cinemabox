package space.nuvio.nativelegacy

import java.nio.ByteBuffer

// SINCRONIA DE LEGENDA POR AUDIO (F06, 1.8): copia o PCM DECODIFICADO que o
// ExoPlayer entrega ao AudioSink, em mono 16 kHz int16 com o tempo de midia da
// primeira amostra, para o C (src/audsync.c). Kotlin puro (nenhum androidx):
// tests/audiosync_android.py roda esta classe na JVM com buffers sinteticos.
//
// REGRAS
//  * So copia enquanto `ligado` (o C liga por escolher(3, 1) quando a pessoa
//    pede "Por audio"); desligado custa uma leitura de @Volatile por buffer.
//  * Nao aloca por buffer: le o ByteBuffer por indice absoluto (posicao e
//    limite do buffer do player ficam intactos) e escreve num ShortArray fixo.
//  * Formato codificado (passthrough AC3/E-AC3/DTS/TrueHD ou offload) nunca
//    chega aqui como PCM: `configurar(pcm = false)` informa BITSTREAM e nada e
//    copiado. O tap NAO desliga passthrough/offload para conseguir o audio.
//  * Mistura todos os canais (media) e reamostra por media de caixa (fase
//    fracionaria): suficiente para detectar fala, nao para ouvir.
class AudioSyncTap(
    private val entrega: (ShortArray, Int, Long) -> Unit,
    private val estado: (Int) -> Unit,
) {
    companion object {
        const val FMT_NENHUM = 0
        const val FMT_PCM = 1
        const val FMT_BITSTREAM = 2
        const val ENC_PCM8 = 1
        const val ENC_PCM16 = 2
        const val ENC_PCM24 = 3
        const val ENC_PCM32 = 4
        const val ENC_FLOAT = 5
        const val SAIDA_HZ = 16000
        const val CAPACIDADE = 2048       // == jshort b[2048] em video_android.c
    }

    @Volatile var ligado = false
    private val saida = ShortArray(CAPACIDADE)
    private var pcm = false
    private var codificacao = 0
    private var canais = 0
    private var taxa = 0
    private var bytesQuadro = 0
    // reamostragem: soma/contagem da amostra de saida em curso e a fase
    private var soma = 0.0
    private var conta = 0
    private var fase = 0L
    private var ptsCaixa = 0L        // media time of the first input frame of the current box
    private var ultimoPts = Long.MIN_VALUE
    private var ultimoBuffer: ByteBuffer? = null

    // Fio de reproducao (AudioSink.configure). Formato sem PCM = bitstream.
    fun configurar(ehPcm: Boolean, enc: Int, nCanais: Int, hz: Int) {
        val bps = when (enc) { ENC_PCM8 -> 1; ENC_PCM16 -> 2; ENC_PCM24 -> 3; ENC_PCM32, ENC_FLOAT -> 4; else -> 0 }
        pcm = ehPcm && bps > 0 && nCanais > 0 && hz > 0
        codificacao = enc; canais = nCanais; taxa = hz; bytesQuadro = bps * nCanais
        descontinuidade()
        estado(if (pcm) FMT_PCM else if (ehPcm) FMT_NENHUM else FMT_BITSTREAM)
    }

    // flush/seek: a proxima amostra nao continua a anterior.
    fun descontinuidade() {
        soma = 0.0; conta = 0; fase = 0L
        ultimoPts = Long.MIN_VALUE; ultimoBuffer = null
    }

    fun encerrar() {
        pcm = false
        descontinuidade()
        estado(FMT_NENHUM)
    }

    // PCM do Android e little-endian (ENCODING_PCM_*). Le byte a byte para nao
    // depender nem mexer na ByteOrder do buffer do player.
    private fun u8(b: ByteBuffer, i: Int) = b.get(i).toInt() and 0xff
    private fun le32(b: ByteBuffer, i: Int) = u8(b, i) or (u8(b, i + 1) shl 8) or (u8(b, i + 2) shl 16) or (b.get(i + 3).toInt() shl 24)
    private fun amostra(b: ByteBuffer, i: Int): Double = when (codificacao) {
        ENC_PCM16 -> (u8(b, i) or (b.get(i + 1).toInt() shl 8)) / 32768.0
        ENC_FLOAT -> java.lang.Float.intBitsToFloat(le32(b, i)).toDouble()
        ENC_PCM8 -> (u8(b, i) - 128) / 128.0
        ENC_PCM24 -> (u8(b, i) or (u8(b, i + 1) shl 8) or (b.get(i + 2).toInt() shl 16)) / 8388608.0
        ENC_PCM32 -> le32(b, i) / 2147483648.0
        else -> 0.0
    }

    // AudioSink.handleBuffer: o MESMO buffer volta enquanto o sink nao o
    // consumiu inteiro; so a primeira vista de cada (buffer, pts) e copiada.
    fun buffer(b: ByteBuffer, ptsUs: Long) {
        if (!ligado || !pcm || taxa <= 0) return
        if (b === ultimoBuffer && ptsUs == ultimoPts) return
        ultimoBuffer = b; ultimoPts = ptsUs
        val ini = b.position()
        val quadros = (b.limit() - ini) / bytesQuadro
        var n = 0
        var ptsSaida = 0L
        for (q in 0 until quadros) {
            val base = ini + q * bytesQuadro
            var m = 0.0
            for (c in 0 until canais) m += amostra(b, base + c * (bytesQuadro / canais))
            if (conta == 0) ptsCaixa = ptsUs + (q.toLong() * 1_000_000L) / taxa
            soma += m / canais; conta++
            fase += SAIDA_HZ
            if (fase >= taxa) {
                fase -= taxa
                if (n == 0) ptsSaida = ptsCaixa
                val v = (soma / conta * 32767.0).coerceIn(-32768.0, 32767.0)
                saida[n++] = v.toInt().toShort()
                soma = 0.0; conta = 0
                if (n == CAPACIDADE) { entrega(saida, n, ptsSaida); n = 0 }
            }
        }
        if (n > 0) entrega(saida, n, ptsSaida)
    }
}
