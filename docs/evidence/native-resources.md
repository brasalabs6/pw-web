# Native client resources and realm evidence

State captured 2026-10-25 21:41 UTC. Client source: `references/client-153`
at `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`. Related Rust server reference:
`references/server-rust` at `423c0b7323e3effb3420edef4545b934afd56fda`.
This is an intake checklist and compatibility inventory, not evidence that a
client build, game asset, or realm has been validated.

## Resource closure evidenced by source

`Element/CCommon/elementpckdir.h` defines the startup package directories. The
client forms `<directory>.pck` and opens each during `_InitGameApp()` in
`Element/CElementClient/ElementClient.cpp`:

- `configs`, `facedata`, `gfx`, `grasses`, `help/script`, `interfaces`,
  `building`, `models`, `sfx`, `shaders`, `surfaces`, `textures`, `trees`,
  `litmodels`, and `loddata`.
- `Angelica2/AngelicaFile/Source/AFilePackBase.cpp` changes a package filename
  to `.pkx` while opening a split package; `.pkx` is therefore part of package
  intake when present.

The source also declares loose-resource trees: `data`, `maps`, `music`, `font`,
`cursors`, `userdata`, `help/voice`, `info`, `bbsfiles`, and
`physx/v2.8.4`. These are declarations, not proof every entry is required for
the first scene. The exact entry-map and first-scene transitive asset closure
remain to be measured from an authorized installed client.

### `.pck` extraction must match this client build

The pinned `Angelica2/AngelicaFile/Source/AFilePackGame.cpp` recognizes package
footer versions `0x00020001` and `0x00020002` through the 32-bit entry/header
branch, and `0x00020003` through a separate 64-bit header branch. Its
`FILEENTRY_INFILE` declaration is a 260-byte path plus four 32-bit fields. The
split-package threshold is `MAX_FILE_PACKAGE = 0x7fffff00` in
`Angelica2/AngelicaFile/Header/AFilePackBase.h`; the paired logical-stream read
is implemented in `AFilePackBase.cpp`.

The `pw-pck-extract` tool in the pinned Rust server reference documents
validation against 1.5.5 client binaries; its accepted footer set is
`0x00020002`/`0x00020003` and its README records build-specific structure
recalibration. It is **not yet validated for this 1.5.3 source/client**. Do not
use it as the asset parser or assume every installed package uses one layout.
After the user's download completes, first inventory package sizes and
companions, then validate version/header/entry layouts against the source and
the actual package bytes before extracting a `.mox` fixture.

Explicit startup data reads found so far:

| Path | Evidence |
| --- | --- |
| `data/elements.data` | `Element/CElementClient/EC_Game.cpp`: MD5 and `itemdataman::load_data`; failure stops initialization |
| `data/tasks.data` | `EC_Game.cpp` hashes it; `Task/EC_TaskInterface.cpp` loads task templates |
| `data/task_npc.data` | `Task/EC_TaskInterface.cpp` loads NPC task information |
| `userdata/dyn_tasks.data` | `Task/EC_TaskInterface.cpp` validates dynamic-task data |
| `configs/DynamicObjects.data` | `EC_Game.cpp` loads dynamic-object paths and fails initialization if absent |

No executable, package, mesh, texture, map, skeleton, animation, or loose
`.data` asset was present in the workspace or the checked local game/download
locations during this snapshot. The user has since reported that a complete
1.5.3 client download is in progress; this snapshot must be replaced with its
sanitized manifest and SHA-256 hashes after download completes. Do not copy
client files into Git. Record their path only in local logs, not this document.

## Realm candidates: compatibility is not established

- The pinned Rust server README lists its `realm_153` as abandoned because the
  available 1.5.3 client never logged in. It describes the 1.5.5 realm as
  playable in a basic feature set. This is direct evidence **against** assuming
  the Rust 1.5.3 realm works; it is not evidence that its 1.5.5 realm accepts
  this 1.5.3 source/client.
- [ThePW Casual](https://casual.thepw.net/) advertises a custom `1.5.3+` realm
  and requires registration to play. The `+` label does not identify an exact
  client build, protocol hash, or compatibility with this pinned source.
- The [PWI download landing page](https://www.playpwi.com/en/success) exposes a
  current client download flow, but the inspected page does not state an exact
  1.5.3 build or protocol. PWI compatibility is therefore unverified.

No realm is selected by this evidence. A target remains unverified until an
authorized client build and realm are identified by version/build metadata,
then login and world-entry packets are captured or reproduced against that
realm. Credentials and server addresses must remain in local secret storage.

## Reproduction after download

1. Keep the installed client outside the repository.
2. Record the source URL/issuer, download/update timestamp, launcher build and
   client version files.
3. Generate a sorted inventory of relative paths, byte lengths and SHA-256
   hashes without opening or publishing file contents.
4. Compare the package names and data paths above; report missing, extra, split
   (`.pkx`), and updated files separately. Do not treat a filename or directory
   name alone as proof of version.
5. Select a realm only after protocol validation. Keep the credentialed network
   test and any captures local and sanitized.
