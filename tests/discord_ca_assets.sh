#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 - <<'PY'
import hashlib
from pathlib import Path
p=Path('deploy/app/art/discord-ca.pem')
assert hashlib.sha256(p.read_bytes()).hexdigest() == 'a41b5d356aea97a529fe27e0f7316d2f9d946d75927476cf9cf1b90637d00505'
assert p.read_bytes().count(b'-----BEGIN CERTIFICATE-----') == 121
PY
sandbox=$(mktemp -d)
trap 'rm -rf "$sandbox"' EXIT
mkdir -p "$sandbox/tools" "$sandbox/deploy/app/art"
cp tools/tizen-art.sh "$sandbox/tools/"
cp deploy/app/art/discord-ca.pem "$sandbox/deploy/app/art/"
# Synthetic sensitive files must never survive the actual staging script.
for file in discord-p1.txt discord-p2.txt.tmp private.pem; do
  printf 'synthetic fixture\n' > "$sandbox/deploy/app/art/$file"
done
NUVIO_ARTE_ESTAGIO="$sandbox/staged" bash "$sandbox/tools/tizen-art.sh" >/dev/null
cmp deploy/app/art/discord-ca.pem "$sandbox/staged/discord-ca.pem"
[ "$(find "$sandbox/staged" -type f | wc -l | tr -d ' ')" = 1 ]
python3 - <<'PY'
from pathlib import Path
import re
s=Path('tools/release-android.sh').read_text()
pattern=re.search(r"^SEGREDO='(.*)'$",s,re.M).group(1)
for name in ('assets/art/discord-p1.txt','assets/art/discord-p2.txt.tmp'):
    assert re.search(pattern,name)
assert not re.search(pattern,'assets/art/discord-ca.pem')
PY
printf 'discord CA asset: verified bundle, exact allowlist and token rejection PASS\n'
