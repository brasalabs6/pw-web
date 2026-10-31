# Skeleton support: coordinates, controllers, IK and engine globals

A3DBone/A3DSkeleton/A3DJoint consume the engine's scalar helpers, geometry
bounds, coordinate hierarchy, controller and IK interfaces plus the PI globals
(`g_A3DErrLog`, axis vectors, identity matrix) and `AAssist` string IDs. This
lot closes those support dependencies; the bone/joint/skeleton classes
themselves remain for the next lot.

Source fixed at `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`. Copyrights and
per-file hashes remain in source-manifest.json. Frozen references, server and
Wine worktrees are not modified.

## Implementation and observed defects

- `AAssist` keeps the CRC32 string IDs, code-page state and the original linear
  congruential generator (multiplier 16807, modulus 2^31-1, fixed seed 765983).
  The CRC32 table is the original one, byte-verified against the reference
  before use. Wide-character conversion and memory-statistic surfaces are
  omitted until a consumer needs them.
- `AMemory` keeps the original release behavior: `A_DEBUG_NEW` maps to plain
  `new`, `a_malloc`/`a_free` map to `malloc`/`free`. The 1835-line custom
  allocator and its global `operator new`/`delete` overrides are deferred; the
  owning-container copy guard from the containers lot covers the double-free
  class it tracked.
- `ACounter` maps Windows timing (`timeGetTime`, `QueryPerformanceCounter`,
  RDTSC inline asm) to the portable monotonic clock. The CPU-cycle API reports
  zero pending consumer review, matching the pending ATime cycle API. The
  original constructor's five-second wall-clock counter validation only applies
  to Windows counter backends and is covered by the platform clock tests.
- `A3DFuncs`: `D3DXMatrixLookAtLH` is replaced by the scalar left-handed
  formula laid out like `a3d_ViewMatrix` in the same file. `memset`-on-class
  is replaced by explicit member zeroing with identical results. The DirectX
  FVF vertex-shader declarator builders and the dead `_KeepOrthogonal` helper
  are omitted; no port consumer exists yet.
- `A3DGeometry`: `A3DAABB` initializer order corrected to declaration order
  (original -Werror=reorder defect); `A3DSPLANE` copy constructor now
  initializes its base; `A3DOBB::Clear` zeroes members explicitly; a dead
  Jacobi rotation counter is removed.
- `A3DCoordinate`: the child-position handle widens from `DWORD` to
  `ALISTPOSITION` — the original cast list-position pointers to a 32-bit
  integer, which loses data on 64-bit and WASM64 targets.
- `A3DController`: the original never defined the pure-virtual destructor
  (dead code under MSVC); the port defines it out-of-line so subclasses link.
- Original IK chain methods (`A3DIKChain::Init`/`AddBone`/`RemoveBone`) are
  stubs that report success; the stub behavior is preserved, not faked.
- `A3DConfig`/`AIniFile` are deferred to the bone lot: only `A3DBone.cpp`
  consumes `g_pA3DConfig`, and `LoadConfig` pulls the file-image family.

## Evidence and reproduction

Native GCC 16.2.1, Emscripten 6.0.13/Node and ASan/UBSan CTest: **9/9 PASS**
(each configuration). The Release WASM resource runtime still builds.

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake --build build/client153-sanitize --parallel 2
ctest --test-dir build/client153-sanitize --output-on-failure
```

Tests cover: CRC32 IDs cross-checked against an independent implementation
(`test`=0xd87f7e0c, `wolf.bon`=0x94dcca54, `ASKE`=0x792d537c) and the
lowercase variant; LCG first values (2136458046, 1523801282) and range bounds;
point/rect arithmetic, containment, union/intersection and normalization;
color packing/unpacking round-trip; identity/zero/scaling/rotation/transpose/
inverse/decompose/look-at scalar helpers (look-at equals view matrix for the
same camera); AABB build/merge/point-in and OBB clear/complete; coordinate
hierarchy absolute-T propagation, relative stacking, move, child detach;
controller updates; IK stubs; counter monotonicity and the explicit
zero CPU-cycle report.

## Limits

This lot does not implement bones, joints, skeletons, skinning or any
renderer change, and no browser screenshot is claimed. The wide-character,
memory-statistic, A3DConfig and vertex-shader-declarator surfaces are omitted
with documented reasons. The IK chain remains the original stub. Character
animation, client loop and gameplay remain pending.
