# Original animation track migration

`client153_animation` explicitly compiles the original A3DTrackData template for
vector3, quaternion and float keys, including all template operations. It depends
on original math and AFile targets, not Windows umbrella headers. Copyrights and
source hashes are in the source manifest. Reference revision remains
`e3d6d0075d0a745c9f9859f2cb25b01d71dec045`.

## Behavior and tests

Test-first compilation failed before the track header existed. Once the original
logic compiled, a truncated track incorrectly returned successful Load; the
regression failed before correction. The original skip-only method named
FakeLoad likewise accepted a seek beyond EOF; its regression failed before
adding available-byte checks. That method is a real metadata skip, not a loader
stub or a fabricated successful animation.

Create/Load now bound allocations and commit key/segment arrays only on success.
Load checks byte counts, finite values, segment key bounds and time ordering.
Invalid counts fail explicitly. Copying owning tracks is prohibited. Native and
WASM compilation exposed potentially uninitialized key indices; invalid queries
now throw instead of indexing unresolved memory. Time arithmetic is widened.
The original interpolation coefficients and SLERP threshold remain unchanged.

Explicit defensive limits: 1,048,576 keys, 65,536 segments, frame rate 1–1000,
and 64 MiB of key/segment data per track. These are not a global streaming budget
or a claim that all client tracks fit. Programmatic Create requires setting keys
and segments before sampling, as in the original API. Not every interpolation,
DownSample edge case, corrupt input or reference-count lifecycle is proven.

Native GCC 16.2.1, Emscripten 6.0.13/Node and native ASan/UBSan CTest: **7/7 PASS**.
Coverage includes scalar/vector interpolation, quaternion SLERP, AFile save/load,
truncation preserving prior keys, unavailable skip and invalid creation metadata.

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake --build build/client153-sanitize --parallel 2
ctest --test-dir build/client153-sanitize --output-on-failure
```

## Real client track evidence

Authorized local input from models.pck:
`models/npcs/怪物/boss/boss血狼王/血狼王.bon`, 332879 bytes, SHA256
`f11d01783512dc0da130350fab63ee71bd4a59d737c98b6f1a057e2b9fefdca7`.
Observed BON version 5, 42 bones, 42 animation joints of class 15, five hooks,
animation frame range 0–411 at 15 fps. No asset bytes are committed.

Reproduce extraction to a new output path (the extractor refuses overwrite):

```
python3 tools/assets/pck153.py /path/to/app/element/models.pck \
  --extract 'models/npcs/怪物/boss/boss血狼王/血狼王.bon' \
  --output artifacts/client153-input/wolf.bon
build/client153-native/client153_tracks_test artifacts/client153-input/wolf.bon
build/client153-sanitize/client153_tracks_test artifacts/client153-input/wolf.bon
```

The optional native harness walks the BON v5 prefix following the original
A3DSkeleton/A3DBone/A3DJoint layouts, then delegates all 84 track loads and actual
sampling to A3DTrackData. All 42 joints contain changing keys. Sampling at 37 ms
intervals produced 31122 finite position/quaternion pairs; native sanitizer run
also passed. This checks data/algorithms, not visual animation parity.

The harness is deliberately a test of this known v5 animation-joint fixture,
not a complete BON runtime loader; it does not load/render hooks or assemble the
skeleton. No real BON bytes were mounted in a WASM/browser test in this lot.
The runtime renderer is unchanged by this PR. A3DSkeleton/A3DBone/A3DJoint,
SKI binding, skinning, action playback and character rendering remain pending.
No login, world entry or gameplay is claimed, and server/Wine worktrees were not
modified. Continue locally; ChatGPT Web is not an execution dependency.
