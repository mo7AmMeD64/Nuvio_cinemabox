// Ambiente dos plugins Nuvio dentro do QuickJS (src/pluginjs.c).
//
// E a mesma superficie que o Android oficial (NuvioTV, PluginRuntime.kt) e o
// app web (pluginWorker.js) dao ao scraper: fetch, console, setTimeout, atob,
// URL, AbortController, cheerio e require de cheerio/crypto-js. O que muda e
// QUEM faz o trabalho: a rede e o HTML ficam no C (rede.c, htmlq.c) e o JS so
// recebe o resultado. `__nv` e a ponte nativa; ela some do escopo global na
// primeira linha, entao o scraper nao chama a ponte direto.
//
// Gerado para C por tools/plugins-js.sh (src/pluginsjs_gerado.h). Mexeu aqui,
// rode o script.
(function () {
  var N = globalThis.__nv;
  delete globalThis.__nv;
  var G = globalThis;
  G.global = G; G.window = G; G.self = G;

  // ---------------------------------------------------------------- console
  function fmt(v) {
    try { if (v && v.stack) return String(v.message || v) + "\n" + String(v.stack); } catch (_) {}
    if (v === undefined) return "undefined";
    if (v === null) return "null";
    if (typeof v === "string") return v;
    try { var j = JSON.stringify(v); return j === undefined ? String(v) : j; } catch (_) { return String(v); }
  }
  function fwd(nivel, a) {
    var s = [];
    for (var i = 0; i < a.length; i++) s.push(fmt(a[i]));
    try { N.log(nivel, s.join(" ")); } catch (_) {}
  }
  G.console = {
    log: function () { fwd(0, arguments); }, info: function () { fwd(0, arguments); },
    debug: function () { fwd(0, arguments); }, trace: function () { fwd(0, arguments); },
    warn: function () { fwd(1, arguments); }, error: function () { fwd(2, arguments); }
  };

  // ---------------------------------------------------------------- tempo
  G.setTimeout = function (fn, ms) {
    var a = Array.prototype.slice.call(arguments, 2);
    if (typeof fn !== "function") return 0;
    return N.timer(function () { fn.apply(G, a); }, Number(ms) || 0, 0);
  };
  G.setInterval = function (fn, ms) {
    var a = Array.prototype.slice.call(arguments, 2);
    if (typeof fn !== "function") return 0;
    return N.timer(function () { fn.apply(G, a); }, Number(ms) || 0, 1);
  };
  G.clearTimeout = G.clearInterval = function (id) { if (id) N.untimer(id | 0); };
  if (typeof G.queueMicrotask !== "function")
    G.queueMicrotask = function (f) { Promise.resolve().then(f); };

  // ---------------------------------------------------------------- AbortController
  function AbortSignal() { this.aborted = false; this.reason = undefined; this._l = []; this.onabort = null; }
  AbortSignal.prototype.addEventListener = function (t, f) { if (t === "abort" && typeof f === "function") this._l.push(f); };
  AbortSignal.prototype.removeEventListener = function (t, f) { if (t === "abort") this._l = this._l.filter(function (x) { return x !== f; }); };
  AbortSignal.prototype.dispatchEvent = function (ev) {
    var self = this;
    if (typeof this.onabort === "function") try { this.onabort(ev); } catch (_) {}
    this._l.slice().forEach(function (f) { try { f.call(self, ev); } catch (_) {} });
    return true;
  };
  AbortSignal.prototype.throwIfAborted = function () { if (this.aborted) throw this.reason; };
  function erroAbort() { var e = new Error("The operation was aborted."); e.name = "AbortError"; return e; }
  AbortSignal.timeout = function (ms) {
    var c = new AbortController();
    setTimeout(function () { var e = new Error("The operation timed out."); e.name = "TimeoutError"; c.abort(e); }, ms);
    return c.signal;
  };
  function AbortController() { this.signal = new AbortSignal(); }
  AbortController.prototype.abort = function (r) {
    if (this.signal.aborted) return;
    this.signal.aborted = true;
    this.signal.reason = r === undefined ? erroAbort() : r;
    this.signal.dispatchEvent({ type: "abort" });
  };
  G.AbortSignal = AbortSignal;
  G.AbortController = AbortController;

  // ---------------------------------------------------------------- texto
  function TextEncoder() {}
  TextEncoder.prototype.encoding = "utf-8";
  TextEncoder.prototype.encode = function (s) {
    s = String(s === undefined ? "" : s);
    var b = [], i, c;
    for (i = 0; i < s.length; i++) {
      c = s.charCodeAt(i);
      if (c >= 0xd800 && c < 0xdc00 && i + 1 < s.length) {
        var d = s.charCodeAt(i + 1);
        if (d >= 0xdc00 && d < 0xe000) { c = 0x10000 + ((c - 0xd800) << 10) + (d - 0xdc00); i++; }
      }
      if (c < 0x80) b.push(c);
      else if (c < 0x800) b.push(0xc0 | (c >> 6), 0x80 | (c & 63));
      else if (c < 0x10000) b.push(0xe0 | (c >> 12), 0x80 | ((c >> 6) & 63), 0x80 | (c & 63));
      else b.push(0xf0 | (c >> 18), 0x80 | ((c >> 12) & 63), 0x80 | ((c >> 6) & 63), 0x80 | (c & 63));
    }
    return new Uint8Array(b);
  };
  function TextDecoder(enc) { this.encoding = String(enc || "utf-8").toLowerCase(); }
  TextDecoder.prototype.decode = function (buf) {
    if (buf === undefined) return "";
    var u = buf instanceof ArrayBuffer ? new Uint8Array(buf)
      : new Uint8Array(buf.buffer, buf.byteOffset || 0, buf.byteLength);
    if (this.encoding === "latin1" || this.encoding === "iso-8859-1" || this.encoding === "ascii") {
      var o = ""; for (var k = 0; k < u.length; k += 8192) o += String.fromCharCode.apply(null, u.subarray(k, k + 8192));
      return o;
    }
    return N.utf8(u.buffer.slice(u.byteOffset, u.byteOffset + u.byteLength));
  };
  G.TextEncoder = TextEncoder;
  G.TextDecoder = TextDecoder;
  if (!G.crypto) G.crypto = {};
  if (typeof G.crypto.getRandomValues !== "function")
    G.crypto.getRandomValues = function (a) {
      for (var i = 0; i < a.length; i++) a[i] = Math.floor(Math.random() * 4294967296);
      return a;
    };
  if (typeof G.crypto.randomUUID !== "function")
    G.crypto.randomUUID = function () {
      return "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(/[xy]/g, function (c) {
        var r = Math.random() * 16 | 0; return (c === "x" ? r : (r & 3 | 8)).toString(16); });
    };

  // ---------------------------------------------------------------- URL
  var RE_URL = /^([a-zA-Z][a-zA-Z0-9+.\-]*:)?(?:\/\/([^\/?#]*))?([^?#]*)(\?[^#]*)?(#.*)?$/;
  function URLSearchParams(init) {
    this._p = [];
    var self = this;
    if (init instanceof URLSearchParams) this._p = init._p.slice();
    else if (Array.isArray(init)) init.forEach(function (e) { self._p.push([String(e[0]), String(e[1])]); });
    else if (init && typeof init === "object") Object.keys(init).forEach(function (k) { self._p.push([k, String(init[k])]); });
    else if (typeof init === "string" && init) {
      init.replace(/^\?/, "").split("&").forEach(function (par) {
        if (!par) return;
        var i = par.indexOf("="), k = i < 0 ? par : par.slice(0, i), v = i < 0 ? "" : par.slice(i + 1);
        function dec(x) { try { return decodeURIComponent(x.replace(/\+/g, " ")); } catch (_) { return x; } }
        self._p.push([dec(k), dec(v)]);
      });
    }
  }
  function enc(s) { return encodeURIComponent(s).replace(/%20/g, "+").replace(/[!'()~]/g, function (c) { return "%" + c.charCodeAt(0).toString(16).toUpperCase(); }); }
  URLSearchParams.prototype.toString = function () { return this._p.map(function (e) { return enc(e[0]) + "=" + enc(e[1]); }).join("&"); };
  URLSearchParams.prototype.get = function (k) { for (var i = 0; i < this._p.length; i++) if (this._p[i][0] === k) return this._p[i][1]; return null; };
  URLSearchParams.prototype.getAll = function (k) { return this._p.filter(function (e) { return e[0] === k; }).map(function (e) { return e[1]; }); };
  URLSearchParams.prototype.has = function (k) { return this.get(k) !== null; };
  URLSearchParams.prototype.append = function (k, v) { this._p.push([String(k), String(v)]); this._u && this._u(); };
  URLSearchParams.prototype.set = function (k, v) {
    k = String(k); var feito = false;
    this._p = this._p.filter(function (e) { if (e[0] !== k) return true; if (feito) return false; e[1] = String(v); feito = true; return true; });
    if (!feito) this._p.push([k, String(v)]);
    this._u && this._u();
  };
  URLSearchParams.prototype["delete"] = function (k) { this._p = this._p.filter(function (e) { return e[0] !== k; }); this._u && this._u(); };
  URLSearchParams.prototype.sort = function () { this._p.sort(function (a, b) { return a[0] < b[0] ? -1 : a[0] > b[0] ? 1 : 0; }); this._u && this._u(); };
  URLSearchParams.prototype.forEach = function (f, t) { var self = this; this._p.forEach(function (e) { f.call(t, e[1], e[0], self); }); };
  URLSearchParams.prototype.keys = function () { return this._p.map(function (e) { return e[0]; })[Symbol.iterator](); };
  URLSearchParams.prototype.values = function () { return this._p.map(function (e) { return e[1]; })[Symbol.iterator](); };
  URLSearchParams.prototype.entries = function () { return this._p.map(function (e) { return [e[0], e[1]]; })[Symbol.iterator](); };
  URLSearchParams.prototype[Symbol.iterator] = URLSearchParams.prototype.entries;
  Object.defineProperty(URLSearchParams.prototype, "size", { get: function () { return this._p.length; } });

  function normCaminho(p) {
    var saida = [], partes = p.split("/");
    for (var i = 0; i < partes.length; i++) {
      var s = partes[i];
      if (s === "..") { if (saida.length > 1) saida.pop(); if (i === partes.length - 1) saida.push(""); }
      else if (s === ".") { if (i === partes.length - 1) saida.push(""); }
      else saida.push(s);
    }
    var r = saida.join("/");
    return r.charAt(0) === "/" ? r : "/" + r;
  }
  function URL(u, base) {
    u = String(u && u.href !== undefined ? u.href : u).trim();
    var m = RE_URL.exec(u);
    if (!m || !m[1]) {
      if (base === undefined) throw new TypeError("Invalid URL: " + u);
      var b = new URL(base);
      m = RE_URL.exec(u) || [];
      if (u.slice(0, 2) === "//") return new URL(b.protocol + u);
      var caminho = m[3] || "", busca = m[4], hash = m[5];
      var r = b.protocol + "//" + b.host;
      if (!caminho) r += b.pathname + (busca !== undefined ? busca : b.search) + (hash || "");
      else if (caminho.charAt(0) === "/") r += normCaminho(caminho) + (busca || "") + (hash || "");
      else r += normCaminho(b.pathname.replace(/[^\/]*$/, "") + caminho) + (busca || "") + (hash || "");
      return new URL(r);
    }
    this.protocol = m[1].toLowerCase();
    var auth = m[2] || "", arroba = auth.lastIndexOf("@");
    this.username = arroba >= 0 ? auth.slice(0, arroba).split(":")[0] : "";
    this.password = arroba >= 0 ? (auth.slice(0, arroba).split(":")[1] || "") : "";
    var hp = arroba >= 0 ? auth.slice(arroba + 1) : auth;
    var pm = /^(\[[^\]]*\]|[^:]*)(?::(\d*))?$/.exec(hp) || [hp, hp, ""];
    this.hostname = (pm[1] || "").toLowerCase();
    var porta = pm[2] || "";
    if ((this.protocol === "https:" && porta === "443") || (this.protocol === "http:" && porta === "80")) porta = "";
    this.port = porta;
    var esp = this.protocol === "http:" || this.protocol === "https:";
    this.pathname = esp ? normCaminho(m[3] || "/") : (m[3] || "");
    this.hash = m[5] && m[5] !== "#" ? m[5] : "";
    var self = this;
    this.searchParams = new URLSearchParams(m[4] || "");
    this._s = m[4] && m[4] !== "?" ? m[4] : "";
    this.searchParams._u = function () { var q = self.searchParams.toString(); self._s = q ? "?" + q : ""; };
  }
  Object.defineProperty(URL.prototype, "host", { get: function () { return this.hostname + (this.port ? ":" + this.port : ""); } });
  Object.defineProperty(URL.prototype, "origin", { get: function () { return this.protocol + "//" + this.host; } });
  Object.defineProperty(URL.prototype, "search", {
    get: function () { return this._s; },
    set: function (v) { v = String(v); this._s = v && v !== "?" ? (v.charAt(0) === "?" ? v : "?" + v) : "";
      var u = this.searchParams._u; this.searchParams = new URLSearchParams(this._s); this.searchParams._u = u; }
  });
  Object.defineProperty(URL.prototype, "href", {
    get: function () {
      var a = this.username ? this.username + (this.password ? ":" + this.password : "") + "@" : "";
      return this.protocol + "//" + a + this.host + this.pathname + this._s + this.hash;
    },
    set: function (v) { var n = new URL(v); for (var k in n) if (Object.prototype.hasOwnProperty.call(n, k)) this[k] = n[k]; }
  });
  URL.prototype.toString = URL.prototype.toJSON = function () { return this.href; };
  URL.canParse = function (u, b) { try { new URL(u, b); return true; } catch (_) { return false; } };
  G.URL = URL;
  G.URLSearchParams = URLSearchParams;

  // ---------------------------------------------------------------- fetch
  function Headers(init) {
    this._h = {};
    var self = this;
    if (!init) return;
    if (init instanceof Headers) init.forEach(function (v, k) { self.append(k, v); });
    else if (Array.isArray(init)) init.forEach(function (e) { self.append(e[0], e[1]); });
    else if (typeof init === "object") Object.keys(init).forEach(function (k) { if (init[k] != null) self.append(k, init[k]); });
  }
  Headers.prototype.append = function (k, v) {
    k = String(k).toLowerCase(); v = String(v);
    this._h[k] = this._h[k] !== undefined ? this._h[k] + ", " + v : v;
  };
  Headers.prototype.set = function (k, v) { this._h[String(k).toLowerCase()] = String(v); };
  Headers.prototype.get = function (k) { var v = this._h[String(k).toLowerCase()]; return v === undefined ? null : v; };
  Headers.prototype.has = function (k) { return this._h[String(k).toLowerCase()] !== undefined; };
  Headers.prototype["delete"] = function (k) { delete this._h[String(k).toLowerCase()]; };
  Headers.prototype.forEach = function (f, t) { var h = this._h, self = this; Object.keys(h).forEach(function (k) { f.call(t, h[k], k, self); }); };
  Headers.prototype.entries = function () { var h = this._h; return Object.keys(h).map(function (k) { return [k, h[k]]; })[Symbol.iterator](); };
  Headers.prototype.keys = function () { return Object.keys(this._h)[Symbol.iterator](); };
  Headers.prototype.values = function () { var h = this._h; return Object.keys(h).map(function (k) { return h[k]; })[Symbol.iterator](); };
  Headers.prototype[Symbol.iterator] = Headers.prototype.entries;
  Headers.prototype.getSetCookie = function () { var v = this.get("set-cookie"); return v ? [v] : []; };
  G.Headers = Headers;

  var UA = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
  function Response(r) {
    this.status = r.status | 0;
    this.ok = this.status >= 200 && this.status < 300;
    this.statusText = r.statusText || "";
    this.url = r.url || "";
    this.redirected = !!r.redirected;
    this.type = "basic";
    this.headers = new Headers();
    var self = this;
    String(r.headers || "").split("\n").forEach(function (l) {
      var i = l.indexOf(":"); if (i > 0) self.headers.append(l.slice(0, i).trim(), l.slice(i + 1).trim());
    });
    this._r = r;
    this.bodyUsed = false;
  }
  Response.prototype.text = function () { this.bodyUsed = true; return Promise.resolve(this._r.body || ""); };
  Response.prototype.json = function () {
    this.bodyUsed = true;
    var b = this._r.body;
    if (b === null || b === undefined || b === "") return Promise.resolve(null);
    try { return Promise.resolve(JSON.parse(b)); }
    catch (e) { console.error("fetch.json parse error:", e && e.message ? e.message : e); return Promise.resolve(null); }
  };
  Response.prototype.arrayBuffer = function () {
    this.bodyUsed = true;
    if (this._r.bytes) return Promise.resolve(this._r.bytes);
    return Promise.resolve(new TextEncoder().encode(this._r.body || "").buffer);
  };
  Response.prototype.clone = function () { return new Response(this._r); };
  G.Response = Response;

  function corpoDe(b) {
    if (b === undefined || b === null) return null;
    if (typeof b === "string") return b;
    if (b instanceof ArrayBuffer) return b;
    if (ArrayBuffer.isView && ArrayBuffer.isView(b)) return b.buffer.slice(b.byteOffset, b.byteOffset + b.byteLength);
    if (b instanceof URLSearchParams) return b.toString();
    return String(b);
  }
  G.fetch = function (input, op) {
    op = op || {};
    var url = String(input && input.url !== undefined ? input.url : input && input.href !== undefined ? input.href : input || "");
    var metodo = String(op.method || (input && input.method) || "GET").toUpperCase();
    if (["GET", "HEAD", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"].indexOf(metodo) < 0) metodo = "GET";
    var h = new Headers(op.headers || (input && input.headers));
    var corpo = corpoDe(op.body);
    var sinal = op.signal;
    if (!h.has("user-agent")) h.set("User-Agent", UA);
    h["delete"]("accept-encoding");
    if (op.body instanceof URLSearchParams && !h.has("content-type"))
      h.set("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
    if (metodo === "POST" && !h.has("content-type")) h.set("Content-Type", "application/x-www-form-urlencoded");
    if ((metodo === "PUT" || metodo === "PATCH") && !h.has("content-type")) h.set("Content-Type", "application/json");
    var linhas = "";
    h.forEach(function (v, k) { linhas += k + ": " + String(v).replace(/[\r\n]+/g, " ") + "\n"; });
    return new Promise(function (resolver, rejeitar) {
      if (sinal && sinal.aborted) { rejeitar(sinal.reason || erroAbort()); return; }
      var id = 0, fim = false;
      function aoAbortar() {
        if (fim) return; fim = true;
        try { N.cancel(id); } catch (_) {}
        rejeitar(sinal.reason || erroAbort());
      }
      id = N.fetch(url, metodo, linhas, corpo, op.redirect !== "manual", function (r, erro) {
        if (fim) return; fim = true;
        if (sinal) sinal.removeEventListener("abort", aoAbortar);
        if (erro) rejeitar(new TypeError("fetch failed: " + erro));
        else resolver(new Response(r));
      });
      if (sinal) sinal.addEventListener("abort", aoAbortar);
    });
  };

  // ---------------------------------------------------------------- cheerio
  // A arvore mora no C (htmlq.c). `_d` e o documento (objeto nativo: o C o
  // solta quando o ultimo wrapper que o segura for coletado), `_e` os indices
  // dos elementos, em ordem.
  var H = N.html;
  function W(d, e) {
    this._d = d; this._e = e; this.length = e.length;
    for (var i = 0; i < e.length; i++) this[i] = new No(d, e[i]);
  }
  // Um elemento como o cheerio entrega em .each/.get/[i]: $(no) volta a ser
  // selecao; .attribs, .tagName e .name para quem le direto.
  function No(d, i) { this._d = d; this._i = i; }
  Object.defineProperty(No.prototype, "tagName", { get: function () { return H.tag(this._d, this._i); } });
  Object.defineProperty(No.prototype, "name", { get: function () { return H.tag(this._d, this._i); } });
  Object.defineProperty(No.prototype, "attribs", { get: function () { return H.attrs(this._d, this._i); } });
  No.prototype.type = "tag";
  function un(d, a) { var v = [], visto = {}; for (var i = 0; i < a.length; i++) if (a[i] >= 0 && !visto[a[i]]) { visto[a[i]] = 1; v.push(a[i]); } v.sort(function (x, y) { return x - y; }); return new W(d, v); }
  function filtroDe(w, s) {
    if (s === undefined || s === null || s === "") return w;
    var d = w._d;
    return new W(d, w._e.filter(function (i) { return H.casa(d, i, String(s)); }));
  }
  var P = W.prototype;
  P.cheerio = "[cheerio object]";
  P.each = function (f) { for (var i = 0; i < this._e.length; i++) { var x = new W(this._d, [this._e[i]]); if (f.call(x, i, x) === false) break; } return this; };
  P.map = function (f) {
    var v = [];
    for (var i = 0; i < this._e.length; i++) {
      var x = new W(this._d, [this._e[i]]), r = f.call(x, i, x);
      if (r === undefined || r === null) continue;
      if (Array.isArray(r)) v.push.apply(v, r); else v.push(r);
    }
    return { length: v.length, get: function (i) { return typeof i === "number" ? v[i < 0 ? v.length + i : i] : v; },
      toArray: function () { return v; }, join: function (s) { return v.join(s); } };
  };
  P.find = function (s) {
    var d = this._d, todos = [];
    if (s && s._e) { var alvo = s._e, meus = this._e; return new W(d, alvo.filter(function (a) {
      for (var p = H.pai(d, a); p >= 0; p = H.pai(d, p)) if (meus.indexOf(p) >= 0) return true; return false; })); }
    for (var i = 0; i < this._e.length; i++) todos = todos.concat(H.sel(d, this._e[i], String(s || "")));
    return this._e.length > 1 ? un(d, todos) : new W(d, todos);
  };
  P.filter = function (s) {
    if (typeof s !== "function") return filtroDe(this, s);
    var d = this._d, v = [];
    for (var i = 0; i < this._e.length; i++) { var x = new W(d, [this._e[i]]); if (s.call(x, i, x)) v.push(this._e[i]); }
    return new W(d, v);
  };
  P.not = function (s) {
    var d = this._d, f = typeof s === "function";
    return new W(d, this._e.filter(function (e, i) { var x = new W(d, [e]); return f ? !s.call(x, i, x) : !H.casa(d, e, String(s)); }));
  };
  P.is = function (s) {
    if (typeof s === "function") return this.filter(s).length > 0;
    for (var i = 0; i < this._e.length; i++) if (H.casa(this._d, this._e[i], String(s))) return true;
    return false;
  };
  P.has = function (s) { var d = this._d; return new W(d, this._e.filter(function (e) { return H.sel(d, e, String(s)).length > 0; })); };
  P.hasClass = function (c) { for (var i = 0; i < this._e.length; i++) { var v = H.attr(this._d, this._e[i], "class"); if (v && (" " + v.replace(/\s+/g, " ") + " ").indexOf(" " + c + " ") >= 0) return true; } return false; };
  P.first = function () { return new W(this._d, this._e.slice(0, 1)); };
  P.last = function () { return new W(this._d, this._e.slice(-1)); };
  P.eq = function (i) { i = i | 0; if (i < 0) i += this._e.length; return new W(this._d, i >= 0 && i < this._e.length ? [this._e[i]] : []); };
  P.slice = function (a, b) { return new W(this._d, this._e.slice(a, b)); };
  P.get = function (i) { if (typeof i === "number") return this[i < 0 ? this._e.length + i : i]; return this.toArray(); };
  P.toArray = function () { var v = []; for (var i = 0; i < this._e.length; i++) v.push(this[i]); return v; };
  P.index = function () { if (!this._e.length) return -1; var d = this._d, e = this._e[0], n = 0; for (var p = H.ant(d, e); p >= 0; p = H.ant(d, p)) n++; return n; };
  P.text = function () { return this._e.length ? H.texto(this._d, this._e) : ""; };
  P.html = function () { return this._e.length ? H.html(this._d, this._e[0], 0) : null; };
  P.attr = function (n) {
    if (!this._e.length) return undefined;
    if (n === undefined) return H.attrs(this._d, this._e[0]);
    var v = H.attr(this._d, this._e[0], String(n));
    return v === null || v === undefined ? undefined : v;
  };
  P.data = function (n) { if (n === undefined) return {}; var v = this.attr("data-" + String(n).replace(/[A-Z]/g, function (c) { return "-" + c.toLowerCase(); })); if (v === undefined) return undefined; try { return JSON.parse(v); } catch (_) { return v; } };
  P.val = function () { if (!this._e.length) return undefined; var t = H.tag(this._d, this._e[0]); if (t === "textarea") return this.text(); if (t === "select") return this.find("option[selected]").attr("value"); return this.attr("value"); };
  function rel(w, f, s, todos) {
    var d = w._d, v = [];
    for (var i = 0; i < w._e.length; i++) {
      for (var x = f(d, w._e[i]); x >= 0; x = f(d, x)) { v.push(x); if (!todos) break; }
    }
    return filtroDe(un(d, v), s);
  }
  P.next = function (s) { return rel(this, H.prox, s, false); };
  P.prev = function (s) { return rel(this, H.ant, s, false); };
  P.nextAll = function (s) { return rel(this, H.prox, s, true); };
  P.prevAll = function (s) { return rel(this, H.ant, s, true); };
  P.parent = function (s) { return rel(this, H.pai, s, false); };
  P.parents = function (s) { return rel(this, H.pai, s, true); };
  P.closest = function (s) {
    var d = this._d, v = [];
    for (var i = 0; i < this._e.length; i++)
      for (var x = this._e[i]; x >= 0; x = H.pai(d, x)) if (H.casa(d, x, String(s))) { v.push(x); break; }
    return un(d, v);
  };
  P.children = function (s) { var d = this._d, v = []; for (var i = 0; i < this._e.length; i++) v = v.concat(H.filhos(d, this._e[i])); return filtroDe(un(d, v), s); };
  P.siblings = function (s) { var d = this._d, meus = this._e, v = []; for (var i = 0; i < meus.length; i++) { var p = H.pai(d, meus[i]); H.filhos(d, p >= 0 ? p : 0).forEach(function (c) { if (meus.indexOf(c) < 0) v.push(c); }); } return filtroDe(un(d, v), s); };
  P.contents = P.children;
  P.add = function (o) { return o && o._e && o._d === this._d ? un(this._d, this._e.concat(o._e)) : this; };
  P.end = function () { return this; };
  Object.defineProperty(P, "attribs", { get: function () { return this._e.length ? H.attrs(this._d, this._e[0]) : {}; } });
  Object.defineProperty(P, "tagName", { get: function () { return this._e.length ? H.tag(this._d, this._e[0]) : undefined; } });
  Object.defineProperty(P, "name", { get: function () { return this._e.length ? H.tag(this._d, this._e[0]) : undefined; } });
  P[Symbol.iterator] = function () { return this.toArray()[Symbol.iterator](); };

  function carregar(html) {
    var d = H.load(html === undefined || html === null ? "" : String(html));
    var $ = function (s, ctx) {
      if (s instanceof W) return s;
      if (s instanceof No) return new W(s._d, [s._i]);
      if (Array.isArray(s)) return un(d, s.filter(function (x) { return x instanceof No; }).map(function (x) { return x._i; }));
      if (s === undefined || s === null || s === "") return new W(d, []);
      s = String(s);
      if (/^\s*</.test(s)) return new W(d, []);   // criar elemento: fora do contrato
      if (ctx !== undefined) { var c = $(ctx); return c.find(s); }
      return new W(d, H.sel(d, -1, s));
    };
    $.html = function (el) {
      if (el === undefined) return H.html(d, -1, 1);
      var w = $(el); return w._e.length ? H.html(d, w._e[0], 1) : "";
    };
    $.text = function (el) { if (el === undefined) return H.texto(d, [0]); return $(el).text(); };
    $.root = function () { return new W(d, [0]); };
    $.load = carregar;
    return $;
  }
  var cheerio = { load: carregar, default: { load: carregar } };
  G.cheerio = cheerio;

  // ---------------------------------------------------------------- require
  // CryptoJS so e compilado quando o scraper pede (require ou o global): sao
  // 64 KB de JS que metade dos scrapers nao usa.
  function cripto() {
    var c = N.cripto();
    Object.defineProperty(G, "CryptoJS", { value: c, writable: true, configurable: true });
    return c;
  }
  Object.defineProperty(G, "CryptoJS", {
    configurable: true,
    get: cripto,
    set: function (v) { Object.defineProperty(G, "CryptoJS", { value: v, writable: true, configurable: true }); }
  });
  G.require = function (n) {
    if (n === "cheerio" || n === "cheerio-without-node-native" || n === "react-native-cheerio") return cheerio;
    if (n === "crypto-js") return G.CryptoJS;
    throw new Error("Module not allowed: " + n);
  };
  // ---------------------------------------------------------------- resultado
  // O contrato do Android (LocalScraperResult -> Stream, PluginManager.kt e
  // resultToStream do web) vira o JSON de um addon Stremio, que o C le com o
  // MESMO parser das fontes de addon (stream_extrair): qualidade, Dolby
  // Vision, tamanho e selos saem de la, sem um segundo classificador.
  function txt(v) {
    if (v === null || v === undefined) return null;
    if (typeof v === "object") { try { v = JSON.stringify(v); } catch (_) { return null; } }
    v = String(v);
    return v.indexOf("[object") >= 0 ? null : v;
  }
  Object.defineProperty(G, "__nuvioMapear", { enumerable: false, value: function (lista, nomeScraper, idScraper, max) {
    var saida = [];
    (Array.isArray(lista) ? lista : []).slice(0, max).forEach(function (r) {
      if (!r || typeof r !== "object") return;
      var url = typeof r.url === "string" ? r.url : r.url && typeof r.url.url === "string" ? r.url.url : "";
      if (!url.trim() || url.indexOf("[object") >= 0) return;
      var q = txt(r.quality); q = q && q.trim() ? q : null;
      var titulo = txt(r.title), nome = txt(r.name);
      var base = (nome && nome.trim()) || (titulo && titulo.trim()) || nomeScraper;
      var rotQ = q || "Unknown";
      var rotulo = base + (String(base).indexOf(rotQ) >= 0 ? "" : " - " + rotQ);
      var desc = [(titulo && titulo.trim()) || (nome && nome.trim()) || nomeScraper];
      var extra = [txt(r.size), txt(r.language)].filter(function (x) { return x !== null && x !== ""; });
      if (extra.length) desc.push(extra.join(" \u2022 "));
      var s = { name: rotulo, description: desc.join("\n"), url: url,
        behaviorHints: { bingeGroup: "nuvio-plugin|" + idScraper + "|" + (q || "") } };
      if (r.headers && typeof r.headers === "object" && !Array.isArray(r.headers)) {
        var h = {}, tem = false;
        Object.keys(r.headers).forEach(function (k) { if (typeof r.headers[k] === "string") { h[k] = r.headers[k]; tem = true; } });
        if (tem) s.behaviorHints.proxyHeaders = { request: h };
      }
      if (typeof r.infoHash === "string" && r.infoHash) s.infoHash = r.infoHash;
      saida.push(s);
    });
    return JSON.stringify({ streams: saida });
  } });

  G.SCRAPER_ID = N.id;
  try { G.SCRAPER_SETTINGS = JSON.parse(N.ajustes || "{}"); } catch (_) { G.SCRAPER_SETTINGS = {}; }
  G.TMDB_API_KEY = N.tmdb || "";
})();
