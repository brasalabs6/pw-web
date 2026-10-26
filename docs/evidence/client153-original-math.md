# Original Angelica math compilation

Hypothesis: retain the engine's math types and conventions for hierarchy,
skinning and animation instead of introducing a second incompatible math API.

Source revision: `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`.
Source hashes and adaptations: `src/client153/source-manifest.json`.
Original Archosaur and Epie Entertainment copyright headers are preserved.

## Implementation

`client153_math` builds the adapted original A3DVector, A3DMatrix and
A3DQuaternion classes. Their scalar formulas replace D3DX calls directly inside
the original owning operations. There is no fake D3DX library or Win32 layer.
The existing model hierarchy uses A3DMATRIX4 multiplication from this target.

Original union/anonymous-struct field layout is retained for consumers and disk
layout (12-byte vector, 64-byte matrix). These targets explicitly use compiler
extensions supported by GCC/Clang; they use Wall/Wextra/Werror but not pedantic
ISO-only enforcement. Other portable targets retain their strict flags.
Assignment operators are explicitly defaulted; matrix input pointer is const.

Two source defects were exposed by tests before fixing them:

- Determinant omitted terms; RotateZ(.7) did not have determinant 1. The fix
  expands the complete determinant using the existing 3x3 determinant helper.
- QuadToEulerAngle was empty in the upstream source. A mixed Euler rotation
  failed to roundtrip; the operation now implements roll/pitch/yaw extraction
  consistent with the original EulerAngleToQuad. Invalid zero/nonfinite norm
  raises an error instead of leaving output unchanged.

SLERP retains the original threshold/algorithm; this is not a redesign of
animation. General singular inverses, invalid quaternion inputs in other
original operations and complete numeric/corpus parity remain to be assessed.
Math compilation does not prove the skeleton or animation loader is integrated.

## Reproduction and observed evidence

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake --build build/client153-sanitize --parallel 2
ctest --test-dir build/client153-sanitize --output-on-failure
```

Native GCC 16.2.1 and Emscripten 6.0.13 tests pass. The native ASan/UBSan suite
also passes. Each suite has six tests including previous regressions. Math
coverage: layout, row-vector composition, affine inverse, determinants,
non-unit rotation-axis normalization, homogeneous divide, quaternion/matrix
conversion, SLERP and mixed Euler roundtrip.

After rebuilding the WASM resource runtime, run the browser scene tests documented
in `client153-full-build.md` for actual hierarchy consumption. Browser evidence
must come from the successful rebuilt output, not a stale output after a failed
build. This remains the original object scene, not gameplay or a complete engine.


Observed after a successful fresh runtime build: Chrome 152 desktop and mobile
emulation each pass all eight browser-scene checks. Desktop has 132880 colored
pixels/16914 unique colors; mobile 116938/16045; zero GL errors. Both use ANGLE
SwiftShader. The hierarchy's actual consumer now links client153_math. No original
assets, generated binaries or local browser artifacts are committed.
