# CI zlib dependency

Hypothesis: the PCK loader's native build needs the zlib development package in
the pinned Emscripten container. On commit `1ac46c3`, GitHub Actions run
[40403896792](https://github.com/brasalabs6/pw-web/actions/runs/40403896792/job/121856569633)
failed at CMake configure with missing `ZLIB_LIBRARY` and `ZLIB_INCLUDE_DIR`.
The workflow now installs `zlib1g-dev` in that container before native configure;
the WASM build retains Emscripten's `-sUSE_ZLIB=1`.

Local native command: `cmake --build build/client153-native --parallel 2 &&
ctest --test-dir build/client153-native --output-on-failure`: PASS 11/11 with
local zlib. Remote validation of the changed workflow is pending its next run;
local tests do not prove the container installation succeeds. No proprietary
assets or server configuration are included. Revert the workflow commit to
restore the previous CI configuration. The skin-model notification seam remains
unresolved and must be reconnected with the real implementation, not hidden by
assuming the public setter is unreachable.

The next run `40404531386` found zlib successfully and then failed because GCC
13 does not recognize Clang's `-Wno-error=keyword-macro`. The compiler options
now apply that flag only to Clang/AppleClang and the GCC-specific
`format-truncation` flag only to GNU. Local GCC 16 native and Emscripten 6.0.13
WASM CTest pass 11/11. Remote CI for this subsequent change remains to be
observed. CTest without original inputs skips the BON integration scenario;
the direct local invocation below separately passed:

```
build/client153-native/client153_skeleton_closure_test artifacts/client153-input/wolf.bon
# wolf BON: joints=42 bones=42 hooks=5
```

Known risk: file-path helpers currently allow truncation; this can select the
wrong asset and needs an explicit failure contract and consumer review. It is
not resolved by the compiler-specific warning flag.

## Path composition and package folder validation

Hypothesis: unbounded `MAX_PATH` writes and empty package folders can corrupt
memory before a real asset is loaded. Regression cases reproduced oversized path
writes and UBSan reported index -1 for empty folders in both package classes.
The composed-path helpers now reject oversized results with `std::length_error`;
their legacy `char*` callers must supply at least `MAX_PATH` bytes. Package and
image open paths reject the tested invalid input. This is a bounded repair, not
an audit of every original file API: extension helpers and other callers still
need review, and exception handling must be preserved as those callers are ported.

Validation on the local source and authorized client data:

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
build/client153-native/client153_skeleton_closure_test artifacts/client153-input/wolf.bon
build/client153-native/client153_file_family_test <authorized-client>/element/facedata.pck
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake --build build/client153-sanitize --parallel 2
UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build/client153-sanitize --output-on-failure
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_file_family_test <authorized-client>/element/facedata.pck
```

All three CTest suites passed 11/11; without asset arguments they skip original
asset integration. Direct native and sanitized PCK tests opened 2,005 entries
and verified 64 shared files. Direct skeleton test loaded 42 joints, 42 bones
and 5 hooks. These results do not establish a playable browser client.

## Disk image decryption failure

A synthetic disk file and rejecting decryption callback exposed a double free
in `AFileImage`: the failure path freed the image but retained the pointer, so
`Close` freed it again; it also leaked the file handle. The regression test
failed under AddressSanitizer before the change and passes after using the
existing `Release` cleanup and closing the file. Reproduce with:

```
cmake --build build/client153-sanitize --parallel 2
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_file_family_test
```

Native and Emscripten CTest also passed 11/11, and the direct native facedata
PCK check still opened 2,005 entries and verified 64 shared files. This test
checks failure cleanup, not the correctness of any proprietary decryption scheme.

## Original skeleton track-to-bone integration

The skeleton closure test now applies real BON tracks to the corresponding
original bones using the same position, normalized quaternion, and exclusive
blend path used by `A3DSkinModelAction`. Native and ASan/UBSan runs against the
authorized wolf BON observed 35 changing finite relative transforms across
42 bones. This exercises the actual skeleton update; it is not the action
controller, skinning, rendering, or proof of gameplay. The normal WASM CTest
passes but skips the authorized BON when no input is supplied. A direct Node
invocation with the host path failed because this build's virtual filesystem
does not mount host files; the real-asset WASM scenario remains unverified.

```
cmake --build build/client153-native --parallel 2
build/client153-native/client153_skeleton_closure_test artifacts/client153-input/wolf.bon
cmake --build build/client153-sanitize --parallel 2
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_skeleton_closure_test artifacts/client153-input/wolf.bon
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
```

## Opt-in real BON test in WASM

`PW153_TEST_BON_PATH` enables the real skeleton test without adding the asset to
Git. For WASM it preloads only this test input into the test's virtual filesystem;
it does not package assets into the game build. A missing path fails configure.
With the authorized wolf BON, CTest passed natively and under Emscripten/Node,
reporting 35 changed transforms in each. This supersedes the failed unmounted
Node attempt above; it is not a browser rendering or gameplay test.

```
emcmake cmake -S . -B build/client153-wasm-real -G Ninja \
  -DBUILD_TESTING=ON -DPW153_TEST_BON_PATH="$PWD/artifacts/client153-input/wolf.bon"
cmake --build build/client153-wasm-real --target client153_skeleton_closure_test --parallel 2
ctest --test-dir build/client153-wasm-real -R '^client153_skeleton_closure$' -V
```

The configured path should point to the locally extracted authorized wolf BON;
the build output contains that asset and must follow the project's distribution
policy. Standard asset-free CI continues to skip this optional integration case.

## Original action core and skeleton tracks

Ported `A3DSkinModelActionCore` and its original header from Angelica source
revision `e3d6d0075d0a745c9f9859f2cb25b01d71dec045` under the documented
2026-10-25 authorization. The core implementation is extracted from
`Angelica2/Angelica3D/Source/A3DSkinModelAct.cpp`; model-dependent action and
channel methods are deliberately not compiled yet. Copyright headers remain.
The real BON test now binds the original action core to the skeleton track set
and uses its `GetBoneStateOnTrack` before applying the state to each original
bone. It reports 35 changed transforms in native, ASan/UBSan, and WASM/Node.
Normal native and WASM CTest suites passed 11/11; the original BON is opt-in.
The action playback/channel and skin model are not implemented by this change,
and the production browser runtime does not yet use this core.

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
build/client153-native/client153_skeleton_closure_test artifacts/client153-input/wolf.bon
cmake --build build/client153-sanitize --target client153_skeleton_closure_test --parallel 2
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_skeleton_closure_test artifacts/client153-input/wolf.bon
cmake --build build/client153-wasm-real --target client153_skeleton_closure_test --parallel 2
ctest --test-dir build/client153-wasm-real -R '^client153_skeleton_closure$' -V
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
```

The original `A3DSkinModelActionCore` default constructor constructed a
separate temporary instead of initializing itself. A regression test calling
`SetStartAndEndFrame(0, 30)` failed before the fix; constructor delegation now
initializes 30 FPS, and the expected 1,000 ms end time passes in native,
ASan/UBSan and WASM/Node. The authorized BON test also still reports 35 changing
transforms. Commands: `cmake --build build/client153-native --parallel 2 &&
ctest --test-dir build/client153-native --output-on-failure`,
`cmake --build build/client153-sanitize --target client153_skeleton_closure_test
--parallel 2 && UBSAN_OPTIONS=halt_on_error=1
build/client153-sanitize/client153_skeleton_closure_test
artifacts/client153-input/wolf.bon`, and the opt-in WASM commands above.

## Original vertex layout for skinning

Copied `Angelica2/Angelica3D/Header/A3DVertex.h` from authorized revision
`e3d6d0075d0a745c9f9859f2cb25b01d71dec045`, retaining copyright and removing
its unconditional Windows/Direct3D platform include. Direct3D FVF macros
remain backend-specific and are not used by the portable test. Compile-time
checks establish that `A3DBVERTEX3` is 48 bytes, with matrix indices at byte 24,
normal at byte 28 and texture coordinates at byte 40, both on native and WASM.
This makes the original format definition available for the skin mesh port;
it does not implement mesh loading, skinning, or rendering.

```
cmake --build build/client153-native --target client153_math_test --parallel 2
build/client153-native/client153_math_test
cmake --build build/client153-sanitize --target client153_math_test --parallel 2
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_math_test
cmake --build build/client153-wasm-tests --target client153_math_test --parallel 2
ctest --test-dir build/client153-wasm-tests -R '^client153_math$' --output-on-failure
```
