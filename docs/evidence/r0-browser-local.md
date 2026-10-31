# Local R0 WebGL2 browser proof

Date: 2026-10-25. PR #15 source HEAD before this evidence: `f24ca8a158e4fa4d91a8ae3a4ca922dbbf7923a3`.
This validates the synthetic R0 backend only. It is not an Angelica draw, a game asset, a native/GPU performance result, or evidence of gameplay.

## Environment

- Emscripten `6.0.13` (`e13ee988ce3d87a738dd8f3661d02223308c412a`)
- Google Chrome for Testing `153.0.8010.12`
- `playwright-core` `1.64.1`, installed outside the checkout under `/tmp/pw-playwright-tools`
- WebGL renderer: `ANGLE (Google, Vulkan 1.3.0 (SwiftShader Device (Subzero) (0x0000C0DE)), SwiftShader driver)`
- GL version: `WebGL 2.0 (OpenGL ES 3.0 Chromium)`

## Commands and results

```bash
source /tmp/pw-web-emsdk/emsdk_env.sh
./scripts/test-render-clip.sh
./scripts/build-render-probe.sh
CHROME_BIN=/home/nonexphere/.cache/ms-playwright/chromium-1243/chrome-linux64/chrome \
  PW_BROWSER_MODULE_DIR=/tmp/pw-playwright-tools PW_RENDER_PORT=8493 \
  ./scripts/probe-render-browser.sh
node --test tests/probes/render-proof-utils.test.cjs
./scripts/verify.sh
git diff --check
```

- Native and WASM R0 math/validation: **14 cases pass on each target**.
- Browser positive run: **10/10 groups** and expected pixels pass, covering initialize, triangle/clear, depth in both submission orders, ARGB channels, row-major translation, reject-before-draw, context loss, explicit restore, and idempotent dispose/reinitialize.
- Browser negative control: **expected FAIL** at the triangle pixel (`255,255,255,255` where the negative control expects clear black); it did not fail on module import or page error.
- Browser page errors: none. Node manifest validator: **5/5 pass**. Repository `verify.sh`: **31 Python tests and 14 native renderer cases pass**. `git diff --check`: pass.
- The runner wrote the local manifest and screenshots under ignored `artifacts/render-probe/`; they contain no game data and are not committed.

## Limits and next gate

This closes the previously missing local synthetic WebGL2 pixel/lifecycle execution, including compilation with the available SDK. The machine used SwiftShader software rendering, so this does not measure GPU performance. It does not exercise original Angelica code, `.mox` parsing, textures, maps, an asset package, the game loop, or network protocol. The first real-object browser proof remains pending an authorized original asset and a ported original mesh-to-WebGL path. See [first draw closure](first-draw-closure.md) for the current source/dependency evidence and asset gap.
