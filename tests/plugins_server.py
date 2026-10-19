#!/usr/bin/env python3
"""Servidor HTTP local para os testes dos plugins (F09). Nunca sai da maquina.

  /ok                 JSON pequeno
  /lento?ms=N         espera N ms antes de responder (corpo pequeno)
  /pinga?ms=N         manda cabecalhos e o corpo devagar (1 byte a cada 100 ms)
  /grande?mb=N        corpo de N MiB (Content-Length declarado)
  /grande-chunk?mb=N  corpo de N MiB sem Content-Length (chunked)
  /html?n=N           HTML com N elementos <div><a href=...>
  /redir              302 para /ok
  /manifest           manifest.json de repositorio com 2 scrapers
  /manifest-vazio     manifest sem scrapers
  /prov/<nome>.js     codigo de scraper (ver CODIGOS)
Escreve a porta no arquivo do primeiro argumento.
"""
import sys, time, threading
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

CODIGOS = {
    "bom": """
async function getStreams(tmdb, tipo, t, e) {
  const r = await fetch(BASE + '/ok');
  const j = await r.json();
  return [{ name: 'Bom', title: 'Bom ' + j.v + ' ' + tmdb + ' ' + tipo, url: 'https://exemplo.org/bom.m3u8', quality: '1080p' }];
}
module.exports = { getStreams };
""",
    "lento": """
async function getStreams() {
  await fetch(BASE + '/lento?ms=4000');
  return [{ name: 'Lento', url: 'https://exemplo.org/lento.m3u8' }];
}
module.exports = { getStreams };
""",
}


class H(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def log_message(self, *a):
        pass

    def corpo(self, b, tipo="application/json", st=200):
        self.send_response(st)
        self.send_header("Content-Type", tipo)
        self.send_header("Content-Length", str(len(b)))
        self.end_headers()
        self.wfile.write(b)

    def do_GET(self):
        u = urlparse(self.path)
        q = parse_qs(u.query)
        p = u.path
        base = "http://127.0.0.1:%d" % self.server.server_address[1]
        try:
            if p == "/ok":
                return self.corpo(b'{"v":42}')
            if p == "/lento":
                time.sleep(int(q.get("ms", ["1000"])[0]) / 1000.0)
                return self.corpo(b'{"v":1}')
            if p == "/pinga":
                self.send_response(200)
                self.send_header("Content-Type", "text/plain")
                self.send_header("Content-Length", "100")
                self.end_headers()
                for _ in range(100):
                    self.wfile.write(b"x"); self.wfile.flush(); time.sleep(0.1)
                return
            if p == "/grande":
                n = int(float(q.get("mb", ["1"])[0]) * 1024 * 1024)
                self.send_response(200)
                self.send_header("Content-Type", "text/plain")
                self.send_header("Content-Length", str(n))
                self.end_headers()
                blk = b"a" * 65536
                while n > 0:
                    k = min(n, len(blk)); self.wfile.write(blk[:k]); n -= k
                return
            if p == "/grande-chunk":
                n = int(float(q.get("mb", ["1"])[0]) * 1024 * 1024)
                self.send_response(200)
                self.send_header("Content-Type", "text/plain")
                self.send_header("Transfer-Encoding", "chunked")
                self.end_headers()
                blk = b"a" * 65536
                while n > 0:
                    k = min(n, len(blk))
                    self.wfile.write(b"%x\r\n" % k + blk[:k] + b"\r\n"); n -= k
                self.wfile.write(b"0\r\n\r\n")
                return
            if p == "/html":
                n = int(q.get("n", ["10"])[0])
                b = "<html><body>" + "".join('<div class="c"><a href="/v/%d">item %d</a></div>' % (i, i) for i in range(n)) + "</body></html>"
                return self.corpo(b.encode(), "text/html")
            if p == "/redir":
                self.send_response(302)
                self.send_header("Location", "/ok")
                self.send_header("Content-Length", "0")
                self.end_headers()
                return
            if p in ("/manifest", "/manifest.json"):
                return self.corpo(b'{"name":"Repo Teste","scrapers":['
                                  b'{"id":"bom","name":"Bom","filename":"prov/bom.js","supportedTypes":["movie","tv"]},'
                                  b'{"id":"lento","name":"Lento","filename":"prov/lento.js","supportedTypes":["movie"]}]}')
            if p == "/manifest-vazio":
                return self.corpo(b'{"name":"Vazio","scrapers":[]}')
            if p.startswith("/prov/") and p.endswith(".js"):
                nome = p[6:-3]
                if nome in CODIGOS:
                    return self.corpo(("var BASE=%r;\n" % base + CODIGOS[nome]).encode(), "application/javascript")
            self.corpo(b"nao", "text/plain", 404)
        except (BrokenPipeError, ConnectionResetError):
            pass


class S(ThreadingHTTPServer):
    def handle_error(self, request, client_address):
        pass  # cliente que desiste (prazo/cancelamento) e o esperado aqui


def main():
    s = S(("127.0.0.1", 0), H)
    s.daemon_threads = True
    with open(sys.argv[1], "w") as f:
        f.write("%d\n" % s.server_address[1])
    s.serve_forever()


if __name__ == "__main__":
    main()
