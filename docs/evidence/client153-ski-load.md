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

## Skeleton binding and vertex transforms

The portable skin binding and vertex blending follow `A3DSkin::BindSkeleton`
and `A3DSkinMesh::GetBlendedVertices` at the same source revision. The test
binds the authorized wolf SKI to the original BON skeleton, applies states from
`A3DSkinModelActionCore`, computes `boneInit * absoluteBone` matrices, and
blends all vertices at the start and end of their tracks. Native, ASan/UBSan,
and WASM/Node each processed 997 vertices and observed all 997 move. A separate
behavior test verifies weighted translation and rejects an invalid bone index.
The test uses identity model transform and a virtual bone identity matrix; it
does not validate world offsets, full action playback, GPU skinning, textures,
or render any image. The browser runtime does not yet call this implementation.

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
build/client153-native/client153_skeleton_closure_test artifacts/client153-input/wolf.bon artifacts/client153-input/wolf.ski
cmake --build build/client153-sanitize --parallel 2
UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build/client153-sanitize --output-on-failure
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_skeleton_closure_test artifacts/client153-input/wolf.bon artifacts/client153-input/wolf.ski
emcmake cmake -S . -B build/client153-wasm-real -G Ninja -DBUILD_TESTING=ON \
  -DPW153_TEST_BON_PATH="$PWD/artifacts/client153-input/wolf.bon" \
  -DPW153_TEST_SKI_PATH="$PWD/artifacts/client153-input/wolf.ski"
cmake --build build/client153-wasm-real --target client153_skeleton_closure_test --parallel 2
ctest --test-dir build/client153-wasm-real -R '^client153_skeleton_closure$' -V
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
```

Asset-free native, WASM and sanitized suites each passed 13/13; the separate
opt-in real-asset runs passed as described. Assets embedded in a local WASM test
build remain subject to the distribution policy and are not committed.

## Animated resource instance

Added an `AnimatedSkin` lifecycle that loads the authorized SKI and original
BON, binds tracks, samples the original action core, updates the original bone
hierarchy, and exposes transformed mesh positions. The real wolf test checks
load, updates at 0 and 1,000 ms, changing positions, invalid time and close.
It passed in native, WASM/Node with both opt-in assets, and ASan/UBSan. The
instance requires initialized original `A3DConfig`; it is not yet wired into
the browser runtime. It uses identity model/virtual-bone transforms, samples
tracks directly rather than complete action playback, and loads a whole SKI
(up to 64 MiB). Shared track sets and concurrent copies of the same skeleton
have not been validated. It is not a game or a rendering proof.

```
cmake --build build/client153-sanitize --target client153_skeleton_closure_test --parallel 2
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_skeleton_closure_test artifacts/client153-input/wolf.bon artifacts/client153-input/wolf.ski
cmake --build build/client153-wasm-real --target client153_skeleton_closure_test --parallel 2
ctest --test-dir build/client153-wasm-real -R '^client153_skeleton_closure$' --output-on-failure
```
