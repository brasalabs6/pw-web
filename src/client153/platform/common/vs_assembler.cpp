/*
 * FILE: vs_assembler.cpp (platform seam)
 *
 * DESCRIPTION: Assembles the original D3D8 vertex-shader assembly
 * (vs.1.1) into GLSL ES 300. The D3D8 VS register model maps directly:
 *   v0-v15   vertex inputs (the D3DVSDE semantic numbers the game's
 *            declarations use: v0 position, v1 blendweight, v2
 *            blendindices, v3 normal, v7+ texture coordinates)
 *   c0-c95   constants (the game sets them with SetVertexShaderConstant;
 *            c[a0.x + n] relative addressing is supported)
 *   r0-r15   temporaries
 *   oPos     clip-space position (the D3D depth range maps with
 *            z' = 2z - w, the same convention the FVF path uses)
 *   oD0/oD1  diffuse/specular color outputs
 *   oT0-oT3  texture coordinate outputs
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#include "vs_assembler.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <vector>

namespace pw153::platform {
namespace {

struct Token {
  std::string text;
};

// Splits one instruction line into tokens: whitespace and commas are
// separators, and a bracketed group ([...], which may contain spaces —
// the relative constant addressing c[a0.x + 14]) stays one token.
void tokenize(const std::string &line, std::vector<Token> &tokens) {
  tokens.clear();
  std::string current;
  int brackets = 0;
  for (const char c : line) {
    const bool separator =
        (std::isspace(static_cast<unsigned char>(c)) || c == ',') &&
        brackets == 0;
    if (c == '[')
      ++brackets;
    else if (c == ']' && brackets > 0)
      --brackets;
    if (separator) {
      if (!current.empty()) {
        tokens.push_back({current});
        current.clear();
      }
    } else {
      current += c;
    }
  }
  if (!current.empty())
    tokens.push_back({current});
}

// Strips a trailing swizzle from a register token and returns it (the
// caller keeps the register part).
std::string split_swizzle(const std::string &token, std::string &reg) {
  const size_t dot = token.find('.');
  if (dot == std::string::npos) {
    reg = token;
    return "";
  }
  reg = token.substr(0, dot);
  return token.substr(dot + 1);
}

// The write mask as a GLSL swizzle string (".x", ".xy", ".xyz", "" for
// the full xyzw). Empty mask means all four components.
std::string mask_to_glsl(const std::string &mask) {
  return mask.empty() ? "" : "." + mask;
}

// The source operand as a GLSL expression: the negation, the register
// (with the constant index bracketed), the swizzle (default xyzw).
bool source_to_glsl(const std::string &token, std::string &out,
                    std::string &error) {
  std::string work = token;
  bool negate = false;
  if (!work.empty() && work[0] == '-') {
    negate = true;
    work.erase(0, 1);
  }
  std::string swizzle;
  std::string reg;
  swizzle = split_swizzle(work, reg);
  if (reg.size() < 2) {
    error = "malformed source operand: " + token;
    return false;
  }
  const char kind = reg[0];
  const int index = std::atoi(reg.c_str() + 1);
  std::string glslReg;
  if (kind == 'v')
    glslReg = "v" + std::to_string(index);
  else if (kind == 'r')
    glslReg = "r" + std::to_string(index);
  else if (kind == 'c')
    glslReg = "c[" + std::to_string(index) + "]";
  else {
    error = "unsupported source register: " + token;
    return false;
  }
  out = glslReg + (swizzle.empty() ? "" : "." + swizzle);
  if (negate)
    out = "-" + out;
  return true;
}

// The relative-addressing source: c[a0.x + N] (the swizzle after the
// bracket is part of the token text).
bool relative_source_to_glsl(const std::string &token, std::string &out,
                             std::string &error) {
  const size_t open = token.find("c[a0");
  if (open == std::string::npos) {
    error = "malformed relative source: " + token;
    return false;
  }
  const size_t close = token.find(']', open);
  if (close == std::string::npos) {
    error = "malformed relative source: " + token;
    return false;
  }
  std::string inner = token.substr(open + 2, close - open - 2);  // "a0.x + N"
  // Normalize: strip the .x swizzle from a0.
  const size_t dot = inner.find(".x");
  if (dot != std::string::npos)
    inner.erase(dot, 2);
  std::string swizzle;
  const size_t after = close + 1;
  if (after < token.size() && token[after] == '.')
    swizzle = token.substr(after + 1);
  out = "c[" + inner + "]" + (swizzle.empty() ? "" : "." + swizzle);
  return true;
}

bool is_relative(const std::string &token) {
  return token.find("c[a0") != std::string::npos;
}

// The destination operand: the register + the write mask. The outputs
// (oPos/oD0/oD1/oT0-oT7/oFog/oPts) map to their GLSL names.
bool dest_to_glsl(const std::string &token, std::string &out,
                  std::string &mask, std::string &error) {
  std::string swizzle;
  std::string reg;
  swizzle = split_swizzle(token, reg);
  if (reg == "oPos") {
    out = "oPos";
    mask = swizzle;
    return true;
  }
  if (reg == "oD0" || reg == "oD1" || reg == "oFog" || reg == "oPts" ||
      (reg.rfind("oT", 0) == 0 && reg.size() == 3 && reg[2] >= '0' &&
       reg[2] <= '7')) {
    out = reg;
    mask = swizzle;
    return true;
  }
  if (reg == "a0") {
    // The address register: only .x is writable in vs.1.1.
    out = "a0";
    mask = swizzle;
    return true;
  }
  if (reg.size() >= 2 && reg[0] == 'r') {
    const int index = std::atoi(reg.c_str() + 1);
    if (index < 0 || index > 15) {
      error = "temp register out of range: " + token;
      return false;
    }
    out = "r" + std::to_string(index);
    mask = swizzle;
    return true;
  }
  error = "unsupported destination register: " + token;
  return false;
}

// The per-component scalar expression for the componentwise ops with a
// partial write mask: the sources repeat the addressed component.
std::string swizzle_component(const std::string &src, const char component) {
  // The source already carries its own swizzle (or none = xyzw); the
  // componentwise op addresses the same component in each source.
  const size_t dot = src.find('.');
  if (dot == std::string::npos)
    return src + "." + std::string(1, component);
  // The source has a swizzle: pick its first component's letter.
  return src.substr(0, dot + 2);
}

}  // namespace

bool AssembleVertexShader(const char *source, std::string &glsl,
                          std::string &error) {
  glsl.clear();
  error.clear();
  if (!source) {
    error = "null source";
    return false;
  }

  bool sawVersion = false;
  bool writesPos = false, writesD0 = false, writesD1 = false;
  bool writesFog = false, writesPts = false;
  bool writesT[8] = {false, false, false, false, false, false, false, false};
  std::vector<std::string> body;

  const char *cursor = source;
  while (*cursor) {
    // One line at a time.
    const char *eol = std::strchr(cursor, '\n');
    const size_t len = eol ? (size_t)(eol - cursor) : std::strlen(cursor);
    std::string line(cursor, len);
    cursor = eol ? eol + 1 : cursor + len;

    // Strip both comment styles (';' and '//') and trim.
    const size_t semi = line.find(';');
    if (semi != std::string::npos)
      line.erase(semi);
    const size_t slash = line.find("//");
    if (slash != std::string::npos)
      line.erase(slash);
    while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())))
      line.pop_back();
    size_t start = 0;
    while (start < line.size() &&
           std::isspace(static_cast<unsigned char>(line[start])))
      ++start;
    if (start >= line.size())
      continue;
    line = line.substr(start);

    std::vector<Token> tokens;
    tokenize(line, tokens);
    if (tokens.empty())
      continue;

    const std::string op = tokens[0].text;
    std::string lower;
    for (const char c : op)
      lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

    if (lower.rfind("vs.", 0) == 0) {
      sawVersion = true;
      continue;
    }
    if (lower.rfind("dcl", 0) == 0)
      continue;  // The declarations wire through the vertex type, not here.
    if (lower == "nop")
      continue;

    if (tokens.size() < 2) {
      error = "instruction without destination: " + line;
      return false;
    }

    std::string dest, mask;
    if (!dest_to_glsl(tokens[1].text, dest, mask, error))
      return false;

    // Record the outputs the shader writes.
    if (dest == "oPos")
      writesPos = true;
    else if (dest == "oD0")
      writesD0 = true;
    else if (dest == "oD1")
      writesD1 = true;
    else if (dest == "oFog")
      writesFog = true;
    else if (dest == "oPts")
      writesPts = true;
    else if (dest.rfind("oT", 0) == 0 && dest.size() == 3) {
      const int ti = dest[2] - '0';
      if (ti >= 0 && ti < 8)
        writesT[ti] = true;
    }

    std::string stmt;
    if (lower == "mov") {
      if (tokens.size() != 3) {
        error = "mov arity: " + line;
        return false;
      }
      if (dest == "a0") {
        // mov a0.x, src: the address register takes the integer value.
        std::string src;
        if (!source_to_glsl(tokens[2].text, src, error))
          return false;
        stmt = "a0 = int(" + swizzle_component(src, 'x') + ");";
      } else if (is_relative(tokens[2].text)) {
        std::string src;
        if (!relative_source_to_glsl(tokens[2].text, src, error))
          return false;
        stmt = dest + mask_to_glsl(mask) + " = " + src + ";";
      } else {
        std::string src;
        if (!source_to_glsl(tokens[2].text, src, error))
          return false;
        stmt = dest + mask_to_glsl(mask) + " = " + src + mask_to_glsl(mask) + ";";
      }
    } else if (lower == "add" || lower == "sub" || lower == "mul" ||
               lower == "min" || lower == "max" || lower == "slt" ||
               lower == "sge") {
      if (tokens.size() != 4) {
        error = lower + " arity: " + line;
        return false;
      }
      std::string a, b;
      if (!source_to_glsl(tokens[2].text, a, error) ||
          !source_to_glsl(tokens[3].text, b, error))
        return false;
      const char *glslOp = lower == "add"    ? "+"
                           : lower == "sub"  ? "-"
                           : lower == "mul"  ? "*"
                           : lower == "min"  ? "min"
                           : lower == "max"  ? "max"
                           : lower == "slt"  ? "lessThan"
                           : "greaterThanEqual";
      if (glslOp[0] == '-' || glslOp[0] == '+') {
        stmt = dest + mask_to_glsl(mask) + " = " + a + mask_to_glsl(mask) +
               " " + glslOp + " " + b + mask_to_glsl(mask) + ";";
      } else {
        // min/max/slt/sge: the componentwise form via the masked vec4.
        const std::string cmp = lower == "slt" ? "lessThan" : "greaterThanEqual";
        if (lower == "slt" || lower == "sge") {
          // The compare yields bvec; select componentwise with the mask.
          stmt = dest + mask_to_glsl(mask) + " = vec4(" + cmp + "(" +
                 a + ", " + b + "))" + mask_to_glsl(mask) + ";";
        } else {
          stmt = dest + mask_to_glsl(mask) + " = " + glslOp + "(" +
                 a + mask_to_glsl(mask) + ", " + b + mask_to_glsl(mask) + ");";
        }
      }
    } else if (lower == "mad") {
      if (tokens.size() != 5) {
        error = "mad arity: " + line;
        return false;
      }
      std::string a, b, c;
      if (!source_to_glsl(tokens[2].text, a, error) ||
          !source_to_glsl(tokens[3].text, b, error) ||
          !source_to_glsl(tokens[4].text, c, error))
        return false;
      stmt = dest + mask_to_glsl(mask) + " = " + a + mask_to_glsl(mask) +
             " * " + b + mask_to_glsl(mask) + " + " + c + mask_to_glsl(mask) + ";";
    } else if (lower == "dp3" || lower == "dp4") {
      if (tokens.size() != 4) {
        error = lower + " arity: " + line;
        return false;
      }
      std::string a, b;
      if (!source_to_glsl(tokens[2].text, a, error) ||
          !source_to_glsl(tokens[3].text, b, error))
        return false;
      const std::string dims = lower == "dp3" ? ".xyz" : "";
      // The dot broadcasts to the masked components: repeat the scalar
      // into each masked channel.
      std::string scalar = "dot(" + a + dims + ", " + b + dims + ")";
      if (mask.empty() || mask == "xyzw")
        stmt = dest + " = vec4(" + scalar + ");";
      else {
        stmt = dest + mask_to_glsl(mask) + " = vec4(" + scalar +
               ")." + mask + ";";
      }
    } else if (lower == "dst") {
      if (tokens.size() != 4) {
        error = "dst arity: " + line;
        return false;
      }
      std::string a, b;
      if (!source_to_glsl(tokens[2].text, a, error) ||
          !source_to_glsl(tokens[3].text, b, error))
        return false;
      // dst: x=1, y=src0.y*src1.y, z=src0.z, w=src1.w.
      stmt = dest + mask_to_glsl(mask) + " = vec4(1.0, " +
             swizzle_component(a, 'y') + " * " + swizzle_component(b, 'y') +
             ", " + swizzle_component(a, 'z') + ", " +
             swizzle_component(b, 'w') + ")" + mask_to_glsl(mask) + ";";
    } else if (lower == "lit") {
      if (tokens.size() != 3) {
        error = "lit arity: " + line;
        return false;
      }
      std::string a;
      if (!source_to_glsl(tokens[2].text, a, error))
        return false;
      // lit: x=1, y=max(0,src.y), z=(src.y>0&&src.z>0)?pow(src.w,src.z):0, w=1.
      const std::string sy = swizzle_component(a, 'y');
      const std::string sz = swizzle_component(a, 'z');
      const std::string sw = swizzle_component(a, 'w');
      stmt = dest + mask_to_glsl(mask) + " = vec4(1.0, max(" + sy +
             ", 0.0), ((" + sy + " > 0.0 && " + sz +
             " > 0.0) ? pow(max(" + sw + ", 0.0), " + sz + ") : 0.0), 1.0)" +
             mask_to_glsl(mask) + ";";
    } else if (lower == "rcp") {
      if (tokens.size() != 3) {
        error = "rcp arity: " + line;
        return false;
      }
      std::string a;
      if (!source_to_glsl(tokens[2].text, a, error))
        return false;
      std::string scalar = "1.0 / " + swizzle_component(a, 'x');
      if (mask.empty() || mask == "xyzw")
        stmt = dest + " = vec4(" + scalar + ");";
      else
        stmt = dest + mask_to_glsl(mask) + " = vec4(" + scalar +
               ")." + mask + ";";
    } else if (lower == "rsq") {
      if (tokens.size() != 3) {
        error = "rsq arity: " + line;
        return false;
      }
      std::string a;
      if (!source_to_glsl(tokens[2].text, a, error))
        return false;
      std::string scalar = "inversesqrt(abs(" + swizzle_component(a, 'x') + "))";
      if (mask.empty() || mask == "xyzw")
        stmt = dest + " = vec4(" + scalar + ");";
      else
        stmt = dest + mask_to_glsl(mask) + " = vec4(" + scalar +
               ")." + mask + ";";
    } else if (lower == "exp") {
      if (tokens.size() != 3) {
        error = "exp arity: " + line;
        return false;
      }
      std::string a;
      if (!source_to_glsl(tokens[2].text, a, error))
        return false;
      // exp: x=2^floor(w), y=w-floor(w), z=2^w, w=1.
      const std::string w = swizzle_component(a, 'w');
      stmt = dest + mask_to_glsl(mask) + " = vec4(exp2(floor(" + w +
             ")), " + w + " - floor(" + w + "), exp2(" + w +
             "), 1.0)" + mask_to_glsl(mask) + ";";
    } else if (lower == "m4x4" || lower == "m4x3" || lower == "m3x3" ||
               lower == "m3x2") {
      if (tokens.size() != 4) {
        error = lower + " arity: " + line;
        return false;
      }
      std::string a;
      std::string baseReg;
      std::string swz;
      swz = split_swizzle(tokens[3].text, baseReg);
      if (baseReg.empty() || baseReg[0] != 'c') {
        error = lower + " matrix source must be a constant: " + line;
        return false;
      }
      const int base = std::atoi(baseReg.c_str() + 1);
      const std::string srcSwz = swz.empty() ? "xyzw" : swz;
      const int rows = lower == "m4x4" ? 4 : lower == "m4x3" ? 4
                             : lower == "m3x3" ? 3 : 2;
      const int srcWidth = lower == "m4x4" ? 4 : lower == "m4x3" ? 3
                             : lower == "m3x3" ? 3 : 2;
      const std::string dims = "." + srcSwz.substr(0, (size_t)srcWidth);
      // The D3D8 matrix ops write the FIRST N components of the
      // destination (N = the op's row count) when no mask is given; an
      // explicit mask must not exceed the row count.
      const std::string effectiveMask =
          mask.empty() ? std::string("xyzw").substr(0, (size_t)rows) : mask;
      if ((int)effectiveMask.size() > rows) {
        error = lower + " writes more components than rows: " + line;
        return false;
      }
      std::string srcExpr;
      if (!source_to_glsl(tokens[2].text, srcExpr, error))
        return false;
      const std::string dims2 = "." + srcSwz.substr(0, (size_t)srcWidth);
      std::string parts;
      for (size_t ci = 0; ci < effectiveMask.size(); ++ci) {
        if (!parts.empty())
          parts += ", ";
        parts += "dot(" + srcExpr + dims2 + ", c[" +
                 std::to_string(base + (int)ci) + "]" + dims2 + ")";
      }
      stmt = dest + mask_to_glsl(effectiveMask) + " = vec4(" + parts + ")" +
             mask_to_glsl(effectiveMask) + ";";
    } else {
      error = "unsupported instruction: " + line;
      return false;
    }
    body.push_back(stmt);
  }

  if (!sawVersion) {
    error = "no vs.x.y version line";
    return false;
  }

  // The GLSL assembly.
  std::string out;
  out += "#version 300 es\n";
  out += "precision highp float;\n";
  for (int i = 0; i < 16; ++i)
    out += "in vec4 v" + std::to_string(i) + ";\n";
  out += "uniform vec4 c[96];\n";
  for (int i = 0; i < 8; ++i)
    if (writesT[i])
      out += "out vec4 oT" + std::to_string(i) + ";\n";
  if (writesD0)
    out += "out vec4 oD0;\n";
  if (writesD1)
    out += "out vec4 oD1;\n";
  if (writesFog)
    out += "out float oFog;\n";
  if (writesPts)
    out += "out float oPts;\n";
  out += "void main() {\n";
  for (int i = 0; i < 16; ++i)
    out += "vec4 r" + std::to_string(i) + " = vec4(0.0);\n";
  out += "int a0 = 0;\n";
  if (writesPos)
    out += "vec4 oPos = vec4(0.0, 0.0, 0.0, 1.0);\n";
  if (writesD0)
    out += "vec4 oD0 = vec4(0.0, 0.0, 0.0, 0.0);\n";
  if (writesD1)
    out += "vec4 oD1 = vec4(0.0, 0.0, 0.0, 0.0);\n";
  if (writesFog)
    out += "float oFog = 0.0;\n";
  if (writesPts)
    out += "float oPts = 0.0;\n";
  for (int i = 0; i < 8; ++i)
    if (writesT[i])
      out += "vec4 oT" + std::to_string(i) + " = vec4(0.0);\n";
  for (const std::string &stmt : body)
    out += stmt + "\n";
  if (writesPos)
    out += "gl_Position = vec4(oPos.x, oPos.y, 2.0 * oPos.z - oPos.w, oPos.w);\n";
  out += "}\n";
  glsl = out;
  return true;
}

}  // namespace pw153::platform
