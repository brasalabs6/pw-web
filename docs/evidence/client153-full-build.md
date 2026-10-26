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
