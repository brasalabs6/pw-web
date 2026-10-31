'use strict';
const assert = require('node:assert/strict');
const path = require('node:path');
const {chromium} = require(path.join(process.env.PW_BROWSER_MODULE_DIR, 'node_modules/playwright-core'));
(async () => {
  const browser = await chromium.launch({executablePath: process.env.CHROME_BIN, headless: true,
    args: ['--no-sandbox', '--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader']});
  try {
    const page = await browser.newPage();
    await page.goto(process.env.PW_D3D8_TEST_URL);
    await page.waitForFunction(() => document.body.dataset.result, null, {timeout: 30000});
    assert.equal(await page.evaluate(() => document.body.dataset.result), 'pass');
  } finally { await browser.close(); }
})().catch(error => { console.error(error); process.exitCode = 1; });
