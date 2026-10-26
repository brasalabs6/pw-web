# First SKI mesh load

Hypothesis: a browser-compatible loader derived from the original Angelica
skin read path can recover the real model's skinned vertices and mesh indices.
This is an intermediate resource-loading component, not a playable client.

Sources: `A3DSkin::Load`, `A3DSkinMesh::LoadData`, and `A3DMaterial::Load`,
authorized source revision `e3d6d0075d0a745c9f9859f2cb25b01d71dec045` and
legal decision in `docs/LEGAL.md`. The adapted implementation retains the
copyright notice. No original game asset is committed.

The loader currently accepts SKI versions 8 and 9 with skinned meshes, texture
names and material records. Rigid, morph, supple and muscle meshes fail clearly;
these must be implemented before it can cover the full game. The version 9
branch has not yet been verified against an original version 9 file. The
loaded meshes are not bound to the original skeleton or rendered by the web
client. Animation, textures, skinning and complete renderer integration remain.

Commands (the BON/SKI test assets are local and authorized):

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
build/client153-native/client153_ski_test artifacts/client153-input/wolf.ski
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake --build build/client153-sanitize --target client153_ski_test --parallel 2
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_ski_test
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_ski_test artifacts/client153-input/wolf.ski
```

Native and WASM suites passed 12/12. Native and ASan/UBSan real-file tests
read the wolf's version 8 SKI, 42 skeleton bones and two skinned meshes. WASM
executed the synthetic behavior, truncation and unsupported-class tests; the
real SKI has not yet been supplied to that WASM test. The parser rejects
out-of-range mesh indices and truncated input, but further validation against
the full asset corpus is required before treating it as complete.
