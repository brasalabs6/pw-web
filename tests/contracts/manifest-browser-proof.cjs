"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const { chromium } = require(path.join(
  process.env.PW_BROWSER_MODULE_DIR || "", "node_modules/playwright-core"
));

(async () => {
  const port = process.env.PW_MANIFEST_PORT || "8486";
  const url = `http://127.0.0.1:${port}/tests/contracts/manifest-browser.html`;
  const output = path.resolve(process.env.PW_MANIFEST_ARTIFACTS || "artifacts/manifest-probe");
  fs.mkdirSync(output, { recursive: true });
  const browser = await chromium.launch({
    executablePath: process.env.CHROME_BIN,
    headless: true,
    args: ["--no-sandbox"],
  });
  const page = await browser.newPage();
  const errors = [];
  page.on("pageerror", error => errors.push(error.message));
  try {
    const response = await page.goto(url, { waitUntil: "networkidle", timeout: 20000 });
    assert.ok(response && response.status() === 200, "manifest proof page must load");
    await page.waitForFunction(() => document.querySelector("#result")?.dataset.state !== undefined,
      null, { timeout: 20000 });
    const report = await page.evaluate(() => JSON.parse(document.querySelector("#result").textContent));
    assert.equal(report.status, "PASS", report.error || "browser manifest proof failed");
    assert.deepEqual(report.passed, [
      "repository root not served",
      "lazy index/catalog/bundle hash chain",
      "content graph and schema modules",
      "release pins content graph",
      "negative traversal rejected",
    ]);
    assert.deepEqual(errors, [], "uncaught browser exceptions");
    const evidence = {
      schema_version: 1,
      scope: "manifest schemas/validator browser compatibility; no game assets or gameplay",
      browser: await browser.version(),
      cases: report.passed,
      page_errors: errors,
      status: "PASS",
    };
    fs.writeFileSync(path.join(output, "browser-proof.json"), JSON.stringify(evidence, null, 2) + "\n");
    console.log(`Manifest validator browser proof PASS in Chrome ${evidence.browser}.`);
  } finally {
    await browser.close();
  }
})().catch(error => { console.error(error.stack || error); process.exitCode = 1; });
