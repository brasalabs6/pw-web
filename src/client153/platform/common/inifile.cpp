/*
 * FILE: inifile.cpp (platform seam)
 *
 * DESCRIPTION: The real INI parse behind the Win32 private-profile
 * surface declared in inifile.h. One pass over the file: sections are
 * bracketed names, entries are key=value with surrounding-space
 * trimming and optional surrounding quotes, `;` starts a comment, and
 * the section/key match is case-insensitive (the documented Win32
 * contract).
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#include "inifile.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::string Trim(const std::string &text) {
  std::size_t begin = 0;
  std::size_t end = text.size();
  while (begin < end && (text[begin] == ' ' || text[begin] == '\t'))
    ++begin;
  while (end > begin && (text[end - 1] == ' ' || text[end - 1] == '\t'))
    --end;
  return text.substr(begin, end - begin);
}

//	Case-insensitive equality over bytes (the INI contract).
bool IEquals(const std::string &a, const std::string &b) {
  if (a.size() != b.size())
    return false;
  for (std::size_t i = 0; i < a.size(); ++i) {
    char x = a[i];
    char y = b[i];
    if (x >= 'A' && x <= 'Z')
      x = static_cast<char>(x - 'A' + 'a');
    if (y >= 'A' && y <= 'Z')
      y = static_cast<char>(y - 'A' + 'a');
    if (x != y)
      return false;
  }
  return true;
}

struct IniResult {
  bool found = false;
  std::string value;
  std::vector<std::string> names;	//	the listing forms' output.
};

//	The single parse pass. section == NULL lists section names;
//	section != NULL with key == NULL lists the section's key names.
IniResult ParseIni(const char *file, const char *section, const char *key) {
  IniResult result;
  std::FILE *in = file ? std::fopen(file, "rb") : nullptr;
  if (!in)
    return result;
  const bool listSections = section == nullptr;
  const bool listKeys = section != nullptr && key == nullptr;
  std::string currentSection;
  char raw[2048];
  while (std::fgets(raw, sizeof(raw), in)) {
    std::string line(raw);
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
      line.pop_back();
    const std::string text = Trim(line);
    if (text.empty() || text[0] == ';')
      continue;
    if (text[0] == '[') {
      const std::size_t close = text.find(']');
      if (close != std::string::npos) {
        currentSection = Trim(text.substr(1, close - 1));
        if (listSections)
          result.names.push_back(currentSection);
      }
      continue;
    }
    const std::size_t eq = text.find('=');
    if (eq == std::string::npos)
      continue;
    const std::string entryKey = Trim(text.substr(0, eq));
    std::string value = Trim(text.substr(eq + 1));
    if (value.size() >= 2 &&
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '\'' && value.back() == '\'')))
      value = value.substr(1, value.size() - 2);
    if (section != nullptr && !IEquals(currentSection, section))
      continue;
    if (listKeys) {
      result.names.push_back(entryKey);
      continue;
    }
    if (key != nullptr && IEquals(entryKey, key)) {
      result.found = true;
      result.value = value;
      break;
    }
  }
  std::fclose(in);
  if (listSections || listKeys)
    result.found = !result.names.empty();
  return result;
}

} // namespace

unsigned GetPrivateProfileStringA(const char *section, const char *key,
                                  const char *defaultValue, char *buffer,
                                  unsigned size, const char *file) {
  if (!buffer || size == 0)
    return 0;
  const IniResult result = ParseIni(file, section, key);
  std::string out;
  bool listForm = false;
  if (result.found && (section == nullptr || key == nullptr)) {
    //	The listing forms: names each null-terminated, then the final
    //	terminator.
    listForm = true;
    for (const std::string &name : result.names) {
      out += name;
      out += '\0';
    }
    out += '\0';
  } else if (result.found) {
    out = result.value;
  } else {
    out = defaultValue ? defaultValue : "";
  }
  const unsigned room = size - 1;
  const unsigned copy =
      out.size() < room ? static_cast<unsigned>(out.size()) : room;
  std::memcpy(buffer, out.data(), copy);
  if (!listForm)
    buffer[copy] = '\0';
  //	The list form counts the bytes copied excluding the final
  //	terminator (which may itself be truncated away).
  return listForm ? copy : copy;
}

unsigned GetPrivateProfileIntA(const char *section, const char *key,
                               unsigned defaultValue, const char *file) {
  const IniResult result = ParseIni(file, section, key);
  if (!result.found)
    return defaultValue;
  //	The documented contract: a found value that is not an integer
  //	reads as 0.
  const char *text = result.value.c_str();
  char *end = nullptr;
  const long parsed = std::strtol(text, &end, 0);
  if (end == text)
    return 0;
  return static_cast<unsigned>(parsed);
}

unsigned GetPrivateProfileStringW(const wchar_t *section, const wchar_t *key,
                                  const wchar_t *defaultValue,
                                  wchar_t *buffer, unsigned size,
                                  const wchar_t *file) {
  if (!buffer || size == 0)
    return 0;
  std::string narrowSection, narrowKey, narrowDefault, narrowFile;
  bool ok = true;
  ok = ok && (!section || pw153::platform::utf16_to_utf8(section, narrowSection));
  ok = ok && (!key || pw153::platform::utf16_to_utf8(key, narrowKey));
  ok = ok && (!defaultValue ||
              pw153::platform::utf16_to_utf8(defaultValue, narrowDefault));
  ok = ok && (!file || pw153::platform::utf16_to_utf8(file, narrowFile));
  if (!ok)
    return 0;
  //	Four UTF-8 bytes per wide char is the worst case; the copy back
  //	truncates to the wide room.
  std::vector<char> scratch(static_cast<std::size_t>(size) * 4 + 1, '\0');
  const unsigned bytes =
      GetPrivateProfileStringA(section ? narrowSection.c_str() : nullptr,
                                key ? narrowKey.c_str() : nullptr,
                                defaultValue ? narrowDefault.c_str() : "",
                                scratch.data(),
                                static_cast<unsigned>(scratch.size()),
                                file ? narrowFile.c_str() : nullptr);
  std::wstring wide;
  if (!pw153::platform::utf8_to_utf16(
          std::string_view(scratch.data(), bytes), wide))
    wide.clear();
  const unsigned room = size - 1;
  const unsigned copy =
      wide.size() < room ? static_cast<unsigned>(wide.size()) : room;
  std::memcpy(buffer, wide.data(), copy * sizeof(wchar_t));
  buffer[copy] = L'\0';
  return copy;
}

unsigned GetPrivateProfileIntW(const wchar_t *section, const wchar_t *key,
                               unsigned defaultValue, const wchar_t *file) {
  std::string narrowSection, narrowKey, narrowFile;
  bool ok = true;
  ok = ok && (!section || pw153::platform::utf16_to_utf8(section, narrowSection));
  ok = ok && (!key || pw153::platform::utf16_to_utf8(key, narrowKey));
  ok = ok && (!file || pw153::platform::utf16_to_utf8(file, narrowFile));
  if (!ok)
    return defaultValue;
  return GetPrivateProfileIntA(section ? narrowSection.c_str() : nullptr,
                               key ? narrowKey.c_str() : nullptr, defaultValue,
                               file ? narrowFile.c_str() : nullptr);
}

//	The INI writer's file serialization.
namespace {
bool WriteLines(const char *file, const std::vector<std::string> &lines) {
  std::FILE *out = std::fopen(file, "wb");
  if (!out)
    return false;
  for (const std::string &line : lines) {
    std::fputs(line.c_str(), out);
    std::fputc('\n', out);
  }
  std::fclose(out);
  return true;
}
} // namespace

unsigned WritePrivateProfileStringA(const char *section, const char *key,
                                    const char *value, const char *file) {
  if (!section || !file)
    return 0;
  //	Read the existing lines (an absent file starts empty).
  std::vector<std::string> lines;
  if (std::FILE *in = std::fopen(file, "rb")) {
    char raw[2048];
    while (std::fgets(raw, sizeof(raw), in)) {
      std::string line(raw);
      while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        line.pop_back();
      lines.push_back(line);
    }
    std::fclose(in);
  }
  //	Locate the section's line bounds [start, end).
  std::size_t start = lines.size(), end = lines.size();
  bool found = false;
  for (std::size_t i = 0; i < lines.size(); ++i) {
    const std::string text = Trim(lines[i]);
    if (text.size() >= 2 && text.front() == '[' && text.back() == ']') {
      if (found) {
        end = i;
        break;
      }
      if (IEquals(Trim(text.substr(1, text.size() - 2)), section)) {
        found = true;
        start = i;
        end = i + 1;
      }
    } else if (found && !text.empty() && text[0] != ';') {
      end = i + 1;
    }
  }
  //	The delete-section form (a null key removes the whole section).
  if (!key) {
    if (!found)
      return 1;	//	nothing to delete: the documented success.
    lines.erase(lines.begin() + static_cast<long>(start),
                lines.begin() + static_cast<long>(end));
    return WriteLines(file, lines) ? 1 : 0;
  }
  //	Locate the key's line within the section.
  std::size_t keyLine = lines.size();
  for (std::size_t i = start + 1; i < end; ++i) {
    const std::string text = Trim(lines[i]);
    const std::size_t eq = text.find('=');
    if (eq != std::string::npos && IEquals(Trim(text.substr(0, eq)), key)) {
      keyLine = i;
      break;
    }
  }
  //	The delete-key form (a null value removes the key).
  if (!value) {
    if (keyLine == lines.size())
      return 1;	//	nothing to delete.
    lines.erase(lines.begin() + static_cast<long>(keyLine));
    return WriteLines(file, lines) ? 1 : 0;
  }
  const std::string entry = std::string(key) + "=" + value;
  if (keyLine != lines.size()) {
    //	Replace the existing entry in place.
    lines[keyLine] = entry;
  } else if (found) {
    //	A new key appends inside its section.
    lines.insert(lines.begin() + static_cast<long>(end), entry);
  } else {
    //	A new section appends with its first entry.
    if (!lines.empty() && !lines.back().empty())
      lines.push_back("");
    lines.push_back("[" + std::string(section) + "]");
    lines.push_back(entry);
  }
  return WriteLines(file, lines) ? 1 : 0;
}

unsigned WritePrivateProfileStringW(const wchar_t *section,
                                     const wchar_t *key,
                                     const wchar_t *value,
                                     const wchar_t *file) {
  std::string narrowSection, narrowKey, narrowValue, narrowFile;
  bool ok = true;
  ok = ok && (!section || pw153::platform::utf16_to_utf8(section, narrowSection));
  ok = ok && (!key || pw153::platform::utf16_to_utf8(key, narrowKey));
  ok = ok && (!value || pw153::platform::utf16_to_utf8(value, narrowValue));
  ok = ok && (!file || pw153::platform::utf16_to_utf8(file, narrowFile));
  if (!ok)
    return 0;
  return WritePrivateProfileStringA(
      section ? narrowSection.c_str() : nullptr,
      key ? narrowKey.c_str() : nullptr,
      value ? narrowValue.c_str() : nullptr,
      file ? narrowFile.c_str() : nullptr);
}
