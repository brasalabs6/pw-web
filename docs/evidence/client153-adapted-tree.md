# Adapted game source tree and the compile-first cycles

The Element client's own sources (Element/CElementClient + Element/CCommon,
2065 files) now compile from an adapted tree under `src/client153/Element/`:
the probe remaps every translation unit to the adapted copy and mirrors the
include directories so include chains stay inside the adapted tree. The
frozen reference is untouched; per-file SHA-256 hashes of the originals are
in source-manifest.json.

Source fixed at `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`.

## Compile-first campaign state (probe objects-cycle8, 787 TUs)

- 109 translation units compile to real WASM objects (up from 66 at the
  campaign start).
- Eliminated groups since the campaign start: Windows.h boundary (384),
  WM_* message constants (375), ACString (50), wchar_t width (88, resolved
  by -fshort-wchar with the 16-bit wide CRT owned by the port), typedef
  redefinitions (87, adapted headers), x86 inline asm (43, portable
  equivalents from the same files), GB18030 encoding (143, 764 files
  transcoded), AUI widget prelude (341), d3d8/d3dx8 type boundary (787 at
  the prelude).
- Remaining top groups: CompactUINT chain (142, gnmarshal int64 adapted;
  needs the socket/thread seam), size_t/unsigned-int signatures (114),
  TypeList dependent-name (31), D3DCAPS8 (20), A3DFrustum::PLANE (15),
  WTYPES.h OLE types (10), eh.h (9).

## Platform seams added (all real implementations)

- `-fshort-wchar` on every port target and the probe: wchar_t keeps the
  original MSVC 16-bit width; the port owns the 16-bit wide CRT
  (platform/common/widestr: formatter with MSVC semantics, strlen/strcmp/
  strstr/strrchr/strcspn/casecmp, tof/tod/toi/toi64) because the system
  wchar_t CRT reads 32-bit units. A static_assert enforces the flag.
- Windows.h seam: real Win32 types (guarded), CRITICAL_SECTION via the
  re-entrant recursive-mutex seam, Interlocked via std::atomic,
  GetTickCount/GetLocalTime via the platform clock, Sleep via
  std::this_thread, per-thread GetLastError slot, MessageBox as the
  diagnostic sink, MultiByteToWideChar/WideCharToMultiByte via the strict
  UTF-8/UTF-16 conversions, MSG/NOTIFYICONDATA/PALETTEENTRY structs, WM_*/
  VK_*/MK_* constants with the real Win32 values, GDI/user handle types.
- d3d8.h/d3dx8.h seams: the documented D3D8 interface signatures the client
  calls (LockRect/UnlockRect/CheckDeviceType/CreateDevice/Present...),
  D3DX math aliases over the ported A3D math, D3DLOCKED_RECT/
  D3DPRESENT_PARAMETERS/D3DDISPLAYMODE/D3DCAPS8 structs. The WebGL2 renderer
  adapter implements the interfaces in the renderer cycle.
- Case/platform forwarders: StdIO/StdLib/String/Math/Time/basetsd/io/
  tchar/process.h, Vector.h, astring.h, a3dtypes/a3dmacros/a3DFontMan/
  a3dgeometry, EC_Observer/ExpTypes/aarray, windowsx/WindowsX, AUIListbox.
- sdk_forward.h: the forward declarations the prebuilt SDK headers rely on
  (A2DSpriteItem, A3DTerrain2Render) - the original precompiled header
  supplied them.

## Evidence and reproduction

```
source /home/nonexphere/.local/share/pw-toolchains/emsdk/emsdk_env.sh
python3 scripts/probe-client153-project.py \
  --source /home/nonexphere/workspace/pw/pw-web/references/client-153 \
  --emxx /home/nonexphere/.local/share/pw-toolchains/emsdk/upstream/emscripten/em++ \
  --port-root src/client153/Angelica2 --adapted-root src/client153 \
  --output artifacts/client153-project-probe/objects-cycle8.json \
  --all --jobs 8 --object-dir /home/nonexphere/workspace/pw/clients/build/pw-browser-port/client153-original-objects
```

Port regression suites stay green: native 16/16, WASM 16/16, ASan/UBSan
16/16 (builds live on the clients volume per the disk-quota constraint).

## Limits

The probe compiles each TU independently; linking the client entry is the
next milestone. The socket/thread seams (SOCKET, WaitForSingleObject,
ReleaseSemaphore, sockaddr_in) are the current frontier and map to the
WebSocket transport per the network analysis (no overlapped I/O in the
original). WTYPES.h (OLE) and eh.h (structured exception handling) remain
unmapped pending their cycles.
