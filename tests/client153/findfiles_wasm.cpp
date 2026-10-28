#include "Windows.h"
#include "log_cleanup.h"
#include <cstdio>
#include <cstring>
#include <set>
#include <stdexcept>
#include <string>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(#condition); } while (false)
int main() {
  CHECK(CreateDirectoryA("logs", nullptr));
  FILE *log = std::fopen("logs/OLD.LOG", "wb");
  CHECK(log); CHECK(std::fputs("log", log) >= 0); CHECK(std::fclose(log) == 0);
  FILE *plain = std::fopen("logs/plain", "wb");
  CHECK(plain); CHECK(std::fclose(plain) == 0);
  WIN32_FIND_DATAA data{};
  HANDLE find = FindFirstFileA("logs\\*.*", &data);
  CHECK(find != INVALID_HANDLE_VALUE);
  std::set<std::string> names;
  do { names.insert(data.cFileName); } while (FindNextFileA(find, &data));
  CHECK(FindClose(find));
  CHECK(names.count("OLD.LOG") == 1 && names.count("plain") == 1);
  find = FindFirstFileA("logs\\*.log", &data);
  CHECK(find != INVALID_HANDLE_VALUE);
  CHECK(std::strcmp(data.cFileName, "OLD.LOG") == 0);
  CHECK(data.nFileSizeLow == 3);
  CHECK(!FindNextFileA(find, &data)); CHECK(FindClose(find));
  CHECK(pw153_is_transient_log("OLD.LOG"));
  CHECK(pw153_is_transient_log("dump.DmP"));
  CHECK(!pw153_is_transient_log("log.log.backup"));
  CHECK(DeleteFileA("logs\\OLD.LOG"));
  CHECK(!DeleteFileA("logs\\OLD.LOG"));
  CHECK(FindFirstFileA("logs\\*.log", &data) == INVALID_HANDLE_VALUE);
  return 0;
}
