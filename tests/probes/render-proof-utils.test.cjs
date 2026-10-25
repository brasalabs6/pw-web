"use strict";
const { test } = require("node:test");
const assert = require("node:assert/strict");
const { groups, pixels, verify } = require("./render-proof-utils.cjs");
const valid = () => ({
  status: "PASS", passed: [...groups], pixels: structuredClone(pixels),
  renderer: "ANGLE (SwiftShader)", gl_version: "WebGL 2.0 (OpenGL ES 3.0)"
});
test("complete R0 evidence accepted", () => assert.doesNotThrow(() => verify(valid(), false)));
test("missing checks or pixels rejected", () => {
  const partial = valid();
  partial.passed.pop();
  assert.throws(() => verify(partial, false));
  const noPixel = valid();
  delete noPixel.pixels["far then near"];
  assert.throws(() => verify(noPixel, false));
});
test("forged pass and invalid pixel rejected", () => {
  const forged = valid();
  forged.pixels.triangle = [0,0,0,255];
  assert.throws(() => verify(forged, false));
  const badChannel = valid();
  badChannel.pixels.ARGB = [64,32,256,128];
  assert.throws(() => verify(badChannel, false));
});
test("negative must fail at the intended control, not on import", () => {
  const negative = valid();
  negative.status = "FAIL";
  negative.passed = ["initialize"];
  negative.error = "Error: triangle: 255,255,255,255 != 0,0,0,255";
  negative.pixels = { triangle: [255,255,255,255] };
  assert.doesNotThrow(() => verify(negative, true));
  negative.error = "Error: failed to fetch module";
  assert.throws(() => verify(negative, true));
});
test("wrong version/renderer rejected", () => {
  const bad = valid(); bad.gl_version = "WebGL 1.0";
  assert.throws(() => verify(bad, false));
  bad.gl_version = "WebGL 2.0"; bad.renderer = "";
  assert.throws(() => verify(bad, false));
});
