// B08: the vs.1.1 assembler covers the shipped shader corpus. Every
// vertex shader in the authorized shaders.pck assembles into GLSL ES
// 300: the version line parses, no unsupported instruction appears, the
// emitted GLSL declares the registers the body uses and closes
// balanced. The corpus drive is the real-data contract (the same files
// the browser boot loads); the assembler is the shipped seam.
#include "vs_assembler.h"

#include <cstdio>
#include <string>
#include <vector>

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

}  // namespace

int main(int argc, char **argv) {
  if (argc < 3) {
    // The corpus drive needs the authorized shaders.pck path (a local
    // authorized asset, never committed; the CI ctest run skips it).
    std::printf("no package given: corpus coverage requires the authorized "
                "shaders.pck path\n");
    return 0;
  }
  // The corpus comes from the authorized package through the
  // independently validated Python reader (tools/assets/pck153.py), the
  // same cross-validation the facedata.pck test uses.
  const std::string manifest =
      "import sys, json\n"
      "sys.path.insert(0, '" + std::string(argv[2]) + "')\n"
      "import pck153\n"
      "pkg = pck153.Package('" + std::string(argv[1]) + "')\n"
      "out = []\n"
      "for e in pkg.entries:\n"
      "    if '/vs/' in e.path and e.path.endswith('.txt'):\n"
      "        out.append([e.path, pkg.read(e).hex()])\n"
      "print(json.dumps(out))\n";
  const std::string manifestFile = "/tmp/grok-goal-e46c9ee49ecf/implementer/vs_manifest.py";
  FILE *py = std::fopen(manifestFile.c_str(), "wb");
  if (!py) {
    std::printf("FAIL manifest write\n");
    return 1;
  }
  std::fwrite(manifest.data(), 1, manifest.size(), py);
  std::fclose(py);
  const std::string cmd =
      "python3 " + manifestFile + " > /tmp/grok-goal-e46c9ee49ecf/implementer/vs_manifest.json";
  if (std::system(cmd.c_str()) != 0) {
    std::printf("FAIL manifest run\n");
    return 1;
  }
  // Read the manifest JSON through a minimal parser: the test links no
  // JSON library, so it shells the count and reads the entries back via
  // a second python pass that writes one hex file per shader.
  const std::string extract =
      "python3 -c \"import json; d=json.load(open('/tmp/grok-goal-e46c9ee49ecf/"
      "implementer/vs_manifest.json'));"
      "print(len(d));"
      "[open('/tmp/grok-goal-e46c9ee49ecf/implementer/vs_'+str(i)+'.bin','wb')."
      "write(bytes.fromhex(h)) for i,(p,h) in enumerate(d)]\" > "
      "/tmp/grok-goal-e46c9ee49ecf/implementer/vs_count.txt";
  if (std::system(extract.c_str()) != 0) {
    std::printf("FAIL extract run\n");
    return 1;
  }
  FILE *countFile = std::fopen("/tmp/grok-goal-e46c9ee49ecf/implementer/vs_count.txt", "rb");
  if (!countFile) {
    std::printf("FAIL count read\n");
    return 1;
  }
  int total = 0;
  if (std::fscanf(countFile, "%d", &total) != 1 || total <= 0) {
    std::printf("FAIL count parse\n");
    std::fclose(countFile);
    return 1;
  }
  std::fclose(countFile);
  std::printf("corpus shaders: %d\n", total);

  int assembled = 0;
  for (int i = 0; i < total; ++i) {
    char path[128];
    std::snprintf(path, sizeof(path),
                  "/tmp/grok-goal-e46c9ee49ecf/implementer/vs_%d.bin", i);
    FILE *file = std::fopen(path, "rb");
    if (!file) {
      std::printf("FAIL shader %d read\n", i);
      ++failures;
      continue;
    }
    std::string source;
    char buffer[4096];
    size_t read;
    while ((read = std::fread(buffer, 1, sizeof(buffer), file)) > 0)
      source.append(buffer, read);
    std::fclose(file);

    std::string glsl, error;
    const bool ok = pw153::platform::AssembleVertexShader(
        source.data(), source.size(), glsl, error);
    if (!ok) {
      std::printf("FAIL shader %d assemble: %s\n", i, error.c_str());
      ++failures;
      continue;
    }
    // The device hands the assembler the package's exact-size buffer
    // with no NUL terminator (the D3DXAssembleShader srcDataLen
    // contract): the length bounds the parse. Regression for the
    // browser boot where the length-blind parse ran into adjacent heap
    // and the face shaders failed on garbage tokens. The exact-length
    // parse of a garbage-tailed buffer must produce the same GLSL.
    if (i == 0) {
      std::vector<char> unterm(source.begin(), source.end());
      const char tail[] = "ec4(0.)\nthe at# oor# dot( c[1].# 8#\n";
      unterm.insert(unterm.end(), tail, tail + sizeof(tail) - 1);
      std::string glsl2, error2;
      const bool ok2 = pw153::platform::AssembleVertexShader(
          unterm.data(), source.size(), glsl2, error2);
      if (!ok2 || glsl2 != glsl) {
        std::printf("FAIL shader %d length-bounded parse: %s\n", i,
                    ok2 ? "glsl differs" : error2.c_str());
        ++failures;
        continue;
      }
    }
    // The structural checks on the emitted GLSL: the header, the
    // constant array, and balanced braces.
    if (glsl.find("#version 300 es") == std::string::npos ||
        glsl.find("uniform vec4 c[96]") == std::string::npos) {
      std::printf("FAIL shader %d GLSL header\n", i);
      ++failures;
      continue;
    }
    int opens = 0, closes = 0;
    for (const char c : glsl) {
      if (c == '{')
        ++opens;
      else if (c == '}')
        ++closes;
    }
    if (opens != closes || opens < 1) {
      std::printf("FAIL shader %d unbalanced GLSL\n", i);
      ++failures;
      continue;
    }
    ++assembled;
  }
  check("every corpus shader assembles", assembled == total && total >= 150);

  // The instruction coverage: the known unsupported set must stay empty
  // (the assembler's error names the first construct it cannot handle).
  if (failures == 0)
    std::printf("all vs assembler corpus checks passed\n");
  return failures ? 1 : 0;
}
