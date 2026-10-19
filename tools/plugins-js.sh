#!/bin/bash
# Gera src/pluginsjs_gerado.h a partir de src/pluginsjs/*.js: o ambiente dos
# plugins (base.js) e o CryptoJS 4.2.0 minificado (MIT, src/pluginsjs/
# LICENSE-crypto-js). Os builds so compilam src/*.c, entao o JS entra no
# binario como texto C. Rode depois de mexer em base.js.
#
#   bash tools/plugins-js.sh
set -eu
cd "$(dirname "$0")/.."
python3 - <<'PY'
import os
def lit(nome, caminho):
    s = open(caminho, 'rb').read()
    out = ['static const char %s[] =' % nome]
    linha = ''
    for b in s:
        c = chr(b)
        if c == '\\': e = '\\\\'
        elif c == '"': e = '\\"'
        elif c == '\n': e = '\\n'
        elif c == '\t': e = '\\t'
        elif 32 <= b < 127: e = c
        else: e = '\\%03o' % b
        # "??" vira trigrafo em C antigo; quebrar o literal evita o problema.
        if e == '?' and linha.endswith('?'): linha += '" "'
        linha += e
        if c == '\n' or len(linha) > 100:
            out.append('  "%s"' % linha); linha = ''
    if linha: out.append('  "%s"' % linha)
    out[-1] += ';'
    return '\n'.join(out) + '\n'
h = ['// GERADO por tools/plugins-js.sh a partir de src/pluginsjs/. Nao edite.\n',
     '#ifndef NV_PLUGINSJS_GERADO_H\n#define NV_PLUGINSJS_GERADO_H\n',
     lit('PJ_BASE', 'src/pluginsjs/base.js'),
     '// CryptoJS 4.2.0 (MIT) — Copyright (c) 2009-2013 Jeff Mott, 2013-2016 Evan Vosberg.\n',
     lit('PJ_CRIPTO', 'src/pluginsjs/crypto-js-4.2.0.min.js'),
     '#endif\n']
open('src/pluginsjs_gerado.h', 'w').write(''.join(h))
print('src/pluginsjs_gerado.h', os.path.getsize('src/pluginsjs_gerado.h'), 'bytes')
PY
