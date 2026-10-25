# First Angelica mesh draw: source and dependency closure (partial)

Date: 2026-10-25. Reference: `references/client-153` at pinned commit
`e3d6d0075d0a745c9f9859f2cb25b01d71dec045`. This report records source paths,
symbols and the first compiler boundary; it does not copy proprietary source
into the port and does not claim a completed compiler closure.

## Version identity needs confirmation

The pinned repository README identifies itself as “Perfect World Client-Source
1.5.3” and says Visual Studio 2005 is required, but also describes the contents
as a customized 1.5.2–1.5.5 version. `Element/PW.sln` is a VS2005 solution, and
`Element/CElementClient/ElementClient.vcproj` contains a Release output path
named `Perfect World 1.5.2 BR`, `Release2.2` output/intermediate directories,
and 2.2 libraries such as `AudioEngine2.2_lib`. This metadata does not establish
which game protocol or exact release the client code implements. A build/hash or
runtime version baseline is still required before naming it a verified 1.5.3
client.

## Source-backed call path

The source files exist in the fixed reference:

1. `Angelica2/Angelica3D/Source/A3DMesh.cpp`: `A3DMesh::Init` allocates mesh
   arrays and initializes its stream as `A3DVT_VERTEX`; `SetVerts` and
   `SetIndices` feed that stream; `RenderDirect` calls `A3DStream::Appear` and
   then `A3DDevice::DrawIndexedPrimitive`.
2. `Angelica2/Angelica3D/Source/A3DStream.cpp`: the `A3DVT_VERTEX` case chooses
   `sizeof(A3DVERTEX)` and `A3DFVF_A3DVERTEX`. `Appear` binds vertex/index
   buffers and applies the FVF through `SetDeviceVertexShader`.
3. `Angelica2/Angelica3D/Source/A3DDevice.cpp`:
   `A3DDevice::DrawIndexedPrimitive` eventually calls the Direct3D 8 device's
   `DrawIndexedPrimitive`, but also updates engine performance records and
   vertex-shader texture-coordinate state.
4. `Angelica2/Angelica3D/Header/A3DVertex.h`: `A3DVERTEX` fields are position
   (3 floats), normal (3 floats), and UV (2 floats): 32 bytes with the current
   declarations. The FVF is XYZ | NORMAL | TEX1. The R0 `VertexPC` (position +
   packed color) is not layout- or semantic-compatible; a cast is invalid.
   `A3DLVERTEX` is a separate XYZ + diffuse + specular + UV format.
5. `A3DStream::Init` defaults index storage to `WORD` unless
   `A3DSTRM_INDEX32` is explicitly set. `A3DMesh::Init` passes reference-pointer
   flags for both buffers; `SetVerts`/`SetIndices` still copy into the D3D
   buffers on the ordinary device path.

The minimal API call path is not yet a closed standalone renderer. It depends
on device/context creation, vertex-format conversion and normal handling,
matrix state, upload/lifetime, draw state, diagnostics, allocator/config globals,
and any library code linked into the client.

### False-success paths that must stay disabled

- `A3DMesh::Init` returns success after setting `m_bHWIMesh` when run environment
  is `A3DRUNENV_PURESERVER`; subsequent render functions can return success
  without drawing.
- `A3DMesh::RenderDirect` returns success for `m_bHWIMesh`.
- `A3DDevice::DrawIndexedPrimitive` returns success immediately for
  `m_bHWIDevice`.

These server/hardware-independent branches are not evidence of a browser draw
and must not be used to satisfy the renderer test.

## Compiler evidence and dependencies

An Emscripten 6.0.13 syntax-only probe was run against the original mesh source:

```bash
source /tmp/pw-web-emsdk/emsdk_env.sh
em++ -std=c++17 -fsyntax-only \
  references/client-153/Angelica2/Angelica3D/Source/A3DMesh.cpp \
  -Ireferences/client-153/Angelica2/Angelica3D/Header \
  -Ireferences/client-153/Angelica2/AngelicaCommon/Header \
  -Ireferences/client-153/Angelica2/AngelicaFile/Header \
  -Ireferences/client-153/Angelica2/AngelicaMedia/Header
```

Result: **failed; no object emitted**. The compiler reports `__int64` and
`_atoi64` in `AngelicaCommon/Header/AString.h`, and then cannot find
`Windows.h` from `Angelica3D/Header/A3DPlatform.h`. This proves the current
Emscripten invocation lacks compatibility definitions and Windows/Direct3D
platform headers; it does **not** prove those source implementations are
missing. Adding typedef shims or fake Windows/Direct3D headers would conceal
more of the closure and is not accepted as a port.

The native client project itself declares Win32, Visual Studio 2005, Direct3D 8
(`d3d8`, `d3dx8`), and many middleware libraries. The exact project configuration
and dependencies for the intended 1.5.3 baseline have not been established, and
the Win32 build has not run. Static project library strings are not proof that
the required matching libraries are present.

## Status and next evidence needed

- Source presence: `A3DMesh`, `A3DStream`, `A3DDevice`, and their headers are
  present at the pinned commit.
- Compiler closure: **incomplete**; Emscripten stops at Windows-specific types
  and platform headers before enumerating downstream symbols/libraries.
- Original Windows baseline: **not run**; requires the matching VS2005/Win32
  toolchain, DirectX 8 SDK and middleware referenced by the exact chosen project
  configuration.
- First real draw: **not run**; no real mesh fixture or Windows reference pixels
  are available in this checkout. The synthetic R0 browser report is separate.
- Version/protocol identity: **unverified**; README range and project metadata
  conflict with a clean, exact 1.5.3 label.

Next: select and hash the intended 1.5.3 project/configuration and known-good
client executable; reproduce its native build with the correct matching SDKs;
then use compiler dependency output on a minimized ordinary-device mesh target
to close headers, symbols, libraries, and D3D calls. Only after that can the
port adapter and a same-input native/browser pixel comparison be specified.

## Source layout for binary Angelica mesh input

The pinned source provides a concrete next data path even though this checkout
contains no mesh or texture asset. `A3DMesh::Save` and `A3DMesh::Load` in
`Angelica2/Angelica3D/Source/A3DMesh.cpp` establish the binary `.mox` record:

1. NUL-terminated `VERSION: n`, then NUL-terminated `MESH: name`.
2. Little-endian Win32 `int` counts for vertices, indices and frames; one-byte
   Win32 `bool` values for LOD and wire flags.
3. Version 1 properties use a size-prefixed legacy `A3DMESH_PROP`; versions 2
   and 100 serialize `A3DMESHPROP` (DWORD flags and material enum).
4. `uint16` indices; optional LOD distance floats, limit integer and `uint16`
   map table; then each frame's `A3DVERTEX` records, eight float32 values per
   vertex (position XYZ, normal XYZ, UV).
5. Optional `DETAILTEXTURE: name` and a 4x4 float32 matrix; then
   `TEXTURE: name`; then `MATERIAL: name`, four float32 RGBA values in serialized
   order ambient/diffuse/emissive/specular, float power and one-byte two-sided.
6. Version 100 appends two float32 lightmap UV values for each vertex in every
   frame.

The layout above is traced from the corresponding `Save` and `Load` calls, plus
`A3DVERTEX`, `A3DMESHPROP`, `A3DMESH_PROP`, `A3DLIGHTMAPCOORD` and
`A3DMaterial::Save/Load` in the same fixed submodule. `Save` writes v2 or v100;
`Load` also handles v1 and text mode. It does **not** prove that the target 1.5.3
content set uses only these binary variants. The parser must bound the v1
size-prefixed legacy property before reading it, retain the distinction between
format versions, and reject unknown/truncated data. No raw proprietary source
was copied into the port for this inventory.

No `.mox`, `.pck`, `.pkx`, texture, map or animation data was found in the
available workspace, Downloads, Desktop, `.local/share`, `/mnt`, `/media` or
`/run/media`. Therefore this source-derived format is a decoder prerequisite,
not a real asset or scene proof. The user confirmed the 1.5.3 reference source
is all that is currently available; a browser pixel comparison still needs an
authorized mesh plus its texture and a baseline or a separately approved
source-derived expected image.
