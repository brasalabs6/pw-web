# Original client compilation probe

Hypothesis: retain the original client systems and migrate their platform boundaries,
rather than rebuild the game around independently adapted resource loaders.

Pinned source: `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`.
Project: `Element/CElementClient/ElementClient.vcproj`, `Release|Win32`.

## Observed evidence

The project enumerates 787 C/C++ translation units. All 787 exist after resolving
Windows case-insensitive paths explicitly. There are 42 link dependencies.
Two include directories are absent even after case resolution:
`Angelica2/Output/apSDK/Include` and
`Angelica2/Output/3rdsdk/include/dshow`.
Their absence does not prove a required runtime component is missing: determine
which active source includes their headers before choosing replacements.

Emscripten 6.0.13 syntax compilation of the original `ElementClient.cpp`, with
original Release definitions and include directories, fails at:

```
EC_Global.h -> ALog.h -> ACPlatform.h:16: Windows.h file not found
```

This is an expected **failed compilation**, not a complete dependency closure.
No fake Windows headers, disabled diagnostics, SDK binary linking or no-op
implementations were used. The reference source was not modified.

## Original runtime boundaries

`ElementClient.cpp:728` implements `wWinMain`. Its startup builds
`CECGame::GAMEINIT`, creates a native window, initializes Lua, calls
`CECGame::Init`, then `CECGameRun::StartLogin`, creates render/defence threads
and enters a blocking loop. This entry point cannot run unchanged in a browser.

`EC_Game.cpp:1248` and `:1272` implement `RunInRenderThread` and
`RunInMainThread`. `:1417` dispatches PeekMessage. `:1455` initializes the engine.
These are the concrete boundaries to separate into browser scheduling/input and
GLES initialization while retaining the game/session logic. Moving a loop alone
does not make the engine portable; its renderer, files, audio and network remain
required implementations.

## Reproduction

```
python3 scripts/probe-client153-project.py \
  --source /path/to/pinned/client-153 \
  --emxx /path/to/emsdk/upstream/emscripten/em++ \
  --output artifacts/client153-full-build/original-project.json
```

Exit code is the compiler's exit code. The local report contains exact compiler
arguments and local paths; it is an ignored artifact and must not be published
as a sanitized release manifest. Source paths in the inventory are relative.

## Next compilation work

Build the source library dependency graph from VC projects, resolve actual
header producers, and introduce explicit source targets under `src/client153`.
Separate AC platform types/logging/locking from Windows headers before attempting
higher layers. Preserve game algorithms and actual error behavior. Implement
platform operations at their owning boundary; do not build a general Win32 or
D3D emulator. Renderer/device work must support real consumers and cannot claim
success by returning true without performing their operations.

The MOX/DDS scene remains useful regression evidence. It is not the client build.
The draft SKI fixture is parked outside CMake while the full-client dependency
path is evaluated. No SKI implementation or character animation is claimed.

## Common-library probe

The same probe supports `--project`, `--configuration` and `--entry`.
`Angelica2/AngelicaCommon/AngelicaCommon.vcproj`, configuration
`Unicode Release|Win32`, enumerates 17 existing translation units.
Syntax compilation of `Source/AString.cpp` reaches three concrete errors:
`_atoi64`, `stricmp`, and undeclared `wcslen`. It also exposes a promoted-char
`va_arg` warning and mismatched delete exception specifications. These require
explicit portable implementations or source corrections, not synthetic headers.
In particular, varargs handling needs ABI tests before reusing string formatting.

Validation: `python3 -m unittest discover -s tests/client153 -p 'test_*.py'`
passes three path-discovery tests (case resolution, missing directories, ambiguous
case collisions). Existing native MOX/DDS CTest remains 2/2 passing. Neither test
suite establishes that the full client compiles.

## First original Common component compiled

`client153_common` now compiles the original `AString` public interface and
copy-on-write implementation from tracked adapted sources. This is the first
Common component, **not** the complete Common library or a linked game.
The frozen reference remains untouched; the source manifest records hashes.

Changes remove the Windows allocator header dependency in favor of C++ array
allocation; fix array deallocation type and the locked copy constructor's
uninitialized storage read; use byte-safe copy/comparison; preserve signed 64-bit
conversion with `long long`; and replace the x86-specific varargs estimator with
bounded libc formatting. MSVC `I64`/`I32` printf lengths are normalized at this
boundary. Rendering happens in temporary storage before assignment so source
arguments may alias the destination. Format failures raise exceptions rather
than return fabricated successful output.

Tests were added first and failed because `AString.h`/the library did not exist.
After the adaptation, CTest passes 3/3 on GCC 16.2.1, Emscripten 6.0.13/Node,
and native AddressSanitizer+UndefinedBehaviorSanitizer builds. Coverage includes
copy-on-write, numeric conversion, character/precision/I64 formatting, aliasing,
9 KB formatting, case conversion, slicing, trimming and buffer release.

Commands (activate the documented compiler/CMake toolchain first):

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
emcmake cmake -S . -B build/client153-wasm-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake -S . -B build/client153-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer' \
  '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'
cmake --build build/client153-sanitize --parallel 2
ctest --test-dir build/client153-sanitize --output-on-failure
```

Remaining: other Common classes, Unicode `AWString` and the client-wide
Windows UTF-16/wchar ABI, platform logging/time/locking, and all higher-library
closure. This test is not proof of every printf variant or every original
AString edge case. No server contract changed; no game assets were committed.

## Platform clock boundary

The original game loop calls `a_GetTime` in `EC_Game.cpp:1321,1326`.
`ATime` now delegates to `client153_platform`: an actual steady clock and local
calendar backend. This preserves unsigned 32-bit millisecond deltas and the
original nullable calendar outputs without pulling ACPlatform/Windows into
Common. ABaseDef DWORD/LONG explicitly retain their original 32-bit widths on
native LP64 too. The monotonic epoch is process-local and not a wall timestamp;
consumers must use elapsed differences as the original game loop does.

The original ACounter CPU-cycle API is not compiled or replaced with fake cycle
values. Its consumers and profiling units remain a subsequent dependency.
No busy-wait calibration or anti-speed-check result is fabricated by this port.
Calendar failures throw; they do not return invented dates.

RED: new time test could not compile before ATime was provided.
GREEN: native and Emscripten/Node CTest 4/4. The time test checks real elapsed time,
monotonic observations, fixed-width types, calendar fields and null outputs.
This is not yet a browser main-loop integration or a complete Common library.

## Angelica File dependency

`client153_file` now compiles a portable adaptation of the original `AFile`
interface against Common. Real operations include file open/close/read/write,
MOXB/MOXT header handling, null-terminated and length-prefixed strings,
line operations, seek, length, rewind and flush. The owning stream cannot be
copied. Paths normalize Windows separators and preserve the original 260-byte
capacity. Filesystem operations run in local libc/VFS; they never synchronously
fetch from the network.

New tests first failed because AFile was not present. Native, Emscripten/Node
and ASan/UBSan CTest pass 5/5 after implementation. The file regression verifies
original MOXB framing and AString bytes, zero-offset ResetPointer, truncated
length-prefixed and missing-terminator failures, missing files and closed seek.
Truncated AString leaves the caller's string unchanged. Its length is bounded
to 16 MiB, an explicit defensive limit requiring corpus validation.

This is not full AngelicaFile: AFileImage, PCK runtime mounting, async resource
availability, eviction and streaming remain pending. At the initial native checkpoint the object-viewer host did not consume this
library. The subsequent browser integration below enables a temporary resource
VFS. No asset package is embedded in a .data file; full CDN streaming is pending.
No server contract or parallel worktree was modified.

### Real-file integration check

The native file test accepts a local original MOX path:

```
build/client153-native/client153_file_test artifacts/client153-input/cabinet.mox
```

Observed PASS: AFile detected MOXB, read the frame label, rewound to byte zero,
read 1199 bytes, and the adapted MOX decoder produced two frames and one mesh.
Repeating this path for the 336 distinct hash-named files in the local extracted
corpus that begin MOXB plus FRAME/VERSION returned 336 PASS and zero failures.
This count describes distinct selected files, not the earlier package-entry
count, and excludes MOXT/building envelopes. Report:
`artifacts/client153-full-build/afile-corpus.json` (ignored, local only).
WASM/Node regressions remain 5/5; the real-file corpus check is native only.


## Browser integration of original file interface

The browser target now links client153_file/Common and enables Emscripten's
memory VFS explicitly. The host downloads and verifies the selected model,
writes that single resource to `/scene.mox` inside the WASM VFS, then calls
`client_load_file`. C++ opens it through AFile and passes the original bytes to
the MOX decoder. A finally block removes the staged file on success or failure.
This does not expose any host filesystem or serve the installation root.
Textures retain their existing verified byte-upload path.

The browser regression was extended first and failed against the old runtime
because the AFile path was absent. After implementation, Chrome 152 desktop and
mobile emulation render the same original pixels with zero WebGL errors; model
load reports AFile/VFS and temporary-file absence. Existing corrupt-download,
valid-hash malformed-model, GPU allocation and context restoration tests pass;
malformed-model failure also removes its staged file. SwiftShader is the actual
renderer, not evidence of hardware GPU performance. WASM/Node CTest is 5/5.

Reproduction after activating Emscripten/CMake:

```
PW_CLIENT153_ROOT=/path/to/authorized/app ./scripts/build-client153.sh
python3 -m http.server 8497 --bind 127.0.0.1 --directory build/client153-wasm/browser
PW_BROWSER_MODULE_DIR=/path/to/playwright-install CHROME_BIN=/path/to/chrome node tests/client153/browser-scene.cjs
PW_BROWSER_MODULE_DIR=/path/to/playwright-install CHROME_BIN=/path/to/chrome PW_CLIENT153_MOBILE=1 PW_CLIENT153_EVIDENCE=artifacts/client153-browser-mobile node tests/client153/browser-scene.cjs
```

This bridges a real original interface into the browser and remains a resource
scene. It is not the original complete client loop, runtime PCK mount, memory
budgeted streaming, world entry or gameplay. Temporary VFS/data copies need
budgeting before large-resource use. No server or Wine worktree changed.

## Direct whole-client compilation recheck

At the user's request to prioritize a broad build attempt, repeated the original
VC-project inventory with Emscripten and attempted an actual WASM object build
of `ElementClient.cpp` using the project's include paths and definitions. The
project enumerates 787 translation units; all source paths resolve, while 2 of
29 configured include directories do not exist. The first entrypoint compile
fails at `ACPlatform.h` because `Windows.h` is unavailable. The project also
links Windows-only APIs and Direct3D libraries, which the browser/WASM target
cannot consume directly. The missing directories alone are not proof of a
missing runtime dependency.

An additional sample of twelve original client translation units (including
world, game loop, login and UI) all failed syntax compilation: nine first at a
Windows header and three first at the MSVC-specific `_atoi64` in an original
SDK header. This is a sample, not an exhaustive error inventory and not a
successful game build. Asset conversion alone does not address these compiler,
linker and browser-platform dependencies.

```
python3 scripts/probe-client153-project.py --source <pinned-reference> \
  --emxx <emsdk>/upstream/emscripten/em++ --output <local-output>/project.json
```

The original project report contains absolute local paths and is retained only
as an ignored local artifact. A full browser port still requires replacing or
adapting the Windows, graphics, network and other platform boundaries while
reusing the game code and authorized assets.

## Unicode width needed for broad build

The original Unicode client defines `ACHAR` as Windows `wchar_t` and directly
uses `sizeof(ACHAR)` in file and character-data paths (for example,
`DlgFriendHistory.cpp` and `EC_Global.h`). On the native and WASM targets,
`wchar_t` is 32 bits, so the previously ported alias was incompatible with the
original two-byte Windows code units. A regression test failed before the
change. The portable `UNICODE` alias now uses `char16_t` and UTF-16 literals;
non-Unicode builds retain their prior alias. Native and WASM suites pass 14/14,
and all currently ported libraries also build with `-DUNICODE`.

This only establishes the character width and literal behavior. `AWString`,
code-page conversions, actual file encoding and network serialization still
need porting and direct compatibility tests; neither server compatibility nor
the original client build has been proven. Functions expecting platform
`wchar_t` must be adapted explicitly rather than reinterpreting buffers.

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake -S . -B build/client153-unicode-probe -G Ninja -DCMAKE_CXX_FLAGS=-DUNICODE -DBUILD_TESTING=OFF
cmake --build build/client153-unicode-probe --parallel 2
```

## Broad original-project syntax inventory

The opt-in `--all` probe checked all 787 original project translation units
with their original configuration. It reported 47 successful syntax checks,
461 first failures at `Windows.h`, 134 at the MSVC-specific `_atoi64`, 57 at
an incompatible `vector<void *>` constructor, and 23 at lowercase `windows.h`;
other first failures include Windows headers and missing include names. These
are only each file's first reported error: removing one can reveal others.
Syntax success is not a successful compile/link or proof of runtime behavior.
The generated detailed report includes local paths and remains an ignored local
artifact. Repeat with `--port-root src/client153/Angelica2` to prioritize the
already adapted common/file/3D headers and measure the next errors.

```
python3 scripts/probe-client153-project.py --source <pinned-reference> \
  --emxx <emsdk>/upstream/emscripten/em++ --output <local-output>/all.json --all --jobs 4
```

## Original hash map dependency

`EC_RTDebug.h` includes `hashmap.h`. The port lacked this original header, so
the broad entrypoint probe fell back to the unported SDK header and produced
errors in that SDK's hashtable. Copied the original `hashmap.h` from the
pinned source (copyright retained), using the already ported `hashtab.h`.
A behavior test failed to compile before the copy and now verifies insertion,
lookup and erase on native, WASM/Node and ASan/UBSan. Re-running the entrypoint
probe with port headers got past that hashtable error; it now reports missing
`CRITICAL_SECTION`, `ACString` and `basetsd.h` among its first errors. This is
not a successful full client build.

```
cmake --build build/client153-native --target client153_common_test --parallel 2
build/client153-native/client153_common_test
cmake --build build/client153-wasm-tests --target client153_common_test --parallel 2
ctest --test-dir build/client153-wasm-tests -R '^client153_common$' --output-on-failure
cmake --build build/client153-sanitize --target client153_common_test --parallel 2
UBSAN_OPTIONS=halt_on_error=1 build/client153-sanitize/client153_common_test
python3 scripts/probe-client153-project.py --source <pinned-reference> \
  --emxx <emsdk>/upstream/emscripten/em++ --port-root src/client153/Angelica2 \
  --output <local-output>/entry.json
```

## Port platform include path

The broad probe's port-header mode omitted `src/client153/platform/common`,
so some original source files could not find the already ported, real
`criticalsection.h`. Added that include directory and a regression test for
its discovery. Rechecking the 13 files whose first diagnostic had been the
missing header: six passed syntax checking; seven reached other errors,
including Windows headers, a forward-declared enum, a 32-bit pointer type
mismatch and a reference binding mismatch. The previous broad report mixed
header revisions during execution, so these figures apply only to this
explicitly selected subset, not to the entire project. No client was linked.

```
python3 -m unittest discover -s tests/client153 -p test_project_probe.py
python3 scripts/probe-client153-project.py --source <pinned-reference> \
  --emxx <emsdk>/upstream/emscripten/em++ --port-root src/client153/Angelica2 \
  --entry DataPathMan.cpp --output <local-output>/datapath.json
```

## Compile real WASM objects

The project probe accepts `--object-dir` with `--all` to compile actual
WebAssembly object files, using stable path-derived names to avoid collisions.
An initial compile of the 13 source files previously blocked by the missing
portable-platform include produced six WASM object files; the other seven
still failed on their next compilation errors. `file` identified the
`DataPathMan.cpp` object as WebAssembly. Objects are not linked, and success
is not a runnable client. A full-project object compilation has been started;
its outcome must be checked separately.

```
python3 -m unittest discover -s tests/client153 -p test_project_probe.py
python3 scripts/probe-client153-project.py --source <pinned-reference> \
  --emxx <emsdk>/upstream/emscripten/em++ --port-root src/client153/Angelica2 \
  --output <local-output>/objects.json --all --object-dir <local-build-dir> --jobs 4
```

In a separate syntax check of 15 files whose initial error was
`CRITICAL_SECTION`, forcing inclusion of the existing real portable mutex
header revealed `ACString` as the first error in every file. This was a
probe, not a permanent preinclude or proof of linked synchronization behavior.
