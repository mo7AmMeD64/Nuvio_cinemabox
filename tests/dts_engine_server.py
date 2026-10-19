#!/usr/bin/env python3
"""Real range HTTP origin, including a sparse MP4 whose mdat sits beyond 2 GiB."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import json, re, sys, threading, time
root, port_file, log_file = map(Path, sys.argv[1:])
lock = threading.Lock()
active = 0
class Handler(BaseHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'
    def log_message(self, *_): pass
    def do_GET(self):
        global active
        authorized = self.headers.get('Authorization') == 'Bearer engine-test' and self.headers.get('X-Fixture') == 'yes'
        if not authorized:
            self.send_response(401); self.send_header('Content-Length', '0'); self.end_headers(); return
        if self.path == '/idle':
            time.sleep(10); return
        match = re.fullmatch(r'bytes=(\d+)-(\d+)', self.headers.get('Range', ''))
        if not match: self.send_error(400); return
        lo, hi = map(int, match.groups())
        assert 0 <= hi - lo < 1024 * 1024
        slow = self.path.startswith('/slow/')
        parallel = self.path.startswith('/parallel/')
        prefetch = self.path.startswith('/prefetch/')
        path = self.path.removeprefix('/slow') if slow else self.path.removeprefix('/parallel')
        if prefetch: path = self.path.removeprefix('/prefetch')
        if slow: time.sleep(.05)
        target = root / path.removeprefix('/')
        if target.parent != root or not target.is_file(): self.send_error(404); return
        total = target.stat().st_size
        with lock, log_file.open('a') as log:
            active += 1
            log.write(json.dumps({'path':self.path, 'lo':lo, 'hi':hi, 'auth':authorized, 'active':active,
                                 'connection':self.client_address[1]})+'\n')
        if parallel: time.sleep(.12)
        if prefetch: time.sleep(.8 if lo == 8*1024*1024 else .02)
        if lo >= total:
            with lock: active -= 1
            self.send_response(416); self.send_header('Content-Range', f'bytes */{total}'); self.send_header('Content-Length', '0'); self.end_headers(); return
        hi = min(hi, total - 1)
        if slow: hi = min(hi, lo + 256 * 1024 - 1)
        with target.open('rb') as source:
            source.seek(lo); body = source.read(hi - lo + 1)
        self.send_response(206)
        self.send_header('Content-Range', f'bytes {lo}-{hi}/{total}')
        self.send_header('Content-Length', str(len(body))); self.end_headers()
        try: self.wfile.write(body)
        finally:
            with lock: active -= 1
class Server(ThreadingHTTPServer):
    daemon_threads = True
    def handle_error(self,*_): pass
server = Server(('127.0.0.1',0), Handler)
port_file.write_text(str(server.server_address[1]))
server.serve_forever()
