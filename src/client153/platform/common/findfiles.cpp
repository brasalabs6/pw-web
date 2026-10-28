#include "Windows.h"
#include "io.h"
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

//	The MSVC CRT file-find surface over the same real scan.
namespace {
//	The FILETIME inverse of filetime() above (100ns since 1601 to the
// Unix epoch seconds).
std::time_t unix_time(FILETIME ft) {
  const std::uint64_t n =
      (static_cast<std::uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
  return static_cast<std::time_t>(n / 10000000ULL - 11644473600ULL);
}
void fill_finddata(const WIN32_FIND_DATAA &from, _finddata_t *to) {
  to->attrib = _A_NORMAL;
  if (from.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    to->attrib |= _A_SUBDIR;
  if (from.dwFileAttributes & 0x01)	//	FILE_ATTRIBUTE_READONLY.
    to->attrib |= _A_RDONLY;
  if (from.dwFileAttributes & 0x20)	//	FILE_ATTRIBUTE_ARCHIVE.
    to->attrib |= _A_ARCH;
  to->time_create = -1;	//	POSIX has no creation time (the CRT's unavailable marker).
  to->time_access = unix_time(from.ftLastAccessTime);
  to->time_write = unix_time(from.ftLastWriteTime);
  to->size = from.nFileSizeLow;
  std::strcpy(to->name, from.cFileName);
}
} // namespace

std::intptr_t _findfirst(const char *pattern, _finddata_t *data) {
  if (!pattern || !*pattern || !data)
    return -1;
  std::string path(pattern);
  std::replace(path.begin(), path.end(), '\\', '/');
  const auto pos = path.find_last_of('/');
  const auto directory =
      pos == std::string::npos ? "." : (pos == 0 ? "/" : path.substr(0, pos));
  auto filename = pos == std::string::npos ? path : path.substr(pos + 1);
  if (filename == "*.*")
    filename = "*";
  DIR *dir = opendir(directory.c_str());
  if (!dir)
    return -1;
  auto search = std::make_unique<Search>(Search{dir, directory, filename});
  WIN32_FIND_DATAA winData{};
  if (!next(*search, &winData)) {
    closedir(dir);
    return -1;
  }
  fill_finddata(winData, data);
  return reinterpret_cast<std::intptr_t>(search.release());
}

int _findnext(std::intptr_t handle, _finddata_t *data) {
  if (handle == -1 || !data)
    return -1;
  auto *search = reinterpret_cast<Search *>(handle);
  WIN32_FIND_DATAA winData{};
  if (!next(*search, &winData))
    return -1;
  fill_finddata(winData, data);
  return 0;
}

int _findclose(std::intptr_t handle) {
  if (handle == -1)
    return -1;
  auto *search = reinterpret_cast<Search *>(handle);
  const int result = closedir(search->dir);
  delete search;
  return result;
}
