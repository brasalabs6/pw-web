// An independent browser proof, not an LLM assertion and not Perfect World gameplay.
"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const { chromium } = require(path.join(process.env.PW_BROWSER_MODULE_DIR || "", "node_modules/playwright-core"));
(async () => {
  const dir = process.env.PW_BROWSER_ARTIFACTS || "artifacts/browser";
  fs.mkdirSync(dir, { recursive: true });
  const browser = await chromium.launch({
    executablePath: process.env.CHROME_BIN,
    headless: true,
    args: ["--no-sandbox", "--enable-unsafe-swiftshader", "--use-gl=angle", "--use-angle=swiftshader"]
  });
  const results = [];
  try {
    for (const viewport of [{width:1366,height:768,name:"desktop"},{width:390,height:844,name:"mobile"}]) {
      const page = await browser.newPage({viewport, deviceScaleFactor:1});
      const issues = [];
      page.on("pageerror", e => issues.push(String(e)));
      const errors=[];
      page.on("console", m => { if (m.type()==="error") errors.push(m.text()); });
      const response = await page.goto("http://127.0.0.1:8080/", {waitUntil:"networkidle"});
      assert.equal(response.status(),200,"Local demo did not respond");
      await page.waitForFunction(() => {
        const s=document.querySelector("#graphics-state");
        return s && s.textContent !== "INICIALIZANDO";
      }, {timeout: 15000});
      await page.waitForTimeout(1400);
      const state = await page.evaluate(() => {
        const canvas=document.querySelector("#viewport");
        const gl=canvas?.getContext("webgl2");
        const data={state:document.querySelector("#graphics-state").textContent,
                    fps:document.querySelector("#fps").textContent,
                    canvas_width:canvas.width,canvas_height:canvas.height,
                    gl_version:gl ? gl.getParameter(gl.VERSION) : null,gl_error:gl?.getError()};
        if(gl && canvas.width>10 && canvas.height>10){
          const pixels=new Uint8Array(4);
          gl.readPixels(Math.floor(canvas.width/2),Math.floor(canvas.height/2),1,1,gl.RGBA,gl.UNSIGNED_BYTE,pixels);
          data.center_pixel=Array.from(pixels);
        }
        return data;
      });
      const shot=path.join(dir, viewport.name+".png");
      await page.screenshot({path:shot, fullPage:true});
      results.push({viewport:viewport.name, http_status:response.status(),...state,
                    page_errors:issues,console_errors:errors, screenshot:path.basename(shot)});
      assert.equal(state.state,"ATIVO","WebGL2 did not initialize");
      assert.match(state.gl_version, /WebGL 2/i);
      assert.ok(state.canvas_width>20 && state.canvas_height>20);
      assert.match(state.fps, /\d+ FPS/);
      assert.deepEqual(issues,[],"Javascript exceptions");
      await page.close();
    }
    fs.writeFileSync(path.join(dir,"report.json"),JSON.stringify({
      schema_version:1,scope:"PW Web scaffold WebGL2 procedural triangle; not a real PW client",
      browser:await browser.version(),results},null,2));
    console.log("Browser WebGL2 proof passed in Chrome at desktop and mobile sizes.");
  } finally {
    await browser.close();
  }
})().catch(err=>{console.error(err.stack);process.exitCode=1;});
