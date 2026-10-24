// Positive/negative Angelica research proof in an authorized local lab only.
// The original engine files/binaries MUST remain local. No network uploads.
"use strict";
const assert=require("node:assert/strict");
const fs=require("node:fs");
const path=require("node:path");
const {chromium}=require(path.join(process.env.PW_BROWSER_MODULE_DIR || "", "node_modules/playwright-core"));
(async ()=>{
  const root=path.resolve(process.env.PW_ANGELICA_PROBE_DIR || "artifacts/angelica-probe");
  const base="http://127.0.0.1:8481/";
  const browser=await chromium.launch({
    executablePath:process.env.CHROME_BIN,
    headless:true,
    args:["--no-sandbox","--enable-unsafe-swiftshader","--use-gl=angle","--use-angle=swiftshader"]
  });
  const report={schema_version:1,scope:"ARandomGen original numeric parity + synthetic WebGL2 grid only",
                evidence_kind:"local opt-in, not CI",cases:[]};
  try{
    async function run(name,breakNative){
      const page=await browser.newPage({viewport:{width:1280,height:900},deviceScaleFactor:1});
      const errors=[];
      page.on("pageerror",e=>errors.push(e.message));
      if(breakNative){
        await page.route("**/native.json",async route=>{
          const response=await route.fetch();
          const value=await response.json();
          assert.ok(value?.cases?.[0]?.uniform?.length>0,"Native fixture absent");
          value.cases[0].uniform[0]+=0.01;
          await route.fulfill({response,contentType:"application/json",body:JSON.stringify(value)});
        });
      }
      const http=await page.goto(base,{waitUntil:"domcontentloaded",timeout:20000});
      assert.equal(http.status(),200);
      await page.waitForFunction(()=>{
        const state=document.querySelector("#status")?.dataset.state;
        return state==="PASS"||state==="FAIL";
      },null,{timeout:35000});
      const evidence=await page.evaluate(()=>{
        const el=document.querySelector("#status");
        return {status:el.dataset.state,message:el.textContent,
                data:JSON.parse(document.querySelector("#result").textContent)};
      });
      const shot=path.join(root,"browser-"+name+".png");
      await page.screenshot({path:shot,fullPage:true});
      report.cases.push({name,status:evidence.status,message:evidence.message,
                         data:evidence.data,screenshot:path.basename(shot),
                         page_errors:errors});
      if(breakNative){
        assert.equal(evidence.status,"FAIL","Corrupted native evidence must fail closed");
        assert.match(evidence.message,/uniform.*seed.*differs/);
      }else{
        assert.equal(evidence.status,"PASS","WASM ↔ native parity or WebGL2 draw failed");
        assert.equal(evidence.data.compared.uniform,4096);
        assert.equal(evidence.data.compared.gaussian,256);
        assert.equal(evidence.data.triangles,1922);
        assert.ok(evidence.data.centerPixel?.[1]>=40,"WebGL pixel evidence missing");
        assert.equal(evidence.data.wasmABI.longBytes,4);
        assert.equal(evidence.data.wasmABI.pointerBytes,4);
        report.browser=await browser.version();
        report.gl_version=evidence.data.webgl;
      }
      assert.deepEqual(errors,[],"Uncaught JavaScript errors");
      await page.close();
    }
    await run("positive",false);
    await run("negative",true);
    fs.writeFileSync(path.join(root,"browser-proof.json"),JSON.stringify(report,null,2)+"\n");
    process.stdout.write("Angelica local positive/negative browser proof passed; manifest, screenshots and report kept only under artifacts/.\n");
  }finally{await browser.close();}
})().catch(err=>{console.error(err.stack);process.exitCode=1;});
