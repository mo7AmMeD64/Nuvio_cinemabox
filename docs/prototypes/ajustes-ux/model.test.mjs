import test from 'node:test';
import assert from 'node:assert/strict';
import {
  OPTIONS, DEFAULTS, DEMO_VALUES, RECOMMENDATION, createModel,
  getOption, valueLabel, searchOptions, changedOptions, isAvailable,
} from './model.mjs';

test('moving through candidates only changes preview; cancel keeps saved values', () => {
  const model = createModel();
  const before = model.saved;
  model.begin({ focusTrailer: before.focusTrailer });
  model.candidate('focusTrailer', 'off');
  assert.equal(model.previewValues().focusTrailer, 'off');
  assert.deepEqual(model.saved, before);
  assert.deepEqual(model.draft.previous, before);
  model.cancel();
  assert.equal(model.draft, null);
  assert.deepEqual(model.previewValues(), before);
});

test('commit reports only changed IDs, and no-op edits never report a change', () => {
  const model = createModel(DEFAULTS);
  assert.deepEqual(model.commit(), []);
  model.begin({ glass: 'on' });
  assert.deepEqual(model.commit(), []);
  model.begin({ glass: 'off', rowLimit: '7', imageQuality: 'low' });
  assert.deepEqual(model.commit(), ['imageQuality', 'glass']);
  assert.equal(model.saved.glass, 'off');
  assert.equal(model.saved.imageQuality, 'low');
  assert.equal(model.draft, null);
});

test('invalid batch and candidate values are rejected atomically', () => {
  const model = createModel();
  model.begin({ focusTrailer: 'off' });
  const before = model.snapshot();
  assert.throws(() => model.begin({ glass: 'off', resolution: '8k' }), /Valor inválido/);
  assert.deepEqual(model.snapshot(), before);
  assert.throws(() => model.candidate('resolution', 1080), /Valor inválido/);
  assert.throws(() => model.candidate('secretToken', 'hello'), /desconhecido/);
  assert.deepEqual(model.snapshot(), before);
  assert.throws(() => createModel({ focusTrailer: true }), /Valor inválido/);
  assert.throws(() => createModel([]), /mapa/);
  model.cancel();
  assert.throws(() => model.candidate('glass', 'off'), /Abra uma edição/);
});

test('recommendation touches only documented device options and cancels as a whole', () => {
  assert.deepEqual(Object.keys(RECOMMENDATION.changes).sort(), ['glass', 'imageQuality', 'rowLimit']);
  for (const id of Object.keys(RECOMMENDATION.changes)) {
    assert.equal(getOption(id).scope, 'device');
  }
  const model = createModel({ focusTrailer: 'on', rowLimit: '16', audioLang: 'es' });
  const before = model.saved;
  model.begin(RECOMMENDATION.changes);
  assert.equal(model.previewValues().rowLimit, '7');
  assert.equal(model.previewValues().focusTrailer, 'on');
  assert.equal(model.previewValues().audioLang, 'es');
  assert.deepEqual(model.saved, before);
  model.cancel();
  assert.deepEqual(model.saved, before);
});

test('search is accent insensitive, includes advanced options and recognizes synonyms', () => {
  assert.deepEqual(searchOptions('   '), []);
  assert.equal(searchOptions('  memoria PARA imagens  ')[0].id, 'textureMemory');
  assert.equal(searchOptions('memória para imagens')[0].advanced, true);
  assert.ok(searchOptions('atraso').some(option => option.id === 'trailerDelay'));
  assert.ok(searchOptions('cc').some(option => option.id === 'subtitleLang'));
  assert.ok(searchOptions('dublado').some(option => option.id === 'audioLang'));
  assert.ok(searchOptions('travando').some(option => option.id === 'glass'));
  assert.deepEqual(searchOptions('termo-sem-correspondencia'), []);
  assert.deepEqual(searchOptions('trailer'), searchOptions('trailer'));
});

test('languages remain independent and carry the correct device/profile scope', () => {
  const model = createModel();
  const before = model.saved;
  model.begin({ uiLang: 'en' });
  assert.equal(model.previewValues().audioLang, before.audioLang);
  assert.equal(model.previewValues().subtitleLang, before.subtitleLang);
  assert.equal(model.previewValues().metadataLang, before.metadataLang);
  assert.deepEqual(model.commit(), ['uiLang']);
  assert.equal(getOption('uiLang').scope, 'device');
  for (const id of ['audioLang', 'subtitleLang', 'metadataLang', 'focusTrailer']) {
    assert.equal(getOption(id).scope, 'profile');
  }
});

test('dependent choices stay discoverable and explain their requirement', () => {
  const values = { ...DEFAULTS, heroTrailer: 'off' };
  assert.deepEqual(isAvailable(getOption('heroSound'), values), {
    available: false,
    reason: 'Ative “No destaque do topo” para alterar esta opção.',
    requires: 'heroTrailer',
  });
  assert.equal(isAvailable('trailerDelay', values).available, false);
  assert.equal(isAvailable('detailSound', values).available, true);
  assert.equal(isAvailable('heroSound', DEFAULTS).available, true);
  assert.ok(searchOptions('som no destaque').some(option => option.id === 'heroSound'));
});

test('default differences include advanced options and reset is complete', () => {
  const model = createModel({ ...DEMO_VALUES, textureMemory: '256' });
  assert.deepEqual(changedOptions(model.saved).map(option => option.id), ['focusTrailer', 'uiLang', 'textureMemory']);
  model.begin({ glass: 'off' });
  model.reset();
  assert.deepEqual(model.saved, DEFAULTS);
  assert.deepEqual(changedOptions(model.saved), []);
  assert.equal(model.draft, null);
});

test('external references cannot mutate defaults, saved state or draft', () => {
  const model = createModel();
  const initial = { glass: 'off' };
  const draft = model.begin(initial);
  initial.glass = 'on';
  draft.changes.glass = 'on';
  draft.previous.focusTrailer = 'off';
  const snapshot = model.snapshot();
  snapshot.saved.focusTrailer = 'off';
  snapshot.draft.changes.glass = 'on';
  model.saved.focusTrailer = 'off';
  assert.equal(model.saved.focusTrailer, 'on');
  assert.equal(model.previewValues().glass, 'off');
  assert.equal(model.draft.previous.focusTrailer, 'on');
  assert.throws(() => { DEFAULTS.glass = 'off'; }, TypeError);
  assert.throws(() => { OPTIONS[0].values[0].label = 'Changed'; }, TypeError);
});

test('all defaults are valid choices with human-readable labels', () => {
  assert.equal(new Set(OPTIONS.map(option => option.id)).size, OPTIONS.length);
  for (const option of OPTIONS) {
    assert.ok(valueLabel(option.id, option.defaultValue));
    assert.equal(typeof option.defaultValue, 'string');
    for (const item of option.values) assert.equal(typeof item.value, 'string');
  }
  assert.throws(() => valueLabel('glass', 'bogus'), /Valor inválido/);
});
