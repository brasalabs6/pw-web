/* Portable adaptation of Angelica File AFile.cpp, pinned in
 * source-manifest.json. Original Copyright (c) 2001 Archosaur Studio, All
 * Rights Reserved. This class owns synchronous local/VFS file operations.
 * Browser network fetch must finish before opening a resource; no synchronous
 * HTTP is performed here.
 */
#include "AFile.h"
#include <algorithm>
#include <array>
#include <climits>
#include <cstring>
#include <limits>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
namespace {
std::array<unsigned char, 4> little(DWORD value) {
  return {static_cast<unsigned char>(value),
          static_cast<unsigned char>(value >> 8),
          static_cast<unsigned char>(value >> 16),
          static_cast<unsigned char>(value >> 24)};
}
DWORD decode(const unsigned char *b) {
  return DWORD(b[0]) | (DWORD(b[1]) << 8) | (DWORD(b[2]) << 16) |
         (DWORD(b[3]) << 24);
}
} // namespace
AFile::AFile()
    : m_pFile(nullptr), m_dwFlags(0), m_dwTimeStamp(0), m_bHasOpened(false) {
  m_szFileName[0] = m_szRelativeName[0] = 0;
}
AFile::~AFile() { Close(); }
bool AFile::Open(const char *folder, const char *file, DWORD flags) {
  if (!folder || !file)
    return false;
  std::string path(folder);
  if (!path.empty() && path.back() != '/' && path.back() != '\\')
    path += '/';
  path += file;
  return Open(path.c_str(), flags);
}
bool AFile::Open(const char *file, DWORD flags) {
  Close();
  m_szFileName[0] = m_szRelativeName[0] = 0;
  m_dwFlags = 0;
  m_dwTimeStamp = 0;
  if (!file || !*file)
    return false;
  std::string path(file);
  std::replace(path.begin(), path.end(), '\\', '/');
  if (path.size() >= sizeof(m_szFileName))
    return false;
  const DWORD mode =
      flags & (AFILE_OPENEXIST | AFILE_CREATENEW | AFILE_OPENAPPEND);
  if (mode != AFILE_OPENEXIST && mode != AFILE_CREATENEW &&
      mode != AFILE_OPENAPPEND)
    return false;
  if ((flags & (AFILE_TEXT | AFILE_BINARY)) == (AFILE_TEXT | AFILE_BINARY))
    return false;
  const char *access = mode == AFILE_OPENEXIST   ? "rb"
                       : mode == AFILE_CREATENEW ? "wb"
                                                 : "ab+";
  m_pFile = std::fopen(path.c_str(), access);
  if (!m_pFile)
    return false;
  std::strcpy(m_szFileName, path.c_str());
  std::string relative(path);
  if (path.front() == '/') {
    char cwd[4096];
    if (!getcwd(cwd, sizeof(cwd))) {
      Close();
      return false;
    }
    std::string prefix(cwd);
    if (prefix.back() != '/')
      prefix += '/';
    if (path.compare(0, prefix.size(), prefix) == 0)
      relative = path.substr(prefix.size());
  }
  std::strcpy(m_szRelativeName, relative.c_str());
  m_dwFlags = flags;
  if (!(flags & AFILE_NOHEAD)) {
    if (mode == AFILE_CREATENEW) {
      auto head =
          little(flags & AFILE_TEXT ? AFILE_TYPE_TEXT : AFILE_TYPE_BINARY);
      if (std::fwrite(head.data(), 1, 4, m_pFile) != 4) {
        Close();
        return false;
      }
    } else if (mode == AFILE_OPENEXIST) {
      unsigned char head[4];
      const auto n = std::fread(head, 1, 4, m_pFile);
      const auto magic = n == 4 ? decode(head) : 0;
      m_dwFlags &= ~(AFILE_BINARY | AFILE_TEXT);
      if (magic == AFILE_TYPE_BINARY)
        m_dwFlags |= AFILE_BINARY;
      else if (magic == AFILE_TYPE_TEXT)
        m_dwFlags |= AFILE_TEXT;
      else {
        m_dwFlags |= AFILE_TEXT;
        if (std::fseek(m_pFile, 0, SEEK_SET)) {
          Close();
          return false;
        }
      }
    }
  }
  struct stat info{};
  if (fstat(fileno(m_pFile), &info)) {
    Close();
    return false;
  }
  m_dwTimeStamp = static_cast<DWORD>(info.st_mtime);
  m_bHasOpened = true;
  return true;
}
bool AFile::Close() {
  bool ok = true;
  if (m_pFile) {
    ok = std::fclose(m_pFile) == 0;
    m_pFile = nullptr;
  }
  m_bHasOpened = false;
  return ok;
}
bool AFile::Read(void *buffer, DWORD length, DWORD *count) {
  if (count)
    *count = 0;
  if (!m_pFile || !count || (!buffer && length))
    return false;
  *count = static_cast<DWORD>(std::fread(buffer, 1, length, m_pFile));
  return !std::ferror(m_pFile);
}
bool AFile::Write(void *buffer, DWORD length, DWORD *count) {
  if (count)
    *count = 0;
  if (!m_pFile || !count || (!buffer && length))
    return false;
  *count = static_cast<DWORD>(std::fwrite(buffer, 1, length, m_pFile));
  return *count == length;
}
bool AFile::ReadLine(char *buffer, DWORD length, DWORD *count) {
  if (count)
    *count = 0;
  if (!m_pFile || !buffer || !count || length < 2 || length > INT_MAX)
    return false;
  if (!std::fgets(buffer, static_cast<int>(length), m_pFile))
    return false;
  auto size = std::strlen(buffer);
  while (size && (buffer[size - 1] == '\r' || buffer[size - 1] == '\n'))
    buffer[--size] = 0;
  *count = static_cast<DWORD>(size + 1);
  return true;
}
bool AFile::ReadString(char *buffer, DWORD length, DWORD *count) {
  if (count)
    *count = 0;
  if (!m_pFile || !buffer || !length || !count)
    return false;
  buffer[0] = 0;
  for (DWORD i = 0; i < length; i++) {
    int c = std::fgetc(m_pFile);
    if (c == EOF) {
      buffer[0] = 0;
      return false;
    }
    if (!c) {
      buffer[i] = 0;
      *count = i + 1;
      return true;
    }
    if (i + 1 == length) {
      buffer[0] = 0;
      return false;
    }
    buffer[i] = static_cast<char>(c);
  }
  return false;
}
bool AFile::WriteString(const AString &text) {
  if (!m_pFile)
    return false;
  auto head = little(static_cast<DWORD>(text.GetLength()));
  return std::fwrite(head.data(), 1, 4, m_pFile) == 4 &&
         std::fwrite(static_cast<const char *>(text), 1, text.GetLength(),
                     m_pFile) == static_cast<std::size_t>(text.GetLength());
}
bool AFile::ReadString(AString &text) {
  if (!m_pFile)
    return false;
  unsigned char head[4];
  if (std::fread(head, 1, 4, m_pFile) != 4)
    return false;
  auto length = decode(head);
  if (length > 16 * 1024 * 1024)
    return false;
  const auto position = std::ftell(m_pFile);
  if (position < 0)
    return false;
  const auto total = GetFileLength();
  if (static_cast<unsigned long>(position) > total ||
      length > total - static_cast<DWORD>(position))
    return false;
  std::vector<char> bytes(static_cast<std::size_t>(length) + 1);
  if (std::fread(bytes.data(), 1, length, m_pFile) != length)
    return false;
  bytes[length] = 0;
  text = AString(bytes.data(), static_cast<int>(length));
  return true;
}
bool AFile::WriteLine(const char *line) {
  return m_pFile && line && std::fprintf(m_pFile, "%s\n", line) >= 0;
}
DWORD AFile::GetPos() {
  if (!m_pFile)
    return std::numeric_limits<DWORD>::max();
  auto p = std::ftell(m_pFile);
  return p < 0 || static_cast<unsigned long>(p) >
                      std::numeric_limits<DWORD>::max()
             ? std::numeric_limits<DWORD>::max()
             : static_cast<DWORD>(p);
}
bool AFile::Seek(int offset, AFILE_SEEK origin) {
  return m_pFile &&
         (origin == AFILE_SEEK_SET || origin == AFILE_SEEK_CUR ||
          origin == AFILE_SEEK_END) &&
         std::fseek(m_pFile, offset, origin) == 0;
}
bool AFile::ResetPointer() { return Seek(0, AFILE_SEEK_SET); }
DWORD AFile::GetFileLength() {
  if (!m_pFile)
    return 0;
  auto p = std::ftell(m_pFile);
  if (p < 0 || std::fseek(m_pFile, 0, SEEK_END))
    return 0;
  auto end = std::ftell(m_pFile);
  if (std::fseek(m_pFile, p, SEEK_SET) || end < 0 ||
      static_cast<unsigned long>(end) > std::numeric_limits<DWORD>::max())
    return 0;
  return static_cast<DWORD>(end);
}
bool AFile::Flush() { return m_pFile && std::fflush(m_pFile) == 0; }
