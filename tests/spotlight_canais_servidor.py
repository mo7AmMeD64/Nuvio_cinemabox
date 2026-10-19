# Addon Stremio FALSO de canais para tests/spotlight_canais.sh: manifesto,
# catalogo "tv" com tres canais, fonte de um canal e a playlist dele.
# Canais e nomes inventados; nada de conta passa por aqui.
import http.server, socketserver, sys, json
PORTA = int(sys.argv[1])
BASE = 'http://127.0.0.1:%d' % PORTA
MANIFESTO = {"id": "teste.canais", "name": "Canais Teste", "version": "1.0.0",
             "resources": ["catalog", "stream"], "types": ["tv"], "idPrefixes": ["teste:"],
             "catalogs": [{"type": "tv", "id": "canais", "name": "Canais"}]}
CANAIS = [
  {"id": "teste:espn", "type": "tv", "name": "ESPN Brasil", "genres": ["Esportes"]},
  {"id": "teste:globo", "type": "tv", "name": "Globo RJ", "genres": ["Abertos"]},
  {"id": "teste:cnn", "type": "tv", "name": "CNN Brasil", "genres": ["Notícias"]},
]
PLAYLIST = "#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-TARGETDURATION:4\n#EXTINF:4.0,\nseg0.ts\n"
class H(http.server.BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def manda(self, corpo, tipo='application/json'):
        b = corpo.encode()
        self.send_response(200); self.send_header('Content-Type', tipo)
        self.send_header('Content-Length', str(len(b))); self.end_headers(); self.wfile.write(b)
    def do_GET(self):
        p = self.path
        sys.stderr.write('[servidor] GET %s\n' % p)
        if p == '/manifest.json': return self.manda(json.dumps(MANIFESTO, ensure_ascii=False))
        if p.startswith('/catalog/'):
            if 'skip=' in p: return self.manda('{"metas":[]}')
            return self.manda(json.dumps({"metas": CANAIS}, ensure_ascii=False))
        if p.startswith('/stream/'):
            if 'teste:espn' in p or 'teste%3Aespn' in p:
                return self.manda(json.dumps({"streams": [{"name": "Canais Teste", "title": "HD",
                                                           "url": BASE + "/ao-vivo/espn.m3u8"}]}))
            return self.manda('{"streams":[]}')
        if p == '/ao-vivo/espn.m3u8': return self.manda(PLAYLIST, 'application/vnd.apple.mpegurl')
        self.send_response(404); self.end_headers()
class S(socketserver.ThreadingMixIn, http.server.HTTPServer): daemon_threads = True
S(('127.0.0.1', PORTA), H).serve_forever()
