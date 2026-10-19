import { before, after, test } from 'node:test';
import assert from 'node:assert/strict';
import { createRequire } from 'node:module';
const { chromium } = createRequire(import.meta.url)('playwright');
import { DEFAULTS, DEMO_VALUES, RECOMMENDATION } from './model.mjs';

// Run while the prototype server is available at http://127.0.0.1:4179.
// Every task gets a fresh browser context; only the persistence task reloads it.
const URL = process.env.PROTOTYPE_URL || 'http://127.0.0.1:4179';
let browser;
before(async () => { browser = await chromium.launch({ channel: 'chrome', headless: true }); });
after(async () => { await browser?.close(); });

const key = (page, name) => page.locator(`[data-key="${name}"]`);
const snapshot = page => page.evaluate(() => window.prototypeSnapshot());
const waitState = (page, expected) => page.waitForFunction(expected => {
  const state = window.prototypeSnapshot();
  return Object.entries(expected).every(([name, value]) => state[name] === value);
}, expected);

async function withPage(task) {
  const context = await browser.newContext({ viewport: { width: 1440, height: 1000 } });
  const page = await context.newPage();
  const errors = [];
  page.on('pageerror', error => errors.push(error.message));
  try {
    await page.goto(URL, { waitUntil: 'networkidle' });
    await waitState(page, { focused: 'opt-focusTrailer', modal: null });
    await task(page);
    assert.deepEqual(errors, [], 'No uncaught browser errors are allowed');
  } finally {
    await context.close();
  }
}

async function openRecommendation(page) {
  await key(page, 'nav-performance').click();
  await key(page, 'recommendation').click();
  await waitState(page, { modal: 'recommend' });
}

async function startRecommendation(page) {
  await openRecommendation(page);
  await key(page, 'experiment-recommend').click();
  await waitState(page, { modal: 'experiment' });
  await page.waitForFunction(() => {
    const keep = document.querySelector('[data-key="keep"]');
    return keep && !keep.disabled;
  });
}

test('remote navigation changes focus and sections without changing preferences', async () => {
  await withPage(async page => {
    const before = (await snapshot(page)).saved;
    assert.deepEqual(before, DEMO_VALUES);
    await page.keyboard.press('ArrowDown');
    await waitState(page, { focused: 'opt-detailTrailer' });
    await page.keyboard.press('ArrowRight');
    await waitState(page, { focused: 'opt-detailTrailer', modal: null });
    await page.keyboard.press('ArrowLeft');
    await waitState(page, { focused: 'nav-trailers' });
    await page.keyboard.press('ArrowDown');
    await waitState(page, { focused: 'nav-languages' });
    await page.keyboard.press('ArrowRight');
    await waitState(page, { section: 'languages', focused: 'opt-uiLang' });
    assert.deepEqual((await snapshot(page)).saved, before);
    assert.equal((await snapshot(page)).draft, null);
  });
});

test('editing with remote creates a candidate; Escape cancels and restores row focus', async () => {
  await withPage(async page => {
    const before = (await snapshot(page)).saved;
    await page.keyboard.press('Enter');
    await waitState(page, { modal: 'edit', focused: 'choice-on' });
    await page.keyboard.press('ArrowDown');
    await waitState(page, { focused: 'choice-off' });
    assert.equal((await snapshot(page)).draft.changes.focusTrailer, 'off');
    assert.deepEqual((await snapshot(page)).saved, before);
    await page.keyboard.press('Escape');
    await waitState(page, { modal: null, focused: 'opt-focusTrailer' });
    assert.deepEqual((await snapshot(page)).saved, before);
    assert.equal((await snapshot(page)).draft, null);
  });
});

test('confirmed preference survives reopening the editor and reloading the page', async () => {
  await withPage(async page => {
    await key(page, 'opt-focusTrailer').click();
    await key(page, 'choice-off').click();
    await waitState(page, { modal: null });
    assert.equal((await snapshot(page)).saved.focusTrailer, 'off');
    await page.reload({ waitUntil: 'networkidle' });
    await waitState(page, { focused: 'opt-focusTrailer' });
    assert.equal((await snapshot(page)).saved.focusTrailer, 'off');
    await key(page, 'opt-focusTrailer').click();
    await waitState(page, { modal: 'edit', focused: 'choice-off' });
    assert.equal(await key(page, 'choice-off').locator('.check').textContent(), '✓');
    assert.equal(await key(page, 'choice-on').locator('.check').textContent(), '');
  });
});

test('search reveals an advanced option and Back restores query, results and focus', async () => {
  await withPage(async page => {
    const before = (await snapshot(page)).saved;
    await key(page, 'nav-search').click();
    await key(page, 'search-input').fill('memória');
    assert.equal(await key(page, 'result-textureMemory').count(), 1);
    await key(page, 'result-textureMemory').click();
    await waitState(page, { section: 'performance', focused: 'opt-textureMemory', advanced: false });
    assert.equal(await key(page, 'opt-textureMemory').isVisible(), true);
    assert.equal(await page.locator('.revealed-note').isVisible(), true);
    await page.keyboard.press('Escape');
    await waitState(page, { section: 'search', query: 'memória', focused: 'result-textureMemory' });
    assert.equal(await key(page, 'search-input').inputValue(), 'memória');
    assert.equal(await key(page, 'result-textureMemory').isVisible(), true);
    assert.deepEqual((await snapshot(page)).saved, before);
  });
});

test('dependent sound remains visible and opens its prerequisite without editing it', async () => {
  await withPage(async page => {
    await key(page, 'opt-heroTrailer').click();
    await key(page, 'choice-off').click();
    await waitState(page, { modal: null });
    const before = (await snapshot(page)).saved;
    assert.equal(await key(page, 'opt-heroSound').getAttribute('data-unavailable'), 'true');
    await key(page, 'opt-heroSound').click();
    await waitState(page, { modal: 'unavailable' });
    assert.match(await page.getByRole('dialog').innerText(), /Ative “No destaque do topo”/);
    await key(page, 'requirement').click();
    await waitState(page, { modal: null, focused: 'opt-heroTrailer' });
    assert.deepEqual((await snapshot(page)).saved, before);
    await page.keyboard.press('Escape');
    await waitState(page, { focused: 'opt-heroSound' });
    assert.deepEqual((await snapshot(page)).saved, before);
  });
});

test('profile restoration explains scope, cancels safely and applies only after confirmation', async () => {
  await withPage(async page => {
    const before = (await snapshot(page)).saved;
    await key(page, 'opt-focusTrailer').click();
    await key(page, 'restore').click();
    await waitState(page, { modal: 'restore', focused: 'cancel-restore' });
    assert.match(await page.getByRole('dialog').innerText(), /outras TVs que usam o mesmo perfil/);
    assert.deepEqual((await snapshot(page)).saved, before);
    await key(page, 'cancel-restore').click();
    await waitState(page, { modal: 'edit' });
    assert.deepEqual((await snapshot(page)).saved, before);
    await page.keyboard.press('Escape');
    await waitState(page, { modal: null });
    await key(page, 'opt-focusTrailer').click();
    await key(page, 'restore').click();
    await key(page, 'confirm-restore').click();
    await waitState(page, { modal: null, focused: 'opt-focusTrailer' });
    const saved = (await snapshot(page)).saved;
    assert.deepEqual(saved, { ...before, focusTrailer: DEFAULTS.focusTrailer });
    await page.reload({ waitUntil: 'networkidle' });
    assert.deepEqual((await snapshot(page)).saved, saved);
  });
});

test('recommendation preview can be canceled with Escape or its explicit revert button', async () => {
  await withPage(async page => {
    const before = (await snapshot(page)).saved;
    await openRecommendation(page);
    await key(page, 'cancel-recommend').click();
    await waitState(page, { modal: null });
    assert.deepEqual((await snapshot(page)).saved, before);
    await startRecommendation(page);
    assert.deepEqual((await snapshot(page)).draft.changes, RECOMMENDATION.changes);
    assert.deepEqual((await snapshot(page)).saved, before);
    await page.keyboard.press('Escape');
    await waitState(page, { modal: null, focused: 'recommendation' });
    assert.deepEqual((await snapshot(page)).saved, before);
    assert.equal((await snapshot(page)).draft, null);
    await startRecommendation(page);
    await key(page, 'cancel-experiment').click();
    await waitState(page, { modal: null, focused: 'recommendation' });
    assert.deepEqual((await snapshot(page)).saved, before);
    assert.equal((await snapshot(page)).draft, null);
  });
});

test('keeping a recommendation saves only its local changes and survives reload', async () => {
  await withPage(async page => {
    const before = (await snapshot(page)).saved;
    await startRecommendation(page);
    await key(page, 'keep').click();
    await waitState(page, { modal: null, focused: 'recommendation' });
    assert.deepEqual((await snapshot(page)).saved, { ...before, ...RECOMMENDATION.changes });
    assert.equal((await snapshot(page)).draft, null);
    await page.reload({ waitUntil: 'networkidle' });
    assert.deepEqual((await snapshot(page)).saved, { ...before, ...RECOMMENDATION.changes });
  });
});

test('recommendation timeout restores the entire previous configuration', async () => {
  await withPage(async page => {
    const before = (await snapshot(page)).saved;
    if (page.clock?.install) await page.clock.install();
    await startRecommendation(page);
    assert.deepEqual((await snapshot(page)).saved, before);
    if (page.clock?.fastForward) await page.clock.fastForward(16_000);
    else await page.waitForTimeout(16_000);
    await waitState(page, { modal: null, focused: 'recommendation' });
    assert.deepEqual((await snapshot(page)).saved, before);
    assert.equal((await snapshot(page)).draft, null);
    assert.match(await page.locator('#toast').innerText(), /tempo terminou/i);
  });
});

test('application timeout preserves all preferences when the preview never becomes ready', async () => {
  await withPage(async page => {
    const before = (await snapshot(page)).saved;
    const storedBefore = await page.evaluate(() => localStorage.getItem('nuvio-ajustes-ux-demo-v1'));
    assert.equal(storedBefore, null, 'The fresh context has not written any preferences');
    await page.clock.install();
    // Intentional fault injection: image loading can succeed, but decoding never
    // signals readiness. This isolates the five-second application watchdog.
    await page.evaluate(() => {
      Image.prototype.decode = () => new Promise(() => {});
    });
    await openRecommendation(page);
    await key(page, 'experiment-recommend').click();
    await waitState(page, { modal: 'experiment' });
    assert.equal(await key(page, 'keep').isDisabled(), true);
    assert.deepEqual((await snapshot(page)).draft.changes, RECOMMENDATION.changes);
    assert.deepEqual((await snapshot(page)).saved, before);
    await page.clock.fastForward(6_000);
    await waitState(page, { modal: null, focused: 'recommendation' });
    assert.deepEqual((await snapshot(page)).saved, before);
    assert.equal((await snapshot(page)).draft, null);
    assert.equal(await page.evaluate(() => localStorage.getItem('nuvio-ajustes-ux-demo-v1')), storedBefore);
    assert.match(await page.locator('#toast').innerText(), /prévia não ficou pronta/);
  });
});
