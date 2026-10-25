// Opt-in synthetic R0 proof. This does not run the original Angelica renderer.
"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const { verify } = require("./render-proof-utils.cjs");
const { chromium } = require(path.join(
  process.env.PW_BROWSER_MODULE_DIR || "", "node_modules/playwright-core"
));

(async () => {
  const root = path.resolve(process.env.PW_RENDER_PROBE_DIR || "artifacts/render-probe");
  const base = `http://127.0.0.1:${process.env.PW_RENDER_PORT || "8482"}/`;
  const browser = await chromium.launch({
    executablePath: process.env.CHROME_BIN,
    headless: true,
    args: ["--no-sandbox", "--enable-unsafe-swiftshader", "--use-gl=angle", "--use-angle=swiftshader"]
  });
  const report = {
    schema_version: 1, scope: "synthetic R0 GLES3/WebGL2 only; no Angelica draw",
    evidence_kind: "local opt-in, not public CI or original renderer",
    browser: await browser.version(), cases: []
  };
  let failure;
  try {
    for (const negative of [false, true]) {
      const name = negative ? "negative" : "positive";
      const page = await browser.newPage({ viewport: { width: 960, height: 720 },
        deviceScaleFactor: 1 });
      const errors = [];
      page.on("pageerror", error => errors.push(error.message));
      try {
        const url = base + (negative ? "?negative=1" : "");
        const http = await page.goto(url, { waitUntil: "domcontentloaded", timeout: 20000 });
        assert.ok(http && http.status() === 200, name + ": unexpected HTTP status");
        await page.waitForFunction(() => {
          const state = document.querySelector("#status")?.dataset.state;
          return state === "PASS" || state === "FAIL";
        }, null, { timeout: 45000 });
        const evidence = await page.evaluate(() => {
          const status = document.querySelector("#status")?.dataset.state;
          const raw = document.querySelector("#result")?.textContent;
          if (!raw) throw new Error("missing JSON result");
          return { status, data: JSON.parse(raw) };
        });
        assert.equal(evidence.status, evidence.data.status, "HTML status and JSON mismatch");
        report.cases.push({ name, status: evidence.status, data: evidence.data,
          screenshot: "browser-" + name + ".png", page_errors: errors });
        await page.screenshot({
          path: path.join(root, "browser-" + name + ".png"), fullPage: true
        });
        assert.deepEqual(errors, [], name + ": uncaught JS errors");
        verify(evidence.data, negative);
        if (!negative) {
          report.renderer = evidence.data.renderer;
          report.gl_version = evidence.data.gl_version;
          report.software_renderer = /SwiftShader/i.test(report.renderer);
        }
      } finally {
        await page.close();
      }
    }
  } catch (error) {
    failure = error;
    report.status = "FAIL";
    report.error = String(error.stack || error);
  } finally {
    if (!failure) report.status = "PASS";
    fs.writeFileSync(path.join(root, "browser-proof.json"),
      JSON.stringify(report, null, 2) + "\n");
    await browser.close();
  }
  if (failure) throw failure;
  process.stdout.write("R0 positive/negative pixel + lifecycle browser proof PASS; " +
    (report.software_renderer ? "SwiftShader software renderer (not GPU benchmark)" :
      "driver recorded without GPU-performance claim") + ".\n" +
    "Evidence stays local under artifacts/render-probe/.\n");
})().catch(error => { console.error(error.stack || error); process.exitCode = 1; });
