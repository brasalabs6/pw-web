"use strict";
// Strict R0 browser-evidence verification; not an Angelica rendering parity claim.
const assert = require("node:assert/strict");

const groups = Object.freeze([
  "initialize", "triangle and clear", "near then far", "far then near",
  "ARGB shader channels", "row-major translation", "reject before drawing",
  "context loss", "explicit restoration", "idempotent dispose and reinitialize"
]);
const pixels = Object.freeze({
  triangle: [255,255,255,255],
  clear: [0,0,0,255],
  "near then far": [255,0,0,255],
  "far then near": [255,0,0,255],
  ARGB: [64,32,16,128],
  "translated interior": [255,255,255,255],
  "translated exterior": [0,0,0,255],
  "invalid draw unchanged": [0,0,0,255],
  "restored triangle": [255,255,255,255],
  "reinitialized triangle": [255,255,255,255]
});
function close(actual, expected, label) {
  assert.ok(Array.isArray(actual) && actual.length === 4, label + ": invalid pixel payload");
  actual.forEach((channel, index) => {
    assert.ok(Number.isInteger(channel) && channel >= 0 && channel <= 255,
      label + ": channel out of range");
    assert.ok(Math.abs(channel - expected[index]) <= 2,
      label + ": channel " + index + ", received " + channel + ", expected " + expected[index]);
  });
}
function verify(result, negative) {
  assert.ok(result && typeof result === "object" && !Array.isArray(result), "missing result object");
  assert.ok(typeof result.renderer === "string" && result.renderer.length > 0,
    "renderer metadata missing");
  assert.match(result.gl_version || "", /WebGL 2\.0/i, "must use WebGL2");
  assert.ok(Array.isArray(result.passed), "passed must be an array");
  assert.ok(result.pixels && typeof result.pixels === "object", "pixels missing");
  if (negative) {
    assert.equal(result.status, "FAIL", "negative control must fail");
    assert.deepEqual(result.passed, ["initialize"],
      "negative must fail at the expected center-pixel assertion, not at an unrelated step");
    assert.match(result.error || "", /triangle: .* != 0,0,0,255/,
      "expected deliberate mismatch rather than an incidental error");
    close(result.pixels.triangle, pixels.triangle, "negative triangle");
    assert.deepEqual(Object.keys(result.pixels), ["triangle"]);
    return;
  }
  assert.equal(result.status, "PASS", "R0 probe must pass");
  assert.equal(result.error, undefined, "unexpected error in positive case");
  assert.deepEqual(result.passed, groups, "must execute every group exactly once, in order");
  assert.deepEqual(Object.keys(result.pixels).sort(), Object.keys(pixels).sort(),
    "expected exactly the documented pixel assertions");
  for (const [name, rgba] of Object.entries(pixels)) close(result.pixels[name], rgba, name);
}
module.exports = { groups, pixels, verify };
