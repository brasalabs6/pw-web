// B20: the case-folded VFS lookup must preserve the caller's lookup
// semantics. The test installs the real hook (pw153_vfs_install_casefold)
// and drives the real Emscripten MEMFS through the C file API:
//   1. an existing asset opened with mismatched case reads its content;
//   2. a directory created under a mismatched-case existing directory
//      lands in the folded parent (the parent lookup with a missing leaf);
//   3. a missing leaf fails cleanly (noent_okay must not invent a file);
//   4. lstat on a mismatched-case symlink path keeps no-follow semantics;
//   5. ".." and mixed-case nested paths resolve.
#include "vfs_casefold.h"

#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

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

bool write_file(const char *path, const char *content) {
  FILE *file = std::fopen(path, "wb");
  if (!file)
    return false;
  const bool ok = std::fwrite(content, 1, std::strlen(content), file) ==
                  std::strlen(content);
  std::fclose(file);
  return ok;
}

bool read_file(const char *path, char *out, size_t size) {
  FILE *file = std::fopen(path, "rb");
  if (!file)
    return false;
  const size_t read = std::fread(out, 1, size - 1, file);
  out[read] = '\0';
  std::fclose(file);
  return true;
}

}  // namespace

int main() {
  pw153_vfs_install_casefold();

  // The fixture tree in the original's on-disk casing.
  check("mkdir maps", mkdir("maps", 0777) == 0);
  check("mkdir maps/login", mkdir("maps/login", 0777) == 0);
  check("mkdir maps/login/bsdata", mkdir("maps/login/bsdata", 0777) == 0);
  check("fixture write", write_file("maps/login/bsdata/1.dat", "PAYLOAD"));

  // 1. The existing asset with mismatched case.
  char buffer[64] = {};
  check("mismatched-case open reads",
        read_file("MAPS/LOGIN/BSDATA/1.DAT", buffer, sizeof(buffer)) &&
            std::strcmp(buffer, "PAYLOAD") == 0);

  // 2. Create under a mismatched-case existing directory: the parent
  // lookup ("/MAPS/LOGIN/newdir") has a missing leaf but the folded
  // parent exists.
  check("mkdir under folded parent",
        mkdir("MAPS/LOGIN/newdir", 0777) == 0);
  struct stat st {};
  check("newdir landed in the folded parent",
        stat("maps/login/newdir", &st) == 0 && S_ISDIR(st.st_mode));

  // 3. A missing leaf fails cleanly.
  FILE *missing = std::fopen("maps/login/bsdata/absent.dat", "rb");
  check("missing leaf fopen fails", missing == nullptr);
  check("missing leaf mismatched case fails",
        std::fopen("MAPS/LOGIN/BSDATA/ABSENT.DAT", "rb") == nullptr);

  // 4. lstat keeps no-follow semantics on a folded path.
  check("symlink create",
        symlink("bsdata/1.dat", "maps/login/link.dat") == 0);
  struct stat lst {};
  check("lstat on folded symlink path",
        lstat("MAPS/LOGIN/link.dat", &lst) == 0 && S_ISLNK(lst.st_mode));
  struct stat stt {};
  check("stat follows through the folded path",
        stat("MAPS/LOGIN/link.dat", &stt) == 0 && S_ISREG(stt.st_mode));

  // 5. ".." and mixed-case nested paths.
  check("dotdot mixed-case open",
        read_file("MAPS/LOGIN/bsdata/../BSDATA/1.DAT", buffer, sizeof(buffer)) &&
            std::strcmp(buffer, "PAYLOAD") == 0);

  if (failures) {
    std::printf("%d failures\n", failures);
    return 1;
  }
  std::printf("all casefold FS checks passed\n");
  return 0;
}
