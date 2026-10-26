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
