# Original PCK/PKX package loader and file family

The client's own file access path is the original AngelicaFile library. This
lot compiles the real PCK/PKX loader (`AFilePackage`, `AFilePackGame`,
`AFilePackMan`), the memory-image file (`AFileImage`), the INI reader
(`AIniFile`), script files, the file manager globals (`AFI`/`AFPI`), the
`abase` containers (`vector`, `hashtab`, `ATempMemMan`) and the lock wrapper
(`ACSWrapper`) — the dependencies the track manager and the skeleton need,
and the first goal item "portar loaders reais PCK/PKX".

Source fixed at `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`. Copyrights and
per-file hashes remain in source-manifest.json. Frozen references, server and
Wine worktrees are not modified.

## Real-package proof

`AFilePackage::Open` loads the authorized v145 package
`element/facedata.pck` and reports **2005 entries**, identical to the count
the independently validated Python reader (`tools/assets/pck153.py`) reports
for the same file. 64 entries were opened through the original shared-file
path (`OpenSharedFile`/`CloseSharedFile`) with reported lengths equal to the
entry table. The zlib `Compress`/`Uncompress` pair round-trips a 10,000-byte
payload, and `AIniFile` round-trips sections and keys through `Save`.

The format constants (`AFPCK_GUARDBYTE0` `0xFDFDFEEE`, `AFPCK_GUARDBYTE1`
`0xF00DBEEF`, mask `0xA8937462`, check `0x59374231`) are the values proven
against real packages by the validated Python reader plus a direct byte read
of `facedata.pck` (2026-10-26). The stale `0xabcdefab`/`0xffeeffee` comments
in the original header predate the v145 format.

## Implementation and observed defects

- The original `AFilePackage.h` snapshot contains an uncompilable merge
  artifact: two same-named `FILEHEADER` structs and two same-named
  `pFileEntry` members. The v145 packages use the 32-bit layout (validated);
  the second variant is renamed to the `FILEHEADER64` the code references and
  the duplicate member is removed. The 64-bit members stay declared, unused,
  as in the original snapshot.
- Shared-file handles widen from `DWORD` to `uintptr_t` (the original stored
  list pointers in a 32-bit integer).
- `CRITICAL_SECTION` maps to a real `std::mutex` with the original
  init/enter/leave surface; `ACSWrapper` keeps RAII semantics.
- Disk path joins use the platform separator and POSIX absolute paths are
  recognized; package-internal names keep the format's backslashes.
- `GetTickCount` → `a_GetTime`, `_fileno`/`_chsize` → `fileno`/`ftruncate`,
  `stricmp`/`strlwr` → `strcasecmp`/portable lowercase, `LPBYTE`/`LPVOID` →
  `BYTE*`/`void*`, zlib `uLongf` lengths bridged through locals so the 32-bit
  PCK length stays authoritative.
- Original defects fixed: `abase::vector(int)` constructor implemented (the
  original left it commented out while `vector_ptr` calls it), `reserve()`
  called without its argument, unsigned `>= 0` asserts, `af_GetFileTitle`
  ignoring its buffer size, `AString` passed to varargs `%s` without the
  explicit conversion, dead `CheckFileEntryValid` and a dead entry read in
  the save loop.
- `AFileImage` write paths keep the original unsupported semantics; the
  custom memory allocator and the wide-char surface remain deferred.

## Evidence and reproduction

Native GCC 16.2.1, Emscripten 6.0.13/Node and ASan/UBSan CTest: **10/10 PASS**
(each configuration). The Release WASM resource runtime still builds.

```
cmake --build build/client153-native --parallel 2
ctest --test-dir build/client153-native --output-on-failure
cmake --build build/client153-wasm-tests --parallel 2
ctest --test-dir build/client153-wasm-tests --output-on-failure
cmake --build build/client153-sanitize --parallel 2
ctest --test-dir build/client153-sanitize --output-on-failure
./build/client153-native/client153_file_family_test \
  /home/nonexphere/workspace/pw/clients/client-1.5.3-v145-pwbr/app/element/facedata.pck
```

The package test requires the authorized client root; without it the suite
still covers zlib round-trip and INI round-trip and skips the package proof.

## Limits

Not yet proven: package creation/encryption paths, `AScriptFile` execution,
`AFilePackMan` multi-package resolution in the browser, CP936 name handling
inside the C++ loader (the Python reader covers it today), and the browser
VFS integration for package-backed reads. Skeleton, skinning, character
animation, client loop and gameplay remain pending.
