# Media seams and the long-tail cycles (probe cycles 10-22)

The compile-first campaign progressed from 281 to **478 of 787 translation
units compiling to real WASM objects**. The adapted tree now includes the SDK
header trees (Common/Output/include, Element/Output/include, Angelica2
Output SDK includes - 543 files copied, 52 GB18030 files transcoded), so
include chains stay inside the adapted copies.

## Seams added this stretch (all real implementations)

- `AChar.h` ported: the ACHAR macro surface routes through the 16-bit wide
  seam (`a_strlen`/`a_strcmp`/`a_sprintf`/`a_sscanf`/`a_atoi`/`a_itow`...);
  the probe prelude force-includes it like the original precompiled header.
  `wide_sscanf` implements the spec set the client uses (%d %i %u %x %f %s
  %c with widths and literal separators); `wide_itow`/`wide_i64tow` handle
  radix conversion.
- Windows.h seam: `min`/`max` as real functions (the game uses them bare AND
  `std::min` in the same units), `_wtoi`/`_wtoi64`/`_itow`/`_i64tow`,
  `timeGetTime` (guarded against the mmsystem seam duplicate),
  `ZeroMemory`/`CopyMemory`/`FillMemory`/`MoveMemory` macros, `ERROR_*`
  codes, `CreateDirectoryA` over POSIX mkdir, `OutputDebugStringA/W` as the
  diagnostic sink, `GetKeyState`/`GetAsyncKeyState` over the browser input
  adapter's real key-state table, `LPBYTE`, `MB_OKCANCEL` family,
  `NOTIFYICONDATA`/`PALETTEENTRY`/`MSG`/`GUITHREADINFO` structs, GDI handle
  types, `CP_ACP` family.
- `dsound.h` seam: the documented DirectSound8 surface the client's media
  library compiles against - IDirectSound8/IDirectSoundBuffer8 with
  Play/Stop/Lock/Unlock/SetCurrentPosition/GetCurrentPosition/SetVolume/
  SetFrequency/SetPan/GetStatus/GetFrequency/GetVolume/Restore,
  IDirectSoundNotify(8) with DSBPOSITIONNOTIFY, IDirectSound3DBuffer(8),
  WAVEFORMATEX/DSBUFFERDESC/DSCAPS/DSBCAPS, the DS_OK/DSERR/DSBPLAY/DSBLOCK/
  DSBSTATUS/DSSCL/DSBVOLUME/DS3DMODE constants, FAILED/SUCCEEDED, and the
  DirectSoundCreate8 factory declaration (the WebAudio adapter implements
  it in the audio cycle).
- `dshow.h` seam: the DirectShow video-clip surface - IGraphBuilder/
  IMediaControl/IMediaEventEx/IMediaPosition/IBasicAudio with the
  documented signatures, REFERENCE_TIME/REFTIME, EC_* event codes (the
  HTML5 media adapter implements it in the media cycle).
- `wtypes.h`/`WTYPES.h` seam: the OLE base types (the includers use none of
  the COM machinery - verified).
- `mmsystem.h`/`mmreg.h` seams: timeGetTime over the platform clock; the
  period functions return the backend's real semantics (no adjustable
  timer period in this environment).
- `d3d8.h`: ID3DXBuffer (LPD3DXBUFFER), D3DCAPS8, D3DDISPLAYMODE, D3DRECT,
  D3DXPLANE alias over the ported A3DPLANE.
- Case/platform forwarders: `windowsx.h`, `Alist2.h`, `a3dflatcollector.h`,
  `D3dx8core.h`, `Vector.h`, `astring.h`, `a3dtypes.h`, `a3dmacros.h`,
  `a3DFontMan.h`, `a3dgeometry.h`, `EC_Observer.h`, `ExpTypes.h`,
  `aarray.h`, `process.h`, `io.h`, `tchar.h`, `basetsd.h`, `StdIO.h`,
  `StdLib.h`, `String.h`, `Math.h`, `Time.h`, `AUIListbox.h`.

## Adapted-source fixes (the frozen reference is untouched)

- 81 TUs: 328 explicit casts for AWString/AString through variadic calls
  (DeepSeek subagent; callee signatures verified; 0 varargs errors remain).
- `EC_NPC.h`/`EC_Scene.h`: exact forward declarations for CECNPCMan/
  CECWorld (the original relied on include order).
- Multi-word function-style casts (`unsigned long(expr)` ->
  `(unsigned long)(expr)`); enum forward declarations and definitions with
  the `: int` underlying type; `typename` for dependent names
  (A3DGFXCachedItemMan.h); `gmtime` calls through `const time_t*` casts;
  trailing-space include paths; backslash include paths; `A3DPlants.h`
  const-reference parameters (the original bound temporaries to non-const
  references).

## Reproduction

```
source /home/nonexphere/.local/share/pw-toolchains/emsdk/emsdk_env.sh
python3 scripts/probe-client153-project.py \
  --source /home/nonexphere/workspace/pw/pw-web/references/client-153 \
  --emxx /home/nonexphere/.local/share/pw-toolchains/emsdk/upstream/emscripten/em++ \
  --port-root src/client153/Angelica2 --adapted-root src/client153 \
  --output artifacts/client153-project-probe/objects-cycle22.json \
  --all --jobs 8 --object-dir build/client153-original-objects
```

Port regressions: native 29/29. Remaining top groups: function-style casts
in newly-reached TUs (30), eh.h/SEH (23), residual varargs (14), atlbase.h
(6), tlhelp32.h (6), and small per-file patterns (MSVC for-scoping `i`,
TYPE_NORMAL, address-of-temporary, member redeclaration).

## Limits

The probe compiles each TU independently; the link milestone follows. The
audio/video adapters (WebAudio/HTML5 media) implement the dsound/dshow
interfaces in their own cycles. The eh.h/SEH, ATL and toolhelp groups are
next; the clients volume holding the authorized assets was rebuilt by the
server agent (root-owned) - asset-dependent tests skip by design until the
assets return.
