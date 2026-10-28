# Evidence: Wininet TU group + ElementClient entry TU + wide CRT ownership

Date: 2026-10-27
Branch: `feat/client153-common-containers` (PR #23)
Probe: `artifacts/client153-project-probe/objects-resume2.json` command, emsdk
Emscripten 6.0.13, `-fshort-wchar -fms-extensions -std=c++17`.

## Hypothesis

The Wininet/BBS dialog TU group and the game's entry TU (`ElementClient.cpp`)
are blocked by a bounded set of common Win32/COM/ATL/CRT dependencies. Porting
those at the owning boundary (the `platform/common` seam) with real
implementations or documented explicit failures unblocks them and a wider set
of TUs that use the same surface.

## What was ported (all real implementations or documented explicit failures)

- `WaitForMultipleObjects` moved from `Windows.h` to `winsync.h` (it referenced
  `WaitForSingleObject`/`Sleep` before their definitions); `waitAll` now
  reports `WAIT_OBJECT_0` only when every handle is signaled, and the array
  wait polls without consuming auto-reset signals (`PollSignaled`).
- Thread surface: `CreateThread` (template start routine so both `DWORD WINAPI`
  and `unsigned long WINAPI` game thread functions bind; `CREATE_SUSPENDED`
  defers the std::thread launch to the first `ResumeThread`), `ExitThread`
  (unwinds via the `ThreadExit` token caught by the entry wrappers),
  `GetExitCodeThread` + `STILL_ACTIVE` (the wrapper records the start
  routine's return), `GetCurrentThreadId` (real per-thread ids),
  `SuspendThread` (mid-flight suspend is not representable over std::thread —
  fails `(DWORD)-1`), `SetThreadPriority` (fails; no scheduler priorities).
- Message surface: plain `SendMessage`/`PostMessage` (posting fails: no Win32
  message queue; the adapter's dispatch replaces the pump at m-boot),
  `GetMessage`/`PeekMessage`/`TranslateMessage`/`DispatchMessage`,
  `EnumChildWindows` (no child windows: callback never invoked, success).
- Windowing: `IsIconic`/`IsZoomed`/`SetActiveWindow`/`GetActiveWindow`,
  `MENUBARINFO`/`OBJID_*`/`GetMenuBarInfo`, `SWP_*`/`HWND_*`,
  `WS_*`/`WS_EX_*` (moved from `atlwin.h` to the umbrella), `CS_*`,
  `WNDCLASSEX(W)`/`WNDPROC`/`RegisterClassEx` (fails: no Win32 window
  classes; the host supplies the canvas), `CREATESTRUCT`,
  `CreateWindow(Ex)(A/W)` (fails: no fake HWND), `AdjustWindowRect`
  (borderless canvas: no-op success), `GetSystemMetrics` (0 until the canvas
  adapter registers), `MoveWindow`/`ShowWindow`/`UpdateWindow`/
  `SetForegroundWindow`/`SetCursor`, `FindWindow(A/W/plain)`,
  `LoadCursor`/`LoadIcon`/`GetStockObject` (none: the host owns cursor/icon
  rendering), `MessageBox` wide overload + `MessageBoxW` fixed to `wchar_t*`.
- Process surface: `OpenProcess` (fails: no other processes),
  `GetCurrentProcessId` (1), `GetProcessHeap`/`HeapAlloc`/`HeapFree` (the C
  malloc heap), `GetClassNameA`, `EnumWindows`, `SystemTimeToFileTime` (the
  real civil-date conversion), `MEMORY_BASIC_INFORMATION`/`VirtualQueryEx`,
  `STARTUPINFO(W)`/`PROCESS_INFORMATION`/`CreateProcess` (fails: no native
  process spawn), `GetVersion` (the documented 5.1 build-2600 encoding),
  `OSVERSIONINFOA` (narrow; ImeUi reads szCSDVersion with lstrlenA) +
  `OSVERSIONINFO`/`OSVERSIONINFOEX` (wide; the UNICODE build) +
  `GetVersionExA/W/plain`, `ExitProcess` (quit code + runtime exit).
- COM/ATL: `GUID` + `IsEqualGUID`/`IsEqualIID` + `__uuidof` support
  (`PW153_COM_UUID` guarded to MSVC/clang `-fms-extensions`; nested `#if` so
  GCC's preprocessor never lexes `__is_identifier`), the full documented
  `IBindStatusCallback` method set in `exdisp.h` (guard `#endif` moved from
  mid-file to EOF — the tail content was unguarded and redefined on double
  inclusion), `LPBINDSTATUSCALLBACK` now the real interface pointer,
  `_ATL_OBJMAP_ENTRY`/`BEGIN_OBJECT_MAP`/`END_OBJECT_MAP`,
  `CComQIPtr::Detach`, `CWindow` members
  (`IsWindow`/`DestroyWindow`/`SetWindowPos`/`SetTimer`/`KillTimer`/
  `GetWindowLong`/`SetWindowLong`), `CWindowImpl::Create` at the documented
  7-arg ATL signature, `AtlAxWinInit` (fails: no AxWin class).
- **Wide CRT ownership**: the game calls the `wcs*` names (72 `wcslen`, 38
  `wcscpy`, 20 `wcsncpy`, 10 `wcscmp`, 6 `wcsicmp`, 5 `wcsstr`, 4 `wcscat`,
  3 `wcschr`, 1 each `wcsrchr`/`wcsncat`/`wcslwr`) and the system libc's
  implementations scan 32-bit units under `-fshort-wchar` (latent bugs even
  in previously "clean" TUs). The port now owns the names in `widestr.h`:
  `wcslen/wcscpy/wcsncpy/wcscat/wcsncat/wcscmp/wcsncmp/wcscspn` as
  `extern "C" inline` with a `__THROW`-conditional exception spec (glibc
  declares them noexcept, musl does not), and the MSVC-only
  `wcsicmp/wcslwr/wcsupr` as plain inline. New primitives
  `wide_strcpy/strncpy/strcat/strncat/strchr` implement the standard
  semantics. The scan trio (`wcschr`/`wcsrchr`/`wcsstr`) is NOT aliased:
  glibc's C++ `<wchar.h>` declares a const-preserving overload pair sharing
  one asm name, which cannot be redefined without a duplicate symbol; the
  game call sites use `pw153::platform::wide_strchr/wide_strrchr/wide_strstr`
  directly (4 non-AUI sites rewritten in this commit; 5 AUI sites are in the
  in-flight AUI agent's scope).
- INI reader: `GetPrivateProfileString(A/W/plain)` and
  `GetPrivateProfileInt(A/W/plain)` — the real parser (`inifile.{h,cpp}`):
  case-insensitive section/key, surrounding-space trim, optional quotes,
  `;` comments, the caller's default on miss, the key/section listing
  forms, wide keys/values over the UTF-16/UTF-8 converters.
- Angelica: `a_WideCharToMultiByte`/`a_MultiByteToWideChar` implemented in
  `AAssist.cpp` over the platform converters (declared in the
  `AngelicaCommon/Header/AAssist.h` the game includes; the Win32
  `WideCharToMultiByte` contract, negative length = null-terminated input).
- D3D8: `D3DADAPTER_IDENTIFIER8` (documented layout; `GUID` via `unknwn.h`),
  `IDirect3D8::GetAdapterIdentifier`, `D3DADAPTER_DEFAULT`,
  `D3DENUM_NO_WHQL_LEVEL`, `D3DXIMAGE_INFO` + `D3DXGetImageInfoFromFileA`
  (fails `D3DERR_INVALIDCALL`: the BBS download seam has no files yet; the
  caller's documented 3x3 placeholder fallback applies; the real header parse
  lands with the graphics cycle).

## Call-site adaptations (game files, semantics preserved)

- `EC_BBSPage.cpp`: 7 `AString`-through-varargs sites cast `(const char*)`;
  3 `WideCharToMultiByte(...,NULL,false)` last args `NULL` (MSVC accepted
  `false` as the null `BOOL*`).
- `TaskProcess.cpp`: two `localtime((long*)&ulCurTime)` sites widen the
  32-bit stamp to the 64-bit `time_t`; one `gmtime` site likewise; the
  2-arg MSVC `swprintf` gains the C99 size argument;
  `TaskInterface::m_tmFinishDlgShown` defined as `long` (the header's
  contract; MSVC `_USE_32BIT_TIME_T` had `time_t` == `long`).
- `ElementClient.cpp`: 22 `static_cast<LPVOID>(function-pointer)` ->
  `reinterpret_cast` (MSVC laxity; same value); one `AC2AS` varargs cast.
- `DlgFaceName.cpp`/`DlgExplorer.cpp`/`TaskTemplMan.cpp`: the scan-trio
  call sites use the port scanners.
- `findfiles_wasm.cpp` test made idempotent (cleans the previous run's
  `logs/` before `CreateDirectoryA`; the Win32 call fails on the existing
  directory); `RemoveDirectoryA` added to the seam (real `rmdir`).

## Defence/CCommon agent results landed (with the umbrella as contract owner)

- defence: `EnumWin32Pro.cpp`, `Process.cpp/.h`, `replaceapi.cpp`,
  `stackinfocollection.cpp` (unblocked by the seam fix), `tracer.cpp` — 5/5
  compile with real wasm objects. The agent's local Win32-gap block in
  `infocollection.h` was migrated into the umbrella `Windows.h`
  (OpenProcess, GetProcessHeap, HeapAlloc/HeapFree, FindWindowA,
  GetWindowThreadProcessId, GetCurrentProcessId, GetClassNameA, EnumWindows,
  SystemTimeToFileTime, VirtualQueryEx, MEMORY_BASIC_INFORMATION) and
  removed from `infocollection.h` — resolving the ODR collision that broke
  `ElementClient.cpp`.
- CCommon: `EL_Grassland.cpp`, `EL_GrassType.cpp`, `EC_Configs.cpp` (header
  fix), `A3DSkillGfxComposer2.cpp`, bonus `EL_Forest.cpp` — documented D3D8
  constants in `d3dx8.h`, clip-plane methods in `d3d8.h`,
  `A3DGfxExMan.h` case forwarder, `EL_Grassland.h` const-ref fix,
  `ELTerrainOutline2.h` forward declaration.

## Verification

- Per-TU objects (probe command, `-c -o ...`): `ElementClient.cpp`,
  `EC_BBSFileDownloader.cpp`, `EC_BBSPage.cpp`, `EC_BBSPageFile.cpp`,
  `DlgExplorer.cpp`, `TaskProcess.cpp`, `defence/{EnumWin32Pro,Process,
  replaceapi,stackinfocollection,tracer}.cpp` all exit 0 with real wasm
  objects.
- Test suites: native 30/30, wasm 26/26, sanitize 30/30
  (`ctest --test-dir build/client153-{native,wasm-tests,sanitize}`), all
  green after the seam changes.
- The wide-CRT probe TU (`wcsprobe.cpp`) compiles under both emscripten
  (musl declarations) and native GCC (glibc `__THROW` declarations) with the
  port owning the names.

## Limits / follow-ups

- `PWBrowser.cpp` (the embedded IE browser host — a dead path on the browser
  target) still fails on the deep `IWebBrowser2`/`CAxWindow`/`CComObject`
  surface; deferred until the BBS/shop path is wired or stubbed at a higher
  level.
- 5 AUI-side scan-trio call sites (`AUICommon.cpp` x4, `CSplit.cpp` x1) are
  in the in-flight AUI agent's scope; they must use the port scanners.
- `fwprintf`/`wprintf` still bind to the system CRT (the same 32-bit
  scanning concern); `wide_fprintf_utf16le` exists for them — call-site
  aliasing is a follow-up.
- `GetSystemMetrics` reports 0 until the canvas adapter registers (m-boot).
- Window creation/registration fail honestly; the browser host supplies the
  canvas window at m-boot.
