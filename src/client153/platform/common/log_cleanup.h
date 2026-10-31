#pragma once
#include <cctype>
#include <cstring>

// The original startup removes only log, dump and binary files from logs/.
inline bool pw153_is_transient_log(const char *name) {
  if (!name) return false;
  const auto length = std::strlen(name);
  if (length < 4 || name[length - 4] != '.') return false;
  char ext[4] = {};
  for (int i = 0; i < 3; ++i)
    ext[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(name[length - 3 + i])));
  return std::strcmp(ext, "log") == 0 || std::strcmp(ext, "dmp") == 0 ||
         std::strcmp(ext, "bin") == 0;
}
