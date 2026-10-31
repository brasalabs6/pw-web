// B08: the ps.1.1/ps.1.4 assembler covers the shipped pixel-shader
// corpus. Every ps.1.x shader in the authorized shaders.pck assembles
// into GLSL ES 300; the five ps_2_0 gfx effects fail with the honest
// classification (outside the port's assembler). The corpus drive is
// the real-data contract (the same files the browser boot loads).
#include "ps_assembler.h"

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
  // same cross-validation the vs corpus test uses.
  const std::string manifest =
      "import sys, json\n"
      "sys.path.insert(0, '" + std::string(argv[2]) + "')\n"
      "import pck153\n"
      "pkg = pck153.Package('" + std::string(argv[1]) + "')\n"
      "out = []\n"
      "for e in pkg.entries:\n"
      "    if '/ps/' in e.path and e.path.endswith('.txt'):\n"
      "        out.append([e.path, pkg.read(e).hex()])\n"
      "print(json.dumps(out))\n";
  const std::string manifestFile = "/tmp/grok-goal-e46c9ee49ecf/implementer/ps_manifest.py";
  FILE *py = std::fopen(manifestFile.c_str(), "wb");
  if (!py) {
    std::printf("FAIL manifest write\n");
    return 1;
  }
  std::fwrite(manifest.data(), 1, manifest.size(), py);
  std::fclose(py);
  const std::string cmd =
      "python3 " + manifestFile + " > /tmp/grok-goal-e46c9ee49ecf/implementer/ps_manifest.json";
  if (std::system(cmd.c_str()) != 0) {
    std::printf("FAIL manifest run\n");
    return 1;
  }
  const std::string extract =
      "python3 -c \"import json; d=json.load(open('/tmp/grok-goal-e46c9ee49ecf/"
      "implementer/ps_manifest.json'));"
      "print(len(d));"
      "[open('/tmp/grok-goal-e46c9ee49ecf/implementer/ps_'+str(i)+'.bin','wb')."
      "write(bytes.fromhex(h)) for i,(p,h) in enumerate(d)]\" > "
      "/tmp/grok-goal-e46c9ee49ecf/implementer/ps_count.txt";
  if (std::system(extract.c_str()) != 0) {
    std::printf("FAIL extract run\n");
    return 1;
  }
  FILE *countFile = std::fopen("/tmp/grok-goal-e46c9ee49ecf/implementer/ps_count.txt", "rb");
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

  int assembled = 0, honest22 = 0;
  for (int i = 0; i < total; ++i) {
    char path[128];
    std::snprintf(path, sizeof(path),
                  "/tmp/grok-goal-e46c9ee49ecf/implementer/ps_%d.bin", i);
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
    const bool ok = pw153::platform::AssemblePixelShader(
        source.data(), source.size(), glsl, error);
    if (!ok) {
      // The only honest failure class: the ps_2_0 gfx effects (the
      // shaders/2.2/ps set) sit outside the port's assembler.
      if (error.find("ps_2_0") != std::string::npos) {
        ++honest22;
        continue;
      }
      std::printf("FAIL shader %d assemble: %s\n", i, error.c_str());
      ++failures;
      continue;
    }
    // The structural checks on the emitted GLSL: the header, the
    // fragment output and balanced braces.
    if (glsl.find("#version 300 es") == std::string::npos ||
        glsl.find("fragColor") == std::string::npos) {
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
  check("every ps.1.x corpus shader assembles", assembled >= 160);
  check("the ps_2_0 gfx effects fail with the honest classification",
        honest22 == 5);
  if (!failures)
    std::printf("all ps assembler corpus checks passed\n");
  return failures ? 1 : 0;
}
