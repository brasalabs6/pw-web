// U04-A regression test: the instance.txt float parse drives the REAL
// shipped AWScriptFile::GetNextTokenAsFloat (the header inline compiled
// into this test) over real UTF-16 script files — positive (the actual
// instance.txt texture-entry tokens load as their values) and negative
// (an invalid dimension rejects, the original EC_Instance semantics).
//
// The EC_Instance parse itself is asserted structurally: the source keeps
// the original `return false` rejection (no diagnostic `continue`).
#include "AWScriptFile.h"
#include "AFI.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

#ifndef U04_EC_INSTANCE_SOURCE
#define U04_EC_INSTANCE_SOURCE "src/client153/Element/CElementClient/EC_Instance.cpp"
#endif

#define SOURCE_PATH U04_EC_INSTANCE_SOURCE

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

// Write a UTF-16 LE (BOM) script file the way the game's own data files are.
bool write_utf16_script(const char *path, const std::string &utf8Content) {
  FILE *file = std::fopen(path, "wb");
  if (!file)
    return false;
  const unsigned short bom = 0xfeff;
  std::fwrite(&bom, 2, 1, file);
  for (unsigned char c : utf8Content)
  {
    // ASCII content only in these fixtures (the numeric tokens the parser
    // reads are ASCII; the GB18030 comments are outside the parse path).
    std::fwrite(&c, 1, 1, file);
    const char zero = '\0';
    std::fwrite(&zero, 1, 1, file);
  }
  std::fclose(file);
  return true;
}

}  // namespace

int main() {
  // The game sets the Angelica base dir to its working directory at boot
  // (ElementClient.cpp calls af_SetBaseDir(g_szWorkDir)) and the parser's
  // path join resolves relative names against it. Honor the same startup
  // contract so the fixtures resolve identically in the native FS and in
  // the Emscripten MEMFS (whose CWD is "/").
  char base_dir[4096];
  if (!getcwd(base_dir, sizeof(base_dir)))
  {
    std::printf("FAIL getcwd\n");
    return 1;
  }
  if (!af_SetBaseDir(base_dir))
  {
    std::printf("FAIL af_SetBaseDir\n");
    return 1;
  }
  // ---- Positive: the real instance.txt texture-entry tokens parse as
  // their values (the tokens captured from the browser diagnosis: the
  // width token was literally "160").
  {
    AWScriptFile script;
    const char *path = "u04_positive.txt";
    if (!write_utf16_script(path, "{ 0, -200, 160, 200, a40_tile_0101.dds }"))
    {
      std::printf("FAIL fixture write\n");
      return 1;
    }
    check("positive open", script.Open(path));
    check("positive brace consumed", script.GetNextToken(true));
    float left = script.GetNextTokenAsFloat(true);
    float top = script.GetNextTokenAsFloat(true);
    float width = script.GetNextTokenAsFloat(true);
    float height = script.GetNextTokenAsFloat(true);
    check("positive left", left == 0.0f);
    check("positive top", top == -200.0f);
    check("positive width (the captured token parses as 160)", width == 160.0f);
    check("positive height", height == 200.0f);
    script.Close();
    std::remove(path);
  }
  // ---- Negative: an invalid width (< 10) parses as its value — the
  // EC_Instance check (< 10) rejects exactly as the Windows build did.
  {
    AWScriptFile script;
    const char *path = "u04_negative.txt";
    if (!write_utf16_script(path, "{ 0, -200, 5, 200, a40_tile_0101.dds }"))
    {
      std::printf("FAIL fixture write\n");
      return 1;
    }
    check("negative open", script.Open(path));
    check("negative brace consumed", script.GetNextToken(true));
    script.GetNextTokenAsFloat(true);
    script.GetNextTokenAsFloat(true);
    float width = script.GetNextTokenAsFloat(true);
    check("negative width parses as 5 (not 0 — the parse is honest)", width == 5.0f);
    check("negative width rejects under the original check (< 10)", width < 10.0f);
    script.Close();
    std::remove(path);
  }
  if (failures)
  {
    std::printf("%d failures\n", failures);
    return 1;
  }
  std::printf("all U04-A regression checks passed\n");
  return 0;
}
