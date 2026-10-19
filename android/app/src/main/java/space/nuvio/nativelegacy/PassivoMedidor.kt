package space.nuvio.nativelegacy

// TELEMETRIA PASSIVA DO STREAMFIT (F03): quanto o host FINAL da fonte entrega
// enquanto o player toca, sem pedido de rede novo.
//
// O QUE CONTA. Bytes de midia lidos do socket pelas transferencias do proprio
// ParaleloDataSource (os pedacos paralelos e a conexao unica), somados por
// SEGUNDO DE RELOGIO: as quatro conexoes que baixam ao mesmo tempo caem no
// mesmo balde e somam uma vez — nunca a soma das taxas de cada pedaco medidas
// em janelas diferentes. Um segundo so vale se houve transferencia ativa em
// pelo menos 90% dele (COBERTURA_MS) e o player estava tocando de verdade
// (playWhenReady, em READY/BUFFERING, depois do primeiro READY desta sessao).
// Stall = transferencia ativa sem byte: vale como zero, e e justamente o
// trecho ruim que a conta precisa ver.
//
// O QUE NAO CONTA. A entrega ao ExoPlayer de pedacos ja baixados (memoria,
// nao rede: por isso nao e um TransferListener no read(), que mediria o
// consumo do player e nao a rede); bytes de cache; pausa; o ocioso de buffer
// cheio (nenhuma conexao ativa = cobertura baixa = segundo descartado);
// manifesto/segmento de HLS/DASH (conexao unica do Media3, nao passa aqui);
// e o comeco da sessao antes de o extrator aceitar os bytes como midia.
//
// ENTREGA. A janela mais recente (5..48 segundos validos) do mesmo host, da
// mesma rede (epoch do NuvioActivity) e da mesma sessao do player. Troca de
// host, de rede ou de sessao descarta o que havia. O C confere de novo a
// geracao e a rede (streamfitpassiva.c) e so aceita a sessao que e a fonte
// real do player. Sem URL: so "esquema://autoridade".
//
// Kotlin puro (sem android.*): o teste roda na JVM com relogio falso.
class PassivoMedidor(
    private val relogio: () -> Long,     // monotonico, ms
    private val parede: () -> Long,      // relogio civil, ms (mesma base do C)
    private val redeAtual: () -> Long,   // epoch conhecido ou 0
    private val entregar: (Entrega) -> Unit,
) {
    class Entrega(val rede: Long, val geracao: Int, val origem: String,
                  val kbps: IntArray, val fimMs: Long)

    companion object {
        const val MAX = 48
        const val MIN = 5
        const val BALDE_MS = 1000L
        const val COBERTURA_MS = 900L
        const val ENTREGA_CADA = 10
        private const val KBPS_MAX = 10_000_000L

        // Publicado pelo NuvioActivity junto com nativeRedeAlterou.
        @JvmStatic @Volatile var redeGlobal = 0L

        // "https://host[:porta]" sem usuario, caminho ou query.
        @JvmStatic fun origemDe(u: java.net.URL?): String? {
            if (u == null) return null
            val p = u.protocol?.lowercase() ?: return null
            if (p != "http" && p != "https") return null
            val h = u.host?.lowercase() ?: return null
            if (h.isEmpty()) return null
            val porta = if (u.port < 0 || u.port == u.defaultPort) "" else ":${u.port}"
            return "$p://$h$porta"
        }
    }

    private val trava = Any()
    private var token = 0L
    private var geracao = 0
    private var rede = 0L
    private var tocando = false
    private var midiaValida = false
    private var origem: String? = null
    private var ativos = 0
    private var baldeIni = -1L
    private var baldeBytes = 0L
    private var baldeAtivoMs = 0L
    private var baldeSujo = true
    private var ultimo = 0L
    private val serie = IntArray(MAX)
    private var n = 0
    private var novos = 0
    private var entregou = false

    // Sessao nova do player (abrirMain). Devolve o token que as transferencias
    // desta sessao carregam; eventos de outro token sao ignorados.
    fun sessao(geracaoNative: Int): Long = synchronized(trava) {
        token++
        geracao = geracaoNative
        tocando = false; midiaValida = false
        origem = null; ativos = 0
        rede = redeAtual()
        zerarSerie(); baldeIni = -1L
        token
    }

    fun encerrar() = synchronized(trava) {
        token++; geracao = 0; ativos = 0; zerarSerie(); baldeIni = -1L
    }

    // Fio principal: estado do player desta sessao.
    fun estado(tok: Long, tocandoAgora: Boolean, valida: Boolean) {
        val e = synchronized(trava) {
            if (tok != token) return
            val agora = relogio()
            val pronta = rolar(agora)
            if (!tocandoAgora || !valida) baldeSujo = true
            tocando = tocandoAgora; midiaValida = valida
            // A second that starts exactly now carries only the new state.
            if (baldeIni == agora && baldeBytes == 0L && baldeAtivoMs == 0L)
                baldeSujo = !(tocando && midiaValida) || rede == 0L
            pronta
        }
        e?.let(entregar)
    }

    fun inicio(tok: Long) {
        val e = synchronized(trava) {
            if (tok != token) return
            val pronta = rolar(relogio())
            ativos++
            pronta
        }
        e?.let(entregar)
    }

    fun bytes(tok: Long, de: String?, qtd: Int) {
        if (qtd <= 0) return
        val e = synchronized(trava) {
            if (tok != token) return
            val pronta = rolar(relogio())
            if (de == null) { baldeSujo = true; return@synchronized pronta }
            if (de != origem) {
                // Outro host final: a janela anterior nao e deste servidor.
                if (origem != null) { zerarSerie(); baldeSujo = true }
                origem = de
            }
            baldeBytes += qtd
            pronta
        }
        e?.let(entregar)
    }

    fun fim(tok: Long) {
        val e = synchronized(trava) {
            if (tok != token) return
            val pronta = rolar(relogio())
            if (ativos > 0) ativos--
            pronta
        }
        e?.let(entregar)
    }

    private fun zerarSerie() { n = 0; novos = 0; entregou = false }

    // Fecha os baldes vencidos ate `agora`. Chamado sob a trava.
    private fun rolar(agora: Long): Entrega? {
        val r = redeAtual()
        if (r != rede) {
            // Outra rede: nada do que havia vale, nem o balde aberto.
            rede = r; zerarSerie(); baldeSujo = true
        }
        if (baldeIni < 0) { abrirBalde(agora); return null }
        var pronta: Entrega? = null
        var voltas = 0
        while (true) {
            val fimBalde = baldeIni + BALDE_MS
            val ate = minOf(agora, fimBalde)
            if (ativos > 0 && ate > ultimo) baldeAtivoMs += ate - ultimo
            if (ate > ultimo) ultimo = ate
            if (agora < fimBalde) break
            fechar()?.let { pronta = it }
            baldeIni = fimBalde
            baldeBytes = 0; baldeAtivoMs = 0
            baldeSujo = !(tocando && midiaValida) || rede == 0L
            // Ocioso longo sem transferencia: pula direto, sem iterar segundos.
            if (ativos == 0 && agora - baldeIni > BALDE_MS) { abrirBalde(agora); break }
            if (++voltas > MAX + 2) { abrirBalde(agora); break }
        }
        return pronta
    }

    private fun abrirBalde(agora: Long) {
        baldeIni = agora; ultimo = agora
        baldeBytes = 0; baldeAtivoMs = 0
        baldeSujo = !(tocando && midiaValida) || rede == 0L
    }

    private fun fechar(): Entrega? {
        if (baldeSujo || baldeAtivoMs < COBERTURA_MS || origem == null) return null
        val kbps = baldeBytes * 8 / BALDE_MS   // bits/ms == kbps
        if (kbps > KBPS_MAX) return null
        if (n == MAX) { System.arraycopy(serie, 1, serie, 0, MAX - 1); n-- }
        serie[n++] = kbps.toInt()
        novos++
        if (n < MIN || (entregou && novos < ENTREGA_CADA) || geracao == 0 || rede == 0L) return null
        novos = 0; entregou = true
        return Entrega(rede, geracao, origem!!, serie.copyOf(n), parede())
    }
}
