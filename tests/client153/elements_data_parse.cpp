// PW23-040 / U05: the elements.data binary parse contract, driven through
// the REAL shipped parser TU (elementdataman::load_data — the exact
// function whose failure EC_Game.cpp bypasses; the itemdataman wrapper
// only adds downstream sell-item generation whose helpers live in the
// full game closure).
//
// The format contract (pinned exporter, built with _USE_32BIT_TIME_T):
//   [0..3]  version == ELEMENTDATA_VERSION (0x30000091)
//   [4..7]  32-bit export timestamp
//   [8..11] first section count (equipment_addon: 2936 in the authorized
//           file), then per section: count (4 bytes) + count * sizeof(T)
//           record bytes, ending exactly at EOF — any width or layout
//           divergence surfaces as an explicit load failure.
//
// With argv[1] = the authorized elements.data path (a local authorized
// asset, never committed), the full byte-exact load runs: the header is
// verified independently, the complete section chain must parse, and the
// public query API must report the pinned counts. Without it (CI), the
// crafted-fixture negative tests run: wrong version, truncated stream
// and absurd count each fail explicitly.
#include "elementdataman.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

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

bool write_fixture(const char *path, const unsigned char *bytes, size_t length) {
  FILE *file = std::fopen(path, "wb");
  if (!file)
    return false;
  const size_t written = std::fwrite(bytes, 1, length, file);
  std::fclose(file);
  return written == length;
}

void put32(unsigned char *out, unsigned int value) {
  for (int i = 0; i < 4; ++i)
    out[i] = static_cast<unsigned char>((value >> (i * 8)) & 0xff);
}

// The negative fixtures follow the format contract exactly except for the
// one defect under test.
void run_negative_fixtures() {
  const unsigned int version = ELEMENTDATA_VERSION;

  { // Wrong version: rejected before any section read.
    unsigned char bytes[12];
    put32(bytes, version + 1);
    put32(bytes + 4, 0x577a12d2u);
    put32(bytes + 8, 2936);
    check("fixture write (wrong version)", write_fixture("u05_wrong_version.data", bytes, sizeof(bytes)));
    elementdataman data;
    check("wrong version rejected", data.load_data("u05_wrong_version.data") != 0);
    std::remove("u05_wrong_version.data");
  }
  { // Truncated: version and timestamp parse, the first section count is
    // cut off — the load must fail, not hang or wrap.
    unsigned char bytes[10];
    put32(bytes, version);
    put32(bytes + 4, 0x577a12d2u);
    bytes[8] = 0x78;
    bytes[9] = 0x0b;
    check("fixture write (truncated)", write_fixture("u05_truncated.data", bytes, sizeof(bytes)));
    elementdataman data;
    check("truncated stream rejected", data.load_data("u05_truncated.data") != 0);
    std::remove("u05_truncated.data");
  }
  { // Absurd count: the header is valid but the first section claims
    // 0xFFFFFFFF records — impossible against the remaining bytes, so
    // the load must fail explicitly instead of attempting the reserve.
    unsigned char bytes[12];
    put32(bytes, version);
    put32(bytes + 4, 0x577a12d2u);
    put32(bytes + 8, 0xffffffffu);
    check("fixture write (absurd count)", write_fixture("u05_absurd_count.data", bytes, sizeof(bytes)));
    elementdataman data;
    check("absurd count rejected", data.load_data("u05_absurd_count.data") != 0);
    std::remove("u05_absurd_count.data");
  }
  { // Missing file: fopen failure is an explicit error.
    elementdataman data;
    check("missing file rejected", data.load_data("u05_does_not_exist.data") != 0);
  }
}

void run_real_file(const char *path) {
  // Independent header verification: the fixture must be the authorized
  // v145 export before the loader's contract is asserted against it.
  FILE *file = std::fopen(path, "rb");
  check("real file opens", file != nullptr);
  if (!file)
    return;
  unsigned char header[12];
  check("real header reads", std::fread(header, 1, sizeof(header), file) == sizeof(header));
  unsigned int version = 0, timestamp = 0, first_count = 0;
  std::memcpy(&version, header, 4);
  std::memcpy(&timestamp, header + 4, 4);
  std::memcpy(&first_count, header + 8, 4);
  std::fclose(file);
  std::printf("header: version=0x%08x timestamp=0x%08x first_count=%u\n",
              version, timestamp, first_count);
  check("real version matches", version == ELEMENTDATA_VERSION);
  check("real first count is the pinned 2936", first_count == 2936);
  // 0x577a12d2 == 2016-06-28: the v145 exporter's 32-bit timestamp.
  check("real timestamp is the 32-bit 2016 export", timestamp == 0x577a12d2u);

  // The real parser TU the game's load path runs (EC_Game.cpp ->
  // itemdataman::load_data -> elementdataman::load_data).
  elementdataman data;
  const int rc = data.load_data(path);
  std::printf("elementdataman::load_data rc=%d\n", rc);
  check("real elements.data loads", rc == 0);
  if (rc != 0)
    return;

  const unsigned int addon = data.get_data_num(ID_SPACE_ADDON);
  const unsigned int essence = data.get_data_num(ID_SPACE_ESSENCE);
  const unsigned int config = data.get_data_num(ID_SPACE_CONFIG);
  std::printf("id spaces: addon=%u essence=%u config=%u\n", addon, essence, config);
  check("addon id space populated", addon > 0);
  check("essence id space populated", essence > 0);
  check("config id space populated", config > 0);
  // The pinned counts of the authorized file (observed deterministic
  // values, the facedata.pck 2005-entries pattern). addon = equipment
  // (2936) + monster addons; essence/config aggregate their sections.
  check("addon count pinned", addon == 2945);
  check("essence count pinned", essence == 47444);
  check("config count pinned", config == 2086);

  // The game's exact call shape (EC_Game.cpp): the Windows spelling
  // "data\\elements.data" relative to the working directory. The raw
  // fopen inside load_data must normalize the separator — the browser
  // VFS treats a backslash as a plain character and this is the call
  // the restored boot abort caught failing there.
  char previous_dir[4096];
  if (getcwd(previous_dir, sizeof(previous_dir)))
  {
    std::string asset_dir(path);
    // .../app/element/data/elements.data -> .../app/element (the game's
    // working directory, with data/ as its subdirectory).
    for (int strip = 0; strip < 2; ++strip)
    {
      const std::size_t slash = asset_dir.find_last_of('/');
      if (slash == std::string::npos)
        break;
      asset_dir.resize(slash);
    }
    if (chdir(asset_dir.c_str()) == 0)
    {
      elementdataman windows_spelling;
      const int windows_rc = windows_spelling.load_data("data\\elements.data");
      std::printf("windows-spelling load rc=%d\n", windows_rc);
      check("windows backslash spelling loads (the game's call shape)",
            windows_rc == 0);
      check("windows spelling count matches",
            windows_spelling.get_data_num(ID_SPACE_ADDON) == 2945);
      chdir(previous_dir);
    }
    else
    {
      check("chdir to the asset directory", false);
    }
  }
}

}  // namespace

int main(int argc, char **argv) {
  // The serialized width is 4 bytes everywhere in this format; the
  // loader must not depend on the host time_t/size_t widths.
  static_assert(sizeof(unsigned int) == 4, "the format's width is fixed");
  run_negative_fixtures();
  if (argc > 1)
    run_real_file(argv[1]);
  else
    std::printf("no asset path given: negative fixtures only "
                "(pass the authorized elements.data path for the full load)\n");
  if (failures) {
    std::printf("%d failures\n", failures);
    return 1;
  }
  std::printf("all elements.data parse checks passed\n");
  return 0;
}
