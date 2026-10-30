// B02/B10: the game's real package flow in the WASM target — the pack man
// opens the SPLIT models package (models.pck + models.pkx) and resolves
// the game's exact GB18030 skin spelling. The native probe proved the
// LP64 path; this test proves the wasm32 path the browser runs, driving
// the shipped AFilePackMan/AFilePackGame code against the authorized
// asset (skipped when the asset is absent).
#include "AFilePackMan.h"
#include "AFI.h"
#include "AFileImage.h"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace {

int failures = 0;

void check(const char *label, bool ok) {
  if (!ok) {
    ++failures;
    std::printf("FAIL %s\n", label);
  } else {
    std::printf("ok %s\n", label);
  }
}

}  // namespace

int main(int argc, char **argv) {
  // argv[1] = the element dir (the authorized asset root); without it the
  // test reports the negative fixtures only (the CI ctest run).
  const char *elementDir = argc > 1 ? argv[1] : nullptr;
  if (elementDir) {
    struct stat st {};
    if (stat(elementDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
      std::printf("asset dir absent: %s — skipping the real-package checks\n",
                  elementDir);
      elementDir = nullptr;
    }
  }
  if (!elementDir) {
    std::printf("no asset dir given: nothing to verify\n");
    return failures ? 1 : 0;
  }

  af_Initialize();
  af_SetBaseDir(elementDir);
  AFilePackMan &man = g_AFilePackMan;
  check("OpenFilePackageInGame(models.pck) opens the split package",
        man.OpenFilePackageInGame("models.pck"));

  // The game's exact GB18030 spelling (EC_Player.cpp's default equipment
  // skin for the wuxia male, level-1 top):
  // Models\Players\装备\男\默认上衣\武侠男默认上衣一级.ski
  const char *asked =
      "Models\\Players\\\xd7\xb0\xb1\xb8\\\xc4\xd0\\\xc4\xac\xc8\xcf\xc9\xcf\xd2\xc2"
      "\\\xce\xe4\xcf\xc0\xc4\xd0\xc4\xac\xc8\xcf\xc9\xcf\xd2\xc2\xd2\xbb\xbc\xb6.ski";
  AFilePackBase *pkg = man.GetFilePck(asked);
  check("GetFilePck resolves the asked name to the models package",
        pkg != nullptr);
  if (pkg) {
    BYTE *data = nullptr;
    DWORD len = 0;
    const uintptr_t handle = pkg->OpenSharedFile(asked, &data, &len, true);
    check("OpenSharedFile returns the skin payload", handle != 0 && data != nullptr);
    if (handle && data) {
      // The .ski files start with the MOXBIKSA ('A' variant) magic.
      check("skin payload carries the MOXBIKSA magic",
            len > 8 && std::memcmp(data, "MOXBIKSA", 8) == 0);
      check("skin payload length matches the pinned entry",
            len == 28242);
      pkg->CloseSharedFile(handle);
    }
  }

  // The game's actual entry for skin loads: A3DSkin::Load opens the file
  // through AFileImage (the package-first lookup with the relative-path
  // computation) — drive the same entry with the same spelling.
  {
    AFileImage image;
    const bool opened = image.Open(asked, AFILE_OPENEXIST | AFILE_TEMPMEMORY);
    check("AFileImage::Open resolves the skin through the game's entry", opened);
    if (opened) {
      check("AFileImage payload length matches", image.GetFileLength() == 28242);
      image.Close();
    }
  }

  // The MIXED-encoding path the game's default-equipment chain builds:
  // the GB18030 template literal from the GB18030-encoded EC_Player.cpp
  // plus the UTF-8 default-name literal from the UTF-8-encoded
  // EC_Player.h ("默认上衣"), inside one segment. The run-level
  // candidate must resolve it to the package's pure-GB18030 name.
  {
    // Models\Players\装备\男\默认上衣\武侠男默认上衣一级.ski with the
    // 默认上衣 runs in UTF-8 and the rest in GB18030.
    const char *mixed =
        "Models\\Players\\\xd7\xb0\xb1\xb8\\\xc4\xd0\\"
        "\xe9\xbb\x98\xe8\xae\xa4\xe4\xb8\x8a\xe8\xa1\xa3\\"
        "\xce\xe4\xcf\xc0\xc4\xd0"
        "\xe9\xbb\x98\xe8\xae\xa4\xe4\xb8\x8a\xe8\xa1\xa3\xd2\xbb\xbc\xb6.ski";
    AFileImage image;
    const bool opened = image.Open(mixed, AFILE_OPENEXIST | AFILE_TEMPMEMORY);
    check("AFileImage::Open resolves the mixed-encoding skin path", opened);
    if (opened) {
      check("mixed-path payload length matches", image.GetFileLength() == 28242);
      image.Close();
    }
  }

  if (failures) {
    std::printf("%d failures\n", failures);
    return 1;
  }
  std::printf("all pack lookup checks passed\n");
  return 0;
}
