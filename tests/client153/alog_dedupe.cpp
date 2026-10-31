// B19: the engine log collapses consecutive identical messages. The
// engine logs every failed draw each frame; without the counter the log
// grows unbounded and the first real failure drowns. The test drives the
// real ALog over a temp file: the first occurrence is written verbatim,
// consecutive repeats collapse into a counted line on the next different
// message, and Release flushes a trailing count.
#include "ALog.h"

#include <cstdio>
#include <cstring>
#include <string>

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

std::string read_file(const char *path) {
  FILE *file = std::fopen(path, "rb");
  if (!file)
    return "";
  std::string content;
  char buffer[4096];
  size_t read;
  while ((read = std::fread(buffer, 1, sizeof(buffer), file)) > 0)
    content.append(buffer, read);
  std::fclose(file);
  return content;
}

}  // namespace

int main() {
  // The test's working directory has no Logs/ tree; point the static log
  // directory at it so the file lands next to the test.
  ALog::SetLogDir(".");
  const char *path = "u19_alog_dedupe.log";
  std::remove(path);
  {
    ALog log;
    check("log init", log.Init(path, "B19 dedupe test"));
    log.Log("A3DDevice::DrawIndexedPrimitive() Fail");
    log.Log("A3DDevice::DrawIndexedPrimitive() Fail");
    log.Log("A3DDevice::DrawIndexedPrimitive() Fail");
    log.Log("A3DDevice::UnlockVertexBuffer() Fail");
    log.Log("A3DDevice::UnlockVertexBuffer() Fail");
    log.Log("A3DDevice::DrawIndexedPrimitive() Fail");
    log.Release();
  }
  const std::string content = read_file(path);
  check("first failure verbatim",
        content.find("A3DDevice::DrawIndexedPrimitive() Fail\n") != std::string::npos);
  // Two runs of 3 consecutive identical draws: the first writes once, the
  // second run's repeats collapse; the counted lines carry the counts.
  const std::string counted = "(repeated 2 more times)";
  check("consecutive repeats counted", content.find(counted) != std::string::npos);
  const std::string counted2 = "(repeated 1 more times)";
  check("second run counted separately", content.find(counted2) != std::string::npos);
  // The raw text appears exactly twice (the two first-occurrences), not
  // five times — the file stays bounded.
  size_t pos = 0;
  int occurrences = 0;
  while ((pos = content.find("A3DDevice::DrawIndexedPrimitive() Fail\n", pos)) !=
         std::string::npos) {
    ++occurrences;
    pos += 1;
  }
  check("repeats collapsed (2 verbatim occurrences)", occurrences == 2);
  check("close line present", content.find("Log file closing.") != std::string::npos);
  std::remove(path);

  if (failures) {
    std::printf("%d failures\n", failures);
    return 1;
  }
  std::printf("all ALog dedupe checks passed\n");
  return 0;
}
