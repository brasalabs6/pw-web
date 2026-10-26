# Common dependencies for skeleton migration

A3DBone/A3DSkeleton/A3DCoordinate require original arrays, pointer lists, base
objects and logging in addition to math and tracks. This lot closes those
foundational components, not the skeleton implementation itself.

Source fixed at `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`. Copyrights and hashes
remain in source-manifest.json. Frozen references, server and Wine worktrees are
not modified.

## Implementation and observed defects

- Original AArray/APtrArray/AList2/APtrList compile with their complete
  operations explicitly instantiated for representative value/pointer types.
  Modern compilation exposed nonexistent APtrArray storage, a missing Append
  return, and wrapper methods forwarding the outer wrapper instead of its inner
  container. These are corrected. The integer list count initializes to zero.
- Containers owning buffers/pools prohibit implicit shallow copies; compile-time
  regression first failed before this constraint. Explicit Append/AddListToTail
  remain available. A3DBone::Duplicate in the reference already copies its child
  array explicitly. Other future port consumers needing value copies must use
  explicit operations or a reviewed deep-copy contract, not re-enable sharing
  of owning pointers. Current tracked consumers pass. No external repository
  consumes these newly introduced internal targets.
- A3DObject and A3DClassID retain original naming/ID behavior, tested against the
  original base type. This does not instantiate a bone or skeleton yet.
- ALog delivers to a real configured callback/console and actual local/VFS files.
  The native Windows debug-window operation maps to that diagnostic sink; no
  fake window messages or silent no-op are provided.
- ALog::Init returning true on failed fopen was reproduced before returning false.
  Windows file separators are normalized, with a failing regression observed
  before correcting the backslash case. Logging without an open file now reports
  failure. Release closes ownership; destruction releases a still-open stream.
- Messages are bounded to 2047 payload bytes plus prefix/newline; this truncation
  is explicit. Windows I64/I32 format normalization is shared with AString at
  platform/common/format.cpp, with existing formatting regressions retained.
  Local calendar now supplies milliseconds from the same wall-clock instant.
  Creation timestamps remain; the closing note says the stream is closing,
  without claiming fclose already succeeded. Release returns actual close status.

## Evidence and reproduction

Native GCC 16.2.1, Emscripten 6.0.13/Node and ASan/UBSan CTest: **8/8 PASS**.
The Release WASM resource runtime also rebuilds successfully. C++ fixtures do
not require original assets. Tests cover array insertion/removal, pointer array
append/element access, pooled-list concatenation/removal, object identity,
callback delivery, I64 formatting, bounded oversized logs, file failures and
file content persistence, plus all prior tests.

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake --build build/client153-sanitize --parallel 2
ctest --test-dir build/client153-sanitize --output-on-failure
PW_CLIENT153_ROOT=/path/to/authorized/app ./scripts/build-client153.sh
```

Renderer behavior is not changed by this lot; no new browser screenshot is
claimed. These tests do not prove all container edge cases, allocation-failure
paths, concurrent logger lifecycle, Unicode diagnostic parity or a complete
Common library. Skeleton closure still requires coordinates, IK/controller
interfaces, bone/joint logic, track-manager integration and resource image I/O.
Character animation, client loop and gameplay remain pending.
