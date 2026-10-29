// Structured asset-failure taxonomy implementation (see asset_diag.h).
#include "asset_diag.h"

#include <cstdio>
#include <map>
#include <string>
#include <utility>

namespace pw153::platform {

std::map<std::pair<int, std::string>, int> &AssetFailures() {
  static std::map<std::pair<int, std::string>, int> map;
  return map;
}

void RecordAssetFailure(AssetFailReason reason, const char *path) {
  if (!path || !*path)
    return;
  ++AssetFailures()[{static_cast<int>(reason), std::string(path)}];
  std::printf("PWASSET record reason=%d map=%zu path=%s\n", static_cast<int>(reason),
              AssetFailures().size(), path);
}

void DumpAssetFailures() {
  static const char *const names[] = {
      "NO_PACKAGE_COVERAGE", "PCK_ENTRY_NOT_FOUND", "DISK_OPEN_FAILED", "DISK_READ_FAILED"};
  auto &map = AssetFailures();
  int total = 0;
  for (const auto &entry : map)
    total += entry.second;
  std::printf("PWASSET total failure reports: %d, unique (reason, path): %zu\n", total, map.size());
  // The full classified report goes to a MEMFS file: byte-exact (the
  // console UTF-8 bridge is lossy for GB18030 paths) and durable as an
  // artifact for the failure taxonomy.
  if (FILE *file = std::fopen("/Logs/pwasset.txt", "wb")) {
    std::fprintf(file, "total failure reports: %d, unique (reason, path): %zu\n", total, map.size());
    for (const auto &entry : map)
      std::fprintf(file, "%s x%d %s\n", names[entry.first.first], entry.second, entry.first.second.c_str());
    std::fclose(file);
    std::printf("PWASSET full report written to /Logs/pwasset.txt\n");
  } else {
    std::printf("PWASSET failed to write /Logs/pwasset.txt\n");
  }
}

}  // namespace pw153::platform

extern "C" void pw153_asset_diag_dump() { pw153::platform::DumpAssetFailures(); }
