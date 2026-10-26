# Skeleton closure: bones, joints, controllers, track manager

The original skeleton family now compiles and loads real data:
`A3DConfig`, `A3DJoint`, `A3DBone`, `A3DBoneController`, `A3DTrackMan` and
`A3DSkeleton` — the classes that own the character hierarchy and animation
state.

Source fixed at `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`. Copyrights and
per-file hashes remain in source-manifest.json. Frozen references, server and
Wine worktrees are not modified.

## Real-data proof

`A3DSkeleton::Load` opens the authorized wolf BON through the original
`AFileImage` path and builds **42 joints, 42 bones and 5 hooks**, matching
the joint count the validated tracks test already proved on the same file
(84 tracks / 31,122 samples). The track manager registers the skeleton's
track set (`GetSkeletonTrackSet` resolves after `Load`), `Update` walks the
bone tree, and the negative case (missing file) fails without side effects.

## Implementation and observed defects

- **Re-entrant lock seam**: the first run deadlocked. Windows
  `CRITICAL_SECTION` is re-entrant per thread and the track manager nests its
  table lock (`CreateSkeletonTrackSet` → `LoadBoneTrack`); the seam now uses
  `std::recursive_mutex` with the same init/enter/leave surface.
- `A3DEngine`/`A3DSkinModel`/`AF.h` includes are omitted: the only engine use
  was performance-record instrumentation; the skin-model whole-scale
  notification (`OnBoneWholeScaleChanges`) is a documented pending seam that
  reconnects when the skin family lands (`m_pSkinModel` stays NULL until
  then, so the call is unreachable).
- `A3DTrackData::m_dwDelFlag1` widens from `DWORD` to `ALISTPOSITION`
  (the original stored a list-position pointer in a 32-bit integer);
  `A3DSkeleton::GetRootBone` returns `-1` instead of `NULL`-into-`int`.
- Initializer order fixed in `A3DSkeletonHook`, `A3DBone`,
  `A3DBoneLookAtController` and `A3DBoneBodyTurnController`; `memset` on
  non-trivial structs (`HOOKDATA`, `BONEDATA`) replaced by explicit zeroing
  with identical bytes.
- `AString` passed to varargs made explicit in error logs (UB the original
  relied on); `stricmp` → `strcasecmp`; dead locals removed (`mattemp1`,
  `mattemp2`), unused-but-kept originals marked with `(void)`.
- `A3DConfig` key table made `const`; `A3DJoint` base stubs keep the
  original behavior; `A3DSkeleton::Clone`'s `return false` in a pointer
  function becomes `return NULL` (the MSVC behavior).

## Evidence and reproduction

Native GCC 16.2.1, Emscripten 6.0.13/Node and ASan/UBSan CTest: **11/11 PASS**
(each configuration).

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
./build/client153-native/client153_skeleton_closure_test artifacts/client153-input/wolf.bon
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake --build build/client153-sanitize --parallel 2
ctest --test-dir build/client153-sanitize --output-on-failure
```

## Limits

The skeleton is not yet attached to a skin mesh: skinning, the animated
character render, IK solving beyond the original stubs, and the browser
integration of this hierarchy remain for the skin milestone. `A3DConfig`'s
`LoadConfig` reads `Configs/angelica.cfg` through `AIniFile` (ported); the
engine startup that calls it is part of the client-shell milestone.
