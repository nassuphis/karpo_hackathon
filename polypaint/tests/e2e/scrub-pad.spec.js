const { test, expect } = require('@playwright/test');

// Scrub pad: dblclick a numeric literal in any program editor to drag-edit
// it. Ephemeral by contract: only the pad writes while open, any external
// edit closes it, Escape reverts. Live preview is lores-only (compute
// preview for pp/cp, render lores for rt/render-ss) — never the full
// pipeline.

async function openPadOnCoeffLiteral(page, source, literal) {
  return page.evaluate(({ source, literal }) => {
    const ta = document.getElementById('cp-source-text');
    ta.value = source;
    const pos = source.indexOf(literal);
    ta.selectionStart = ta.selectionEnd = pos + 1;
    _onProgramSourceDblClick('cp', { clientX: 300, clientY: 300 });
    return { padVisible: document.getElementById('program-scrub-pad').style.display === 'block' };
  }, { source, literal });
}

test.beforeEach(async ({ page }) => {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
});

test.describe('Scrub pad', () => {
  test('dblclick on a numeric literal opens the pad; on a word opens help', async ({ page }) => {
    const opened = await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    expect(opened.padVisible).toBe(true);
    await expect(page.locator('#program-scrub-value')).toHaveText('2');

    const wordCase = await page.evaluate(() => {
      const ta = document.getElementById('cp-source-text');
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('linear') + 2;
      _onProgramSourceDblClick('cp', { clientX: 300, clientY: 300 });
      return {
        padVisible: document.getElementById('program-scrub-pad').style.display === 'block',
        helpVisible: document.getElementById('program-help-inspector').style.display === 'block',
      };
    });
    expect(wordCase.padVisible).toBe(false);
    expect(wordCase.helpVisible).toBe(true);
  });

  test('dragging scrubs the literal in place and runs validation', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    const after = await page.evaluate(() => {
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      // Range defaults to value ± max(1, |value|) = [0, 4] for value 2;
      // drag to the far right = max.
      _scrubPadDragStart({ clientX: rect.right, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
      return {
        text: document.getElementById('cp-source-text').value,
        readout: document.getElementById('program-scrub-value').textContent,
        status: document.getElementById('coeff-program-status').textContent,
      };
    });
    expect(after.text).toBe('poly = linear(poly, 4, 3)\nemit');
    expect(after.readout).toBe('4');
    expect(after.status).toContain('Text source changed');
  });

  test('editable range widens the scrub span', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    const after = await page.evaluate(() => {
      document.getElementById('program-scrub-min').value = '0';
      document.getElementById('program-scrub-max').value = '100';
      _scrubPadSetRange();
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      _scrubPadDragStart({ clientX: rect.right, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
      return document.getElementById('cp-source-text').value;
    });
    expect(after).toBe('poly = linear(poly, 100, 3)\nemit');
  });

  test('Escape reverts to the original literal and closes the pad', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    await page.evaluate(() => {
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      _scrubPadDragStart({ clientX: rect.right, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
    });
    await page.keyboard.press('Escape');
    const after = await page.evaluate(() => ({
      text: document.getElementById('cp-source-text').value,
      padVisible: document.getElementById('program-scrub-pad').style.display === 'block',
    }));
    expect(after.text).toBe('poly = linear(poly, 2, 3)\nemit');
    expect(after.padVisible).toBe(false);
  });

  test('typing in the editor closes the pad (ephemeral binding)', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    await page.locator('#cp-source-text').focus();
    await page.keyboard.press('End');
    await page.keyboard.type('x');
    await expect.poll(async () => page.evaluate(() =>
      document.getElementById('program-scrub-pad').style.display
    )).not.toBe('block');
  });

  test('live preview routes to the lores endpoint only, never the full pipeline', async ({ page }) => {
    await page.evaluate(() => {
      window._previewCalls = 0;
      window._fullPipelineCalls = 0;
      window.runComputePreview = async () => { window._previewCalls += 1; };
      window.runRasterPipeline = async () => { window._fullPipelineCalls += 1; };
    });
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    await page.evaluate(() => {
      document.getElementById('program-scrub-live').checked = true;
      _scrubPadToggleLive(true);
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      _scrubPadDragStart({ clientX: rect.right, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
    });
    await expect.poll(async () => page.evaluate(() => window._previewCalls), { timeout: 3000 }).toBeGreaterThan(0);
    expect(await page.evaluate(() => window._fullPipelineCalls)).toBe(0);
  });

  test('arrow keys nudge the value with range-relative steps', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    // Default range for 2 is [0, 4]; 1% step = 0.04, Shift = 0.4.
    await page.keyboard.press('ArrowRight');
    await expect(page.locator('#program-scrub-value')).toHaveText('2.04');
    await page.keyboard.press('Shift+ArrowRight');
    await expect(page.locator('#program-scrub-value')).toHaveText('2.44');
    await page.keyboard.press('ArrowLeft');
    expect(await page.evaluate(() => document.getElementById('cp-source-text').value)).toBe('poly = linear(poly, 2.4, 3)\nemit');
  });

  test('root editor scrubs static args and offers the render lores preview', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const opened = await page.evaluate(() => {
      const ta = document.getElementById('render-rt-source-text');
      ta.value = 'pull_unit_circle(0.75, 1)';
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('0.75') + 1;
      _onProgramSourceDblClick('rt', { clientX: 300, clientY: 300 });
      return {
        padVisible: document.getElementById('program-scrub-pad').style.display === 'block',
        padHtml: document.getElementById('program-scrub-pad').innerHTML,
      };
    });
    expect(opened.padVisible).toBe(true);
    expect(opened.padHtml).toContain('live render lores preview');
  });
});
