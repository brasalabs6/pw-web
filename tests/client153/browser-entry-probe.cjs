'use strict';
// The browser entry probe (m-boot): loads the ORIGINAL client entry module
// (the reachable closure of wWinMain, linked by scripts/link-client153-full.py
// --entry-symbol pw153_browser_probe) in headless Chromium with a real
// WebGL2 context, and captures the worker message stream — the client's
// own print/stderr output, the entry return value, and the VFS logs
// (AF.log/EC.log) — as the boot diagnosis. This is the browser counterpart
// of the Node entry probe; where Node stopped at "WebGL2 context is not
// current", Chromium has a real context.
//
// Environment: CHROME_BIN (executable path; default: system
// google-chrome-stable), PW_BROWSER_MODULE_DIR (node_modules containing
// playwright-core), PW153_ENTRY_PORT (server port; default 8497).
const {spawn} = require('node:child_process');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const {classify} = require('./browser-entry-outcome.cjs');

const PORT = process.env.PW153_ENTRY_PORT || '8497';
const ROOT = path.resolve(__dirname, '..', '..');
const CHROME_BIN = process.env.CHROME_BIN || '/usr/bin/google-chrome-stable';
const PW_BROWSER_MODULE_DIR = process.env.PW_BROWSER_MODULE_DIR ||
    '/home/nonexphere/websites/guidev';
const {chromium} = require(
    path.join(PW_BROWSER_MODULE_DIR, 'node_modules', 'playwright-core'));

(async () => {
  const server = spawn('python3',
      [path.join(ROOT, 'scripts', 'client153-entry-server.py'), PORT],
      {cwd: ROOT, stdio: ['ignore', 'pipe', 'pipe']});
  const serverLog = [];
  server.stdout.on('data', d => serverLog.push(String(d)));
  server.stderr.on('data', d => serverLog.push(String(d)));
  await new Promise(resolve => setTimeout(resolve, 800));

  const browser = await chromium.launch({
    executablePath: CHROME_BIN,
    headless: true,
    args: ['--no-sandbox', '--enable-unsafe-swiftshader', '--use-gl=angle',
           '--use-angle=swiftshader'],
  });
  let report = {terminal: null, messages: []};
  try {
    const page = await browser.newPage({viewport: {width: 1100, height: 900}});
    page.on('pageerror', error => report.messages.push({pageerror: String(error)}));
    await page.goto(`http://127.0.0.1:${PORT}/`);
    const deadline = Date.now() + 600000;
    while (Date.now() < deadline) {
      const status = await page.evaluate(
          () => document.querySelector('#status').textContent);
      if (status.includes('"type":"return"') ||
          status.includes('"type":"exception"') ||
          status.includes('WORKER ERROR')) {
        report.terminal = status;
        break;
      }
      await new Promise(resolve => setTimeout(resolve, 1000));
    }
    report.terminal = report.terminal ||
        await page.evaluate(() => document.querySelector('#status').textContent);
  } finally {
    await browser.close();
    server.kill();
  }

  const full = ['--- server log ---', ...serverLog,
                '--- probe stream ---', report.terminal || '(no terminal)'].join('\n');
  const outPath = path.join(os.tmpdir(), 'pw153-entry-probe-report.log');
  fs.writeFileSync(outPath, full);
  console.log(full);
  console.log('report saved:', outPath);

  const outcome = classify(report.terminal);
  console.log('entry outcome:', outcome, '(a clean return does not prove gameplay)');
  process.exitCode = outcome === 'returned' ? 0 : 1;
})().catch(error => {
  console.error(error);
  process.exitCode = 1;
});
