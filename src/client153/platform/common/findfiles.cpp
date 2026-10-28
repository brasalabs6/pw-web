#include "Windows.h"
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <memory>
#include <string>

namespace {
struct Search { DIR *dir; std::string directory, pattern; };
bool matches(const char *p, const char *s) {
  const char *star = nullptr, *retry = nullptr;
  while (*s) {
    if (*p == '?' || (*p && std::tolower(static_cast<unsigned char>(*p)) == std::tolower(static_cast<unsigned char>(*s)))) { ++p; ++s; }
    else if (*p == '*') { star = p++; retry = s; }
    else if (star) { p = star + 1; s = ++retry; }
    else return false;
  }
  while (*p == '*') ++p;
  return !*p;
}
FILETIME filetime(std::time_t t) {
  const auto n = (static_cast<std::uint64_t>(t) + 11644473600ULL) * 10000000ULL;
  return {static_cast<DWORD>(n), static_cast<DWORD>(n >> 32)};
}
bool next(Search &search, WIN32_FIND_DATAA *data) {
  while (auto *entry = readdir(search.dir)) {
    if (!matches(search.pattern.c_str(), entry->d_name)) continue;
    const std::string path = search.directory + "/" + entry->d_name;
    struct stat st{};
    if (stat(path.c_str(), &st) != 0 || std::strlen(entry->d_name) >= MAX_PATH) continue;
    *data = {};
    data->dwFileAttributes = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : 0x80;
    const auto size = static_cast<std::uint64_t>(st.st_size);
    data->nFileSizeHigh = static_cast<DWORD>(size >> 32);
    data->nFileSizeLow = static_cast<DWORD>(size);
    // POSIX ctime is metadata-change time, not file creation time.
    data->ftCreationTime = {};
    data->ftLastAccessTime = filetime(st.st_atime);
    data->ftLastWriteTime = filetime(st.st_mtime);
    std::strcpy(data->cFileName, entry->d_name);
    return true;
  }
  return false;
}
}
HANDLE FindFirstFileA(const char *pattern, WIN32_FIND_DATAA *data) {
  if (!pattern || !*pattern || !data) return INVALID_HANDLE_VALUE;
  std::string path(pattern);
  std::replace(path.begin(), path.end(), '\\', '/');
  const auto pos = path.find_last_of('/');
  const auto directory = pos == std::string::npos ? "." : (pos == 0 ? "/" : path.substr(0, pos));
  auto filename = pos == std::string::npos ? path : path.substr(pos + 1);
  if (filename == "*.*") filename = "*";
  DIR *dir = opendir(directory.c_str());
  if (!dir) return INVALID_HANDLE_VALUE;
  auto search = std::make_unique<Search>(Search{dir, directory, filename});
  if (!next(*search, data)) { closedir(dir); return INVALID_HANDLE_VALUE; }
  return search.release();
}
BOOL FindNextFileA(HANDLE handle, WIN32_FIND_DATAA *data) {
  if (!handle || handle == INVALID_HANDLE_VALUE || !data) return FALSE;
  return next(*static_cast<Search *>(handle), data) ? TRUE : FALSE;
}
BOOL FindClose(HANDLE handle) {
  if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
  auto *search = static_cast<Search *>(handle);
  const int result = closedir(search->dir);
  delete search;
  return result == 0 ? TRUE : FALSE;
}
