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
