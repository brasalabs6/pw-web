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
bool is_relative(const std::string &token) {
  // The corpus carries both orders: c[a0.x + N] and c[N+a0.x] (no
  // spaces). The general form: a constant register whose index
  // expression mentions a0.
  return token.rfind("c[", 0) == 0 && token.find("a0") != std::string::npos;
}

bool relative_source_to_glsl(const std::string &token, std::string &out,
                             std::string &error) {
  const size_t open = token.find("c[");
  if (open == std::string::npos) {
    error = "malformed relative source: " + token;
    return false;
  }
  const size_t close = token.find(']', open);
  if (close == std::string::npos) {
    error = "malformed relative source: " + token;
    return false;
  }
  // The index expression between the brackets: the a0.x swizzle strips
  // to a0 (the int the GLSL side keeps); everything else passes.
  std::string inner = token.substr(open + 2, close - open - 2);
  const size_t dot = inner.find("a0.x");
  if (dot != std::string::npos)
    inner.erase(dot + 2, 2);  // "a0.x" -> "a0"
  std::string swizzle;
  const size_t after = close + 1;
  if (after < token.size() && token[after] == '.')
    swizzle = token.substr(after + 1);
  out = "c[" + inner + "]" + (swizzle.empty() ? "" : "." + swizzle);
  return true;
}

bool source_to_glsl(const std::string &token, std::string &out,
                    std::string &error) {
  // The relative constant addressing routes through its own form (the
  // componentwise/mad/dp ops accept it the same way mov does).
  if (is_relative(token))
    return relative_source_to_glsl(token, out, error);
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
// The source's effective swizzle for an op that consumes the first N
// components (dp3/dp4, the matrix ops): the source's own swizzle
// truncated to N (a partial swizzle replicates its last element — the
// D3D8 rule), or xyzw when the source carries none.
std::string effective_swizzle(const std::string &src, size_t n) {
  const size_t dot = src.find('.');
  const std::string swz = dot == std::string::npos ? std::string("xyzw")
                                                   : src.substr(dot + 1);
  std::string result;
  for (size_t i = 0; i < n; ++i)
    result += i < swz.size() ? swz[i] : swz.back();
  return result;
}

// The source expression with its swizzle replaced by the effective one
// (never a double swizzle).
std::string with_dims(const std::string &src, size_t n) {
  const size_t dot = src.find('.');
  const std::string base = dot == std::string::npos ? src : src.substr(0, dot);
  return base + "." + effective_swizzle(src, n);
}

// The GLSL vec constructor for the component count (vec2/vec3/vec4 —
// the GLSL constructors require the exact component count).
std::string vec_constructor(size_t count) {
  // GLSL ES has no vec1: the single-component constructor is float.
  return count == 1 ? "float" : "vec" + std::to_string(count);
}

// The assignment emission: the float constructor (the single-component
// case) takes no trailing swizzle — a float has no fields; the
// destination's own mask selects the component.
std::string emit_assign(const std::string &dest, const std::string &mask,
                        const std::string &constructor,
                        const std::string &parts) {
  if (constructor == "float")
    return dest + mask_to_glsl(mask) + " = " + constructor + "(" + parts + ");";
  return dest + mask_to_glsl(mask) + " = " + constructor + "(" + parts + ")" +
         mask_to_glsl(mask) + ";";
}

// The D3D8 component mapping: the written component i takes the
// source's swizzle's i-th element (a partial swizzle replicates its
// last element — ".x" means .xxxx), or the destination's own letter
// when the source carries no swizzle.
std::string src_component(const std::string &src, size_t index,
                          char fallback) {
  const size_t dot = src.find('.');
  if (dot == std::string::npos)
    return src + "." + std::string(1, fallback);
  const std::string swz = src.substr(dot + 1);
  const char c = index < swz.size() ? swz[index] : swz.back();
  return src.substr(0, dot + 1) + std::string(1, c);
}

}  // namespace

bool AssembleVertexShader(const char *source, size_t length,
                          std::string &glsl, std::string &error) {
  glsl.clear();
  error.clear();
  if (!source) {
    error = "null source";
    return false;
  }
  if (length == 0) {
    error = "empty source";
    return false;
  }

  bool sawVersion = false;
  bool writesPos = false, writesD0 = false, writesD1 = false;
  bool writesFog = false, writesPts = false;
  bool writesT[8] = {false, false, false, false, false, false, false, false};
  std::vector<std::string> body;

  // The parse is bounded by the caller's length: the device buffer is
  // the package's exact-size read with no NUL terminator, so the loop
  // must never scan past source + length.
  const char *cursor = source;
  const char *const end = source + length;
  while (cursor < end) {
    // One line at a time.
    const char *eol = static_cast<const char *>(
        std::memchr(cursor, '\n', static_cast<size_t>(end - cursor)));
    const size_t len = eol ? (size_t)(eol - cursor)
                           : static_cast<size_t>(end - cursor);
    std::string line(cursor, len);
    cursor = eol ? eol + 1 : end;

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
    bool scalarDest = false;
    if (!dest_to_glsl(tokens[1].text, dest, mask, error))
      return false;
    if (dest == "oFog" || dest == "oPts")
      scalarDest = true;

    // Record the outputs the shader writes. The scalar outputs (oFog,
    // oPts) take the first component's expression as a scalar.
    if (dest == "oPos")
      writesPos = true;
    else if (dest == "oD0")
      writesD0 = true;
    else if (dest == "oD1")
      writesD1 = true;
    else if (dest == "oFog") {
      writesFog = true;
      scalarDest = true;
    } else if (dest == "oPts") {
      writesPts = true;
      scalarDest = true;
    }
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
        stmt = "a0 = int(" + src_component(src, 0, 'x') + ");";
      } else if (is_relative(tokens[2].text)) {
        std::string src;
        if (!relative_source_to_glsl(tokens[2].text, src, error))
          return false;
        stmt = dest + mask_to_glsl(mask) + " = " + src + ";";
      } else {
        std::string src;
        if (!source_to_glsl(tokens[2].text, src, error))
          return false;
        if (scalarDest) {
          stmt = dest + " = " + src_component(src, 0, 'x') + ";";
        } else {
          const std::string letters = mask.empty() ? "xyzw" : mask;
          std::string parts;
          for (size_t i = 0; i < letters.size(); ++i) {
            if (!parts.empty()) parts += ", ";
            parts += src_component(src, i, letters[i]);
          }
          stmt = emit_assign(dest, mask, vec_constructor(letters.size()),
                             parts);
        }
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
      const std::string letters = scalarDest ? "x" : (mask.empty() ? "xyzw" : mask);
      std::string parts;
      for (size_t i = 0; i < letters.size(); ++i) {
        const char L = letters[i];
        const std::string ea = src_component(a, i, L);
        const std::string eb = src_component(b, i, L);
        std::string e;
        if (lower == "add") e = ea + " + " + eb;
        else if (lower == "sub") e = ea + " - " + eb;
        else if (lower == "mul") e = ea + " * " + eb;
        else if (lower == "min") e = "min(" + ea + ", " + eb + ")";
        else if (lower == "max") e = "max(" + ea + ", " + eb + ")";
        else if (lower == "slt") e = "(" + ea + " < " + eb + ") ? 1.0 : 0.0";
        else e = "(" + ea + " >= " + eb + ") ? 1.0 : 0.0";
        if (!parts.empty()) parts += ", ";
        parts += e;
      }
      stmt = scalarDest
                 ? dest + " = " + parts + ";"
                 : emit_assign(dest, mask, vec_constructor(letters.size()),
                               parts);
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
      const std::string letters = scalarDest ? "x" : (mask.empty() ? "xyzw" : mask);
      std::string parts;
      for (size_t i = 0; i < letters.size(); ++i) {
        const char L = letters[i];
        if (!parts.empty()) parts += ", ";
        parts += src_component(a, i, L) + " * " + src_component(b, i, L) +
                 " + " + src_component(c, i, L);
      }
      stmt = scalarDest
                 ? dest + " = " + parts + ";"
                 : emit_assign(dest, mask, vec_constructor(letters.size()),
                               parts);
    } else if (lower == "dp3" || lower == "dp4") {
      if (tokens.size() != 4) {
        error = lower + " arity: " + line;
        return false;
      }
      std::string a, b;
      if (!source_to_glsl(tokens[2].text, a, error) ||
          !source_to_glsl(tokens[3].text, b, error))
        return false;
      // The dot broadcasts to the masked components: repeat the scalar
      // into each masked channel.
      const size_t width = lower == "dp3" ? 3 : 4;
      std::string scalar = "dot(" + with_dims(a, width) + ", " +
                           with_dims(b, width) + ")";
      const std::string letters = scalarDest ? "x" : (mask.empty() ? "xyzw" : mask);
      std::string parts;
      for (size_t i = 0; i < letters.size(); ++i) {
        if (!parts.empty()) parts += ", ";
        parts += scalar;
      }
      stmt = scalarDest
                 ? dest + " = " + parts + ";"
                 : emit_assign(dest, mask, vec_constructor(letters.size()),
                               parts);
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
      const std::string letters = mask.empty() ? "xyzw" : mask;
      std::string parts;
      for (size_t i = 0; i < letters.size(); ++i) {
        if (!parts.empty()) parts += ", ";
        parts += i == 0 ? "1.0"
               : i == 1 ? src_component(a, 0, 'y') + " * " +
                          src_component(b, 0, 'y')
               : i == 2 ? src_component(a, 1, 'z')
                        : src_component(b, 1, 'w');
      }
      stmt = emit_assign(dest, mask, vec_constructor(letters.size()),
                         parts);
    } else if (lower == "lit") {
      if (tokens.size() != 3) {
        error = "lit arity: " + line;
        return false;
      }
      std::string a;
      if (!source_to_glsl(tokens[2].text, a, error))
        return false;
      // lit: x=1, y=max(0,src.y), z=(src.y>0&&src.z>0)?pow(src.w,src.z):0, w=1.
      const std::string sy = src_component(a, 0, 'y');
      const std::string sz = src_component(a, 1, 'z');
      const std::string sw = src_component(a, 2, 'w');
      const std::string letters = mask.empty() ? "xyzw" : mask;
      std::string parts;
      for (size_t i = 0; i < letters.size(); ++i) {
        if (!parts.empty()) parts += ", ";
        parts += i == 0 ? "1.0"
               : i == 1 ? "max(" + sy + ", 0.0)"
               : i == 2 ? "((" + sy + " > 0.0 && " + sz +
                          " > 0.0) ? pow(max(" + sw + ", 0.0), " + sz +
                          ") : 0.0)"
                        : "1.0";
      }
      stmt = emit_assign(dest, mask, vec_constructor(letters.size()),
                         parts);
    } else if (lower == "rcp") {
      if (tokens.size() != 3) {
        error = "rcp arity: " + line;
        return false;
      }
      std::string a;
      if (!source_to_glsl(tokens[2].text, a, error))
        return false;
      const std::string scalar = "1.0 / " + src_component(a, 0, 'x');
      const std::string letters =
          scalarDest ? "x" : (mask.empty() ? "xyzw" : mask);
      std::string parts;
      for (size_t i = 0; i < letters.size(); ++i) {
        if (!parts.empty()) parts += ", ";
        parts += scalar;
      }
      stmt = scalarDest
                 ? dest + " = " + parts + ";"
                 : emit_assign(dest, mask, vec_constructor(letters.size()),
                               parts);
    } else if (lower == "rsq") {
      if (tokens.size() != 3) {
        error = "rsq arity: " + line;
        return false;
      }
      std::string a;
      if (!source_to_glsl(tokens[2].text, a, error))
        return false;
      const std::string scalar =
          "inversesqrt(abs(" + src_component(a, 0, 'x') + "))";
      const std::string letters =
          scalarDest ? "x" : (mask.empty() ? "xyzw" : mask);
      std::string parts;
      for (size_t i = 0; i < letters.size(); ++i) {
        if (!parts.empty()) parts += ", ";
        parts += scalar;
      }
      stmt = scalarDest
                 ? dest + " = " + parts + ";"
                 : emit_assign(dest, mask, vec_constructor(letters.size()),
                               parts);
    } else if (lower == "exp") {
      if (tokens.size() != 3) {
        error = "exp arity: " + line;
        return false;
      }
      std::string a;
      if (!source_to_glsl(tokens[2].text, a, error))
        return false;
      // exp: x=2^floor(w), y=w-floor(w), z=2^w, w=1.
      const std::string w = src_component(a, 2, 'w');
      const std::string letters = mask.empty() ? "xyzw" : mask;
      std::string parts;
      for (size_t i = 0; i < letters.size(); ++i) {
        if (!parts.empty()) parts += ", ";
        parts += i == 0 ? "exp2(floor(" + w + "))"
               : i == 1 ? w + " - floor(" + w + ")"
               : i == 2 ? "exp2(" + w + ")" : "1.0";
      }
      stmt = emit_assign(dest, mask, vec_constructor(letters.size()),
                         parts);
    } else if (lower == "m4x4" || lower == "m4x3" || lower == "m3x3" ||
               lower == "m3x2") {
      if (tokens.size() != 4) {
        error = lower + " arity: " + line;
        return false;
      }
      const int rows = lower == "m4x4" ? 4 : lower == "m4x3" ? 4
                             : lower == "m3x3" ? 3 : 2;
      const int srcWidth = lower == "m4x4" ? 4 : lower == "m4x3" ? 3
                             : lower == "m3x3" ? 3 : 2;
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
      // The matrix source: a constant (c[N] or the relative c[a0.x + N])
      // whose rows feed the dot products.
      const std::string &matToken = tokens[3].text;
      std::string rowBase;  // The GLSL row base expression.
      if (is_relative(matToken)) {
        // c[a0.x + N] or c[N+a0.x]: the rows are the index expression
        // plus the row offset.
        const size_t open = matToken.find("c[");
        const size_t close = matToken.find(']', open);
        std::string inner = matToken.substr(open + 2, close - open - 2);
        const size_t dot = inner.find("a0.x");
        if (dot != std::string::npos)
          inner.erase(dot + 2, 2);  // "a0.x" -> "a0"
        rowBase = inner;
      } else {
        std::string baseReg;
        split_swizzle(matToken, baseReg);
        if (baseReg.empty() || baseReg[0] != 'c') {
          error = lower + " matrix source must be a constant: " + line;
          return false;
        }
        rowBase = std::to_string(std::atoi(baseReg.c_str() + 1));
      }
      std::string parts;
      for (size_t ci = 0; ci < effectiveMask.size(); ++ci) {
        if (!parts.empty())
          parts += ", ";
        parts += "dot(" + with_dims(srcExpr, (size_t)srcWidth) + ", c[" +
                 rowBase + " + " + std::to_string((int)ci) + "]." +
                 std::string("xyzw").substr(0, (size_t)srcWidth) + ")";
      }
      stmt = dest + mask_to_glsl(effectiveMask) + " = " +
             vec_constructor(effectiveMask.size()) + "(" + parts + ")" +
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
