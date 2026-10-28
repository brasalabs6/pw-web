'use strict';
// Run against a staged original model scene and an authorized BGRA DDS.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const { chromium } = require(path.join(process.env.PW_BROWSER_MODULE_DIR, 'node_modules/playwright-core'));

(async () => {
  const texture = fs.readFileSync(process.env.PW_CLIENT153_BGRA_DDS);
  assert.ok(texture.length >= 128 && texture.toString('ascii', 0, 4) === 'DDS ');
  assert.equal(texture.readUInt32LE(80) & 0x45, 0x41, 'expected uncompressed RGBA');
  assert.equal(texture.readUInt32LE(88), 32);
  assert.deepEqual([92, 96, 100, 104].map(offset => texture.readUInt32LE(offset)),
                   [0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000]);
  const expectedMips = Math.max(1, texture.readUInt32LE(28));
  const sha256 = crypto.createHash('sha256').update(texture).digest('hex');
  const browser = await chromium.launch({
    executablePath: process.env.CHROME_BIN,
    headless: true,
    args: ['--no-sandbox', '--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader']
  });
  try {
    const page = await browser.newPage({ viewport: { width: 1100, height: 900 } });
    await page.route('**/scene.json', async route => {
      const response = await route.fetch();
      const scene = await response.json();
      for (const record of Object.values(scene.textures)) {
        record.url = 'test-uncompressed.dds';
        record.bytes = texture.length;
        record.sha256 = sha256;
      }
      await route.fulfill({ json: scene });
    });
    await page.route('**/test-uncompressed.dds', route =>
      route.fulfill({ body: texture, contentType: 'application/octet-stream' }));
    await page.goto(process.env.PW_CLIENT153_URL || 'http://127.0.0.1:8497/');
    await page.waitForFunction(() => window.__pw153?.status === 'FAIL' || window.__pw153?.frames > 2,
                               null, { timeout: 30000 });
    const result = await page.evaluate(() => {
      const canvas = document.querySelector('canvas');
      const gl = canvas.getContext('webgl2');
      const pixels = new Uint8Array(canvas.width * canvas.height * 4);
      gl.readPixels(0, 0, canvas.width, canvas.height, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
      const colors = new Set();
      for (let i = 0; i < pixels.length; i += 4)
        colors.add((pixels[i] << 16) | (pixels[i + 1] << 8) | pixels[i + 2]);
      return { status: window.__pw153.status, error: window.__pw153.error,
               mipLevels: window.__pw153.mipLevels, colors: colors.size, glError: gl.getError() };
    });
    assert.equal(result.status, 'READY', result.error);
    assert.ok(result.mipLevels.length > 0 && result.mipLevels.every(levels => levels === expectedMips));
    assert.ok(result.colors > 100, 'expected textured pixels');
    assert.equal(result.glError, 0);
    console.log(JSON.stringify(result));
  } finally {
    await browser.close();
  }
})().catch(error => { console.error(error); process.exitCode = 1; });
