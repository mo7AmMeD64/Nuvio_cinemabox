// A arte cinematografica e GERADA, nao desenhada aqui. Este teste existe para
// impedir uma coisa so: que alguem substitua uma imagem aprovada por uma
// aproximacao feita a mao e ninguem perceba.
//
// A arte ENVIADA no pacote e JPEG (o PNG custava 145 MB). O PNG original de
// cada par continua no diretorio de geracao, e e ele que carrega a prova: o
// registro guarda o sha256 do PNG de origem E o do JPEG derivado. Conferir so
// o JPEG nao provaria nada, porque quem substituisse a arte tambem gravaria o
// hash novo.
import fs from 'node:fs';
import assert from 'node:assert/strict';
import crypto from 'node:crypto';

const sha = f => crypto.createHash('sha256').update(fs.readFileSync(f)).digest('hex');

// Dimensoes de JPEG: varrer os marcadores ate um SOF. Nao ha offset fixo como
// no PNG — segmentos de metadado (EXIF, quantizacao) vem antes e variam.
function jpegDims(buf) {
  let i = 2;
  while (i < buf.length) {
    if (buf[i] !== 0xFF) { i++; continue; }
    const m = buf[i + 1];
    if (m >= 0xC0 && m <= 0xCF && m !== 0xC4 && m !== 0xC8 && m !== 0xCC)
      return { height: buf.readUInt16BE(i + 5), width: buf.readUInt16BE(i + 7) };
    i += 2 + buf.readUInt16BE(i + 2);
  }
  throw new Error('JPEG sem marcador SOF');
}

const jobs = JSON.parse(fs.readFileSync('assets/cinematic/prompts.json', 'utf8'));
const records = fs.existsSync('assets/cinematic/provenance.jsonl')
  ? fs.readFileSync('assets/cinematic/provenance.jsonl', 'utf8').trim().split('\n').filter(Boolean).map(JSON.parse)
  : [];
const partial = process.argv.includes('--partial');

let checked = 0, pairs = 0;
const hashes = new Set();
for (const j of jobs) {
  if (!fs.existsSync(j.output)) { assert.ok(partial, `Falta ${j.title} ${j.variant}`); continue; }
  const r = records.findLast(r => r.id === j.id && r.variant === j.variant);
  assert.ok(r, `Sem proveniencia de geracao: ${j.output}`);

  // O JPEG do pacote confere com o registro...
  assert.equal(sha(j.output), r.sha256Jpeg, 'Nao substituir a arte gerada por aproximacoes');
  // ...e o registro confere com o PNG original, que este repo nao carrega.
  // Sem esta linha o teste so provaria que o arquivo nao mudou desde o ultimo
  // `git add`, o que nao e a pergunta.
  if (fs.existsSync(r.source)) assert.equal(sha(r.source), r.sha256, `Original divergiu: ${r.source}`);

  assert.ok(!hashes.has(r.sha256Jpeg), 'Home e detalhe usam artes distintas');
  hashes.add(r.sha256Jpeg);

  const d = jpegDims(fs.readFileSync(j.output));
  assert.equal(d.width, r.jpegWidth);
  assert.equal(d.height, r.jpegHeight);
  assert.ok(d.width >= 1900 && d.width / d.height >= 2.7 && d.width / d.height <= 3.3);

  if (j.variant === 'home' && fs.existsSync(j.output.replace('-home.jpg', '-detail.jpg'))) pairs++;
  checked++;
}
console.log(`cinematic images: ${partial ? 'PARCIAL' : 'PASS'} (${checked}/${jobs.length} imagens, ${pairs}/${jobs.length / 2} pares completos)`);
