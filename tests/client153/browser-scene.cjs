'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const { chromium } = require(path.join(process.env.PW_BROWSER_MODULE_DIR, 'node_modules/playwright-core'));

(async () => {
  const browser = await chromium.launch({
    executablePath: process.env.CHROME_BIN,
    headless: true,
    args: ['--no-sandbox', '--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader']
  });
  const mobile = process.env.PW_CLIENT153_MOBILE === '1';
  const page = await browser.newPage({
    viewport: mobile ? { width: 390, height: 844 } : { width: 1100, height: 900 },
    isMobile: mobile,
    hasTouch: mobile
  });
  const errors = [];
  page.on('pageerror', error => errors.push(String(error)));
  const url = process.env.PW_CLIENT153_URL || 'http://127.0.0.1:8497/';
  try {
    await page.goto(url);
    await page.waitForFunction(() => ['READY', 'FAIL'].includes(window.__pw153?.status), null, { timeout: 30000 });
    await page.waitForFunction(() => window.__pw153?.frames > 2 || window.__pw153?.status === 'FAIL');
    const report = await page.evaluate(() => {
      const canvas = document.querySelector('canvas');
      const gl = canvas.getContext('webgl2');
      const data = new Uint8Array(canvas.width * canvas.height * 4);
      gl.readPixels(0, 0, canvas.width, canvas.height, gl.RGBA, gl.UNSIGNED_BYTE, data);
      let colored = 0;
      const colors = new Set();
      for (let i = 0; i < data.length; i += 4) {
        if (Math.abs(data[i] - 15) > 2 || Math.abs(data[i + 1] - 19) > 2 || Math.abs(data[i + 2] - 26) > 2) {
          colored++;
          colors.add(`${data[i]},${data[i + 1]},${data[i + 2]}`);
        }
      }
      return { ...window.__pw153, coloredPixels: colored, uniqueColors: colors.size, gpuError: gl.getError() };
    });
    assert.equal(report.status, 'READY');
    assert.equal(report.meshes, 1);
    assert.equal(report.drawCalls, 1);
    assert.equal(report.assets.length, 2);
    assert.deepEqual(report.mipLevels, [9]);
    assert.equal(report.gpuError, 0);
    assert.ok(report.coloredPixels > 1000);
    assert.ok(report.uniqueColors > 32);
    report.checks = ['original-model-pixels', 'all-original-texture-mipmaps'];

    const before = await page.locator('#canvas').screenshot();
    const box = await page.locator('#canvas').boundingBox();
    await page.mouse.move(box.x + box.width * .25, box.y + box.height * .5);
    await page.mouse.down();
    await page.mouse.move(box.x + box.width * .7, box.y + box.height * .52, { steps: 10 });
    await page.mouse.up();
    await page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve))));
    assert.notDeepEqual(await page.locator('#canvas').screenshot(), before, 'orbit must change rendered geometry');
    report.checks.push('orbit-changes-pixels');

    await page.evaluate(() => {
      window.__lose = document.querySelector('canvas').getContext('webgl2').getExtension('WEBGL_lose_context');
      window.__lose.loseContext();
    });
    await page.waitForFunction(() => window.__pw153.status === 'LOST');
    await page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve))));
    await page.evaluate(() => window.__lose.restoreContext());
    await page.waitForFunction(() => ['READY', 'FAIL'].includes(window.__pw153.status));
    assert.equal(await page.evaluate(() => window.__pw153.status), 'READY', 'restore must recreate resources');
    report.checks.push('loss-and-resource-restoration');

    const negative = await browser.newPage();
    await negative.route('**/*.mox', async route => {
      const response = await route.fetch();
      const data = await response.body();
      data[0] ^= 1;
      await route.fulfill({ response, body: data });
    });
    await negative.goto(url);
    await negative.waitForFunction(() => window.__pw153?.status === 'FAIL');
    assert.match(await negative.evaluate(() => window.__pw153.error), /integrity mismatch/);
    await negative.close();
    report.checks.push('corrupt-download-rejected');

    const malformed = await browser.newPage();
    let corrupted;
    await malformed.route('**/scene.json', async route => {
      const response = await route.fetch();
      const manifest = await response.json();
      const asset = await malformed.request.get(new URL(manifest.model.url, url).href);
      corrupted = await asset.body();
      corrupted[0] ^= 1;
      manifest.model.sha256 = crypto.createHash('sha256').update(corrupted).digest('hex');
      await route.fulfill({ response, json: manifest });
    });
    await malformed.route('**/*.mox', route => route.fulfill({ body: corrupted, contentType: 'application/octet-stream' }));
    await malformed.goto(url);
    await malformed.waitForFunction(() => window.__pw153?.status === 'FAIL');
    assert.match(await malformed.evaluate(() => window.__pw153.error), /unsupported MOX signature/);
    await malformed.close();
    report.checks.push('valid-hash-malformed-model-rejected-without-wasm-abort');
    const allocation = await browser.newPage();
    await allocation.addInitScript(() => {
      const prototype = WebGL2RenderingContext.prototype;
      const live = new Set();
      let textureFailure = false;
      for (const kind of ['Buffer', 'VertexArray', 'Texture', 'Program']) {
        const create = prototype[`create${kind}`], remove = prototype[`delete${kind}`];
        prototype[`create${kind}`] = function (...args) {
          if (kind === 'Texture' && !textureFailure) { textureFailure = true; return null; }
          const result = create.apply(this, args); if (result) live.add(result); return result;
        };
        prototype[`delete${kind}`] = function (resource) { live.delete(resource); return remove.call(this, resource); };
      }
      window.__liveGpuResources = () => live.size;
    });
    await allocation.goto(url);
    await allocation.waitForFunction(() => window.__pw153?.status === 'FAIL');
    assert.equal(await allocation.evaluate(() => window.__liveGpuResources()), 0, 'failed GPU allocation must release partial resources');
    await allocation.close();
    report.checks.push('gpu-allocation-failure-cleans-resources');
    assert.deepEqual(errors, []);
    report.browser = await browser.version();
    report.mobileEmulation = mobile;
    const out = process.env.PW_CLIENT153_EVIDENCE || 'artifacts/client153-browser-proof';
    fs.mkdirSync(out, { recursive: true });
    await page.screenshot({ path: path.join(out, 'original-model.png'), fullPage: true });
    fs.writeFileSync(path.join(out, 'report.json'), JSON.stringify(report, null, 2) + '\n');
    console.log(JSON.stringify(report, null, 2));
  } finally {
    await browser.close();
  }
})().catch(error => { console.error(error); process.exitCode = 1; });
