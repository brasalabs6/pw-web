/* Adapted from original ALog.cpp, revision and hashes in source-manifest.json.
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 * Debug messages use the configured sink/console instead of a Windows window.
 */
#include "ALog.h"
#include "clock.h"
#include "format.h"
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
namespace {
std::atomic<LPFNDEFLOGOUTPUT> output{nullptr};
void directory(const char *path) {
  struct stat info{};
  if (mkdir(path, 0700) &&
      (errno != EEXIST || stat(path, &info) || !S_ISDIR(info.st_mode)))
    throw std::runtime_error("log directory unavailable");
}
std::string message(const char *format, va_list args) {
  char text[2048];
  const auto portable = pw153::platform::printf_format(format);
  if (std::vsnprintf(text, sizeof(text), portable.c_str(), args) < 0)
    throw std::runtime_error("log format failed");
  return text;
}
void deliver(int level, const char *format, va_list args, bool newline) {
  std::string text = level ? "<!> " : "<-> ";
  text += message(format, args);
  if (newline)
    text += '\n';
  if (auto sink = output.load())
    sink(text.c_str());
  else
    std::fputs(text.c_str(), stderr);
}
} // namespace
char ALog::m_szLogDir[260] = "Logs";
LPFNDEFLOGOUTPUT a_RedirectDefLogOutput(LPFNDEFLOGOUTPUT sink) {
  return output.exchange(sink);
}
void a_LogOutput(int level, const char *format, ...) {
  va_list args;
  va_start(args, format);
  try {
    deliver(level, format, args, true);
  } catch (...) {
    va_end(args);
    throw;
  }
  va_end(args);
}
void a_LogOutputNoReturn(int level, const char *format, ...) {
  va_list args;
  va_start(args, format);
  try {
    deliver(level, format, args, false);
  } catch (...) {
    va_end(args);
    throw;
  }
  va_end(args);
}
void a_SendLogToDebugWnd(int level, const char *format, ...) {
  va_list args;
  va_start(args, format);
  try {
    deliver(level, format, args, true);
  } catch (...) {
    va_end(args);
    throw;
  }
  va_end(args);
}
ALog::ALog() : m_pFile(nullptr) {}
ALog::~ALog() { Release(); }
bool ALog::Init(const char *file, const char *hello, bool append) {
  Release();
  if (!file || !hello)
    return false;
  try {
    directory(m_szLogDir);
  } catch (const std::exception &) {
    return false;
  }
  std::string relative(file);
  std::replace(relative.begin(), relative.end(), '\\', '/');
  const auto path = std::string(m_szLogDir) + "/" + relative;
  if (path.size() >= 260)
    return false;
  m_pFile = std::fopen(path.c_str(), append ? "a" : "w");
  if (!m_pFile)
    return false;
  try {
    const auto t = pw153::platform::local_calendar();
    if (std::fprintf(
            m_pFile,
            "%s\nCreated(or opened) on %02u/%02u/%04u %02u:%02u:%02u\n\n",
            hello, unsigned(t.day), unsigned(t.month), unsigned(t.year),
            unsigned(t.hour), unsigned(t.minute), unsigned(t.second)) < 0) {
      Release();
      return false;
    }
  } catch (const std::exception &) {
    Release();
    return false;
  }
  return true;
}
bool ALog::Release() {
  if (!m_pFile)
    return true;
  const bool wrote = FlushRepeated() && LogString("Log file closing.");
  auto *file = m_pFile;
  m_pFile = nullptr;
  const bool closed = std::fclose(file) == 0;
  return wrote && closed;
}
// Writes the pending repeat count for the previous message, if any.
bool ALog::FlushRepeated() {
  if (!m_pFile || !m_repeatCount)
    return true;
  try {
    const auto value = pw153::platform::local_calendar();
    const bool ok = std::fprintf(m_pFile,
                                 "[%02u:%02u:%02u.%03u] %s (repeated %u more times)\n",
                                 unsigned(value.hour), unsigned(value.minute),
                                 unsigned(value.second), unsigned(value.millisecond),
                                 m_lastRepeated.c_str(), m_repeatCount) >= 0 &&
                    std::fflush(m_pFile) == 0;
    m_repeatCount = 0;
    return ok;
  } catch (const std::exception &) {
    return false;
  }
}
bool ALog::Log(const char *format, ...) {
  va_list args;
  va_start(args, format);
  std::string text;
  try {
    text = message(format, args);
  } catch (...) {
    va_end(args);
    return false;
  }
  va_end(args);
  return LogString(text.c_str());
}
bool ALog::LogString(const char *text) {
  if (!m_pFile || !text)
    return false;
  //	Consecutive identical messages collapse into a counter: the first
  //	occurrence is written verbatim, repeats are counted, and the count
  //	flushes when a different message arrives (or the log closes). The
  //	engine logs every failed draw each frame; without this the log
  //	grows unbounded and the first real failure drowns.
  if (m_lastRepeated == text) {
    ++m_repeatCount;
    return true;
  }
  if (!FlushRepeated())
    return false;
  m_lastRepeated = text;
  try {
    const auto value = pw153::platform::local_calendar();
    return std::fprintf(m_pFile, "[%02u:%02u:%02u.%03u] %s\n",
                        unsigned(value.hour), unsigned(value.minute),
                        unsigned(value.second), unsigned(value.millisecond),
                        text) >= 0 &&
           std::fflush(m_pFile) == 0;
  } catch (const std::exception &) {
    return false;
  }
}
void ALog::SetLogDir(const char *path) {
  if (!path || !*path || std::strlen(path) >= sizeof(m_szLogDir))
    throw std::invalid_argument("invalid log directory");
  std::string name(path);
  for (auto &c : name)
    if (c == '\\')
      c = '/';
  while (name.size() > 1 && name.back() == '/')
    name.pop_back();
  directory(name.c_str());
  std::strcpy(m_szLogDir, name.c_str());
}
