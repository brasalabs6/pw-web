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
