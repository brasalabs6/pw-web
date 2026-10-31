/*
 * FILE: ps_assembler.cpp (platform seam)
 *
 * DESCRIPTION: Translates the shipped ps.1.1/ps.1.4 shader assembly
 * into GLSL ES 300 fragment-shader source. The register model, the
 * co-issue pairs, the instruction modifiers (_sat/_x2/_x4/_x8/_d2) and
 * the source modifiers (_bias/_bx2/_comp/_x2/_dw and the 1- invert)
 * follow the D3D8 ps.1.x contract; the emitted GLSL keeps the D3D
 * read-before-write semantics by building each statement's right side
 * as one vector expression the assignment consumes atomically.
 */

#include "ps_assembler.h"

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
// separators.
void tokenize(const std::string &line, std::vector<Token> &tokens) {
  tokens.clear();
  std::string current;
  for (const char c : line) {
    if (std::isspace(static_cast<unsigned char>(c)) || c == ',') {
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

// The written components of a destination write mask as GLSL letters
// (x/y/z/w). Empty, rgba and xyzw all mean the full four.
std::string dest_letters(const std::string &mask, std::string &error,
                         const std::string &context) {
  if (mask.empty())
    return "xyzw";
  std::string out;
  for (const char c : mask) {
    switch (c) {
      case 'r': case 'x': out += 'x'; break;
      case 'g': case 'y': out += 'y'; break;
      case 'b': case 'z': out += 'z'; break;
      case 'a': case 'w': out += 'w'; break;
      default:
        error = "invalid write mask ." + mask + " in: " + context;
        return "";
    }
  }
  return out;
}

// One source operand, resolved to a GLSL expression per component. The
// D3D read semantics: the i-th written destination component takes the
// source's i-th component (the explicit swizzle remaps, a shorter
// swizzle replicates its last letter, no swizzle is the identity).
struct Source {
  std::string comp[4];  // GLSL expressions for the source's xyzw
  bool valid = false;
};

// The register's plain component expressions (before swizzle/modifiers).
bool register_components(const std::string &reg, std::string comp[4],
                         std::string &error) {
  if (reg.size() < 2) {
    error = "malformed register: " + reg;
    return false;
  }
  const char kind = reg[0];
  const int index = std::atoi(reg.c_str() + 1);
  std::string base;
  if (kind == 'r')
    base = "r" + std::to_string(index);
  else if (kind == 't')
    base = "t" + std::to_string(index);
  else if (kind == 'c')
    // The pixel-constant uniform is pc[8]: the linked program shares
    // one uniform namespace with the vertex stage's c[96], so the two
    // constant banks carry distinct names.
    base = "pc[" + std::to_string(index) + "]";
  else if (kind == 'v')
    base = index == 0 ? "oD0" : index == 1 ? "oD1" : "";
  else {
    error = "unsupported source register: " + reg;
    return false;
  }
  if (base.empty()) {
    error = "unsupported color interpolant: " + reg;
    return false;
  }
  static const char *letters = "xyzw";
  for (int i = 0; i < 4; ++i)
    comp[i] = base + "." + letters[i];
  return true;
}

// Parses one source token (register, optional _modifier, optional
// swizzle, optional 1- invert or - negate) into per-component
// expressions.
bool parse_source(const std::string &token, Source &src, std::string &error) {
  std::string work = token;
  bool invert = false, negate = false;
  if (work.rfind("1-", 0) == 0) {
    invert = true;
    work.erase(0, 2);
  }
  if (!work.empty() && work[0] == '-') {
    negate = true;
    work.erase(0, 1);
  }
  std::string swizzle;
  const size_t dot = work.find('.');
  if (dot != std::string::npos) {
    swizzle = work.substr(dot + 1);
    work = work.substr(0, dot);
  }
  std::string mod;
  if (work.size() >= 2 && work[1] >= '0' && work[1] <= '9') {
    const size_t under = work.find('_', 2);
    if (under != std::string::npos) {
      mod = work.substr(under + 1);
      work = work.substr(0, under);
    }
  }
  std::string comp[4];
  if (!register_components(work, comp, error))
    return false;
  // The projection modifier replaces the base components before the
  // swizzle (the ps.1.4 _dw form: the coordinates divide by w).
  if (mod == "dw") {
    if (work[0] != 't') {
      error = "_dw projection on a non-texture register: " + token;
      return false;
    }
    comp[0] = "(" + work + ".x / " + work + ".w)";
    comp[1] = "(" + work + ".y / " + work + ".w)";
    // z and w pass through.
  } else if (!mod.empty() && mod != "bias" && mod != "bx2" &&
             mod != "comp" && mod != "x2") {
    error = "unsupported source modifier _" + mod + " in: " + token;
    return false;
  }
  // The swizzle remaps: component i of the source = the register's
  // swizzle[i] component; a shorter swizzle replicates its last letter.
  std::string mapped[4];
  static const char *letters = "xyzw";
  for (int i = 0; i < 4; ++i) {
    const char pick = swizzle.empty()
                          ? letters[i]
                          : swizzle[i < (int)swizzle.size() ? i
                                                            : (int)swizzle.size() - 1];
    // The D3D channel letters map to the GLSL components.
    char glslPick = pick;
    if (pick == 'r') glslPick = 'x';
    else if (pick == 'g') glslPick = 'y';
    else if (pick == 'b') glslPick = 'z';
    else if (pick == 'a') glslPick = 'w';
    int slot = -1;
    for (int j = 0; j < 4; ++j)
      if (letters[j] == glslPick)
        slot = j;
    if (slot < 0) {
      error = "invalid source swizzle ." + swizzle + " in: " + token;
      return false;
    }
    mapped[i] = comp[slot];
  }
  for (int i = 0; i < 4; ++i) {
    std::string e = mapped[i];
    if (mod == "bias")
      e = "(" + e + " - 0.5)";
    else if (mod == "bx2")
      e = "(" + e + " * 2.0 - 1.0)";
    else if (mod == "comp")
      e = "(1.0 - " + e + ")";
    else if (mod == "x2")
      e = "(" + e + " * 2.0)";
    if (invert)
      e = "(1.0 - " + e + ")";
    if (negate)
      e = "-" + e;
    src.comp[i] = e;
  }
  src.valid = true;
  return true;
}

// The vecN constructor for the written mask's components.
std::string vec_of(const std::string &letters,
                   const std::string *values) {
  static const char *all = "xyzw";
  if (letters.size() == 1) {
    const char *p = std::strchr(all, letters[0]);
    return values[p - all];
  }
  std::string out = letters.size() == 4
                        ? "vec4("
                        : letters.size() == 3 ? "vec3(" : "vec2(";
  for (size_t i = 0; i < letters.size(); ++i) {
    if (i)
      out += ", ";
    const char *p = std::strchr(all, letters[i]);
    out += values[p - all];
  }
  out += ")";
  return out;
}

// The destination register name for a parsed token (r0.rgb -> r0).
bool dest_register(const std::string &token, std::string &reg,
                   std::string &mask, std::string &error) {
  const size_t dot = token.find('.');
  if (dot == std::string::npos) {
    reg = token;
    mask.clear();
  } else {
    reg = token.substr(0, dot);
    mask = token.substr(dot + 1);
  }
  if (reg.size() < 2 || reg[0] != 'r') {
    error = "unsupported destination register: " + token;
    return false;
  }
  return true;
}

struct Used {
  bool r[6] = {}, t[6] = {}, v[2] = {}, sampler[8] = {};
  bool c = false, bump = false, bumpL = false;
};

// The register name a source token refers to (the 1- invert and -
// negate prefixes and the modifier/swizzle suffixes stripped).
std::string source_register_name(const std::string &token) {
  std::string work = token;
  if (work.rfind("1-", 0) == 0)
    work.erase(0, 2);
  if (!work.empty() && work[0] == '-')
    work.erase(0, 1);
  const size_t cut = work.find_first_of("._");
  return cut == std::string::npos ? work : work.substr(0, cut);
}

// Marks a source register's usage (the varying/uniform declarations).
void mark_source(const std::string &token, Used &used, std::string &error) {
  const std::string reg = source_register_name(token);
  if (reg.size() < 2) {
    error = "malformed register: " + token;
    return;
  }
  const char kind = reg[0];
  const int index = std::atoi(reg.c_str() + 1);
  if (kind == 'r') {
    if (index < 0 || index >= 6) {
      error = "temporary register out of range: " + reg;
      return;
    }
    used.r[index] = true;
  } else if (kind == 't') {
    if (index < 0 || index >= 6) {
      error = "texture register out of range: " + reg;
      return;
    }
    used.t[index] = true;
  } else if (kind == 'c') {
    if (index < 0 || index >= 8) {
      error = "constant register out of range: " + reg;
      return;
    }
    used.c = true;
  } else if (kind == 'v') {
    if (index < 0 || index >= 2) {
      error = "color interpolant out of range: " + reg;
      return;
    }
    used.v[index] = true;
  }
}

// The instruction modifiers: the suffix chain _sat/_x2/_x4/_x8/_d2.
struct OpMods {
  bool sat = false;
  int scale = 1;  // 1, 2, 4, 8; -2 encodes _d2 (half).
};

bool parse_op_mods(std::string &op, OpMods &mods) {
  for (;;) {
    if (op.size() > 4 && op.compare(op.size() - 4, 4, "_sat") == 0) {
      mods.sat = true;
      op.erase(op.size() - 4);
    } else if (op.size() > 3 && op.compare(op.size() - 3, 3, "_x2") == 0) {
      mods.scale = mods.scale == 1 ? 2 : mods.scale * 2;
      op.erase(op.size() - 3);
    } else if (op.size() > 3 && op.compare(op.size() - 3, 3, "_x4") == 0) {
      mods.scale = mods.scale == 1 ? 4 : mods.scale * 4;
      op.erase(op.size() - 3);
    } else if (op.size() > 3 && op.compare(op.size() - 3, 3, "_x8") == 0) {
      mods.scale = mods.scale == 1 ? 8 : mods.scale * 8;
      op.erase(op.size() - 3);
    } else if (op.size() > 3 && op.compare(op.size() - 3, 3, "_d2") == 0) {
      mods.scale = -2;
      op.erase(op.size() - 3);
    } else {
      break;
    }
  }
  return true;
}

// Wraps one component expression with the statement's modifiers.
std::string apply_mods(const std::string &expr, const OpMods &mods) {
  std::string out = expr;
  if (mods.scale == -2)
    out = "(" + out + " * 0.5)";
  else if (mods.scale != 1)
    out = "(" + out + " * " + std::to_string(mods.scale) + ".0)";
  if (mods.sat)
    out = "clamp(" + out + ", 0.0, 1.0)";
  return out;
}

}  // namespace

bool AssemblePixelShader(const char *source, size_t length, std::string &glsl,
                         std::string &error,
                         std::vector<PixelShaderDef> *defs) {
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

  enum class Version { None, Ps11, Ps14 };
  Version version = Version::None;
  bool sawVersion = false;
  Used used;
  std::vector<std::string> body;

  const char *cursor = source;
  const char *const end = source + length;
  while (cursor < end) {
    const char *eol = static_cast<const char *>(
        std::memchr(cursor, '\n', static_cast<size_t>(end - cursor)));
    const size_t len = eol ? (size_t)(eol - cursor)
                           : static_cast<size_t>(end - cursor);
    std::string line(cursor, len);
    cursor = eol ? eol + 1 : end;

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

    // The co-issue marker: "+mov" or "+ mov".
    bool coissue = false;
    if (tokens[0].text == "+") {
      coissue = true;
      tokens.erase(tokens.begin());
    } else if (tokens[0].text.size() > 1 && tokens[0].text[0] == '+') {
      coissue = true;
      tokens[0].text.erase(0, 1);
    }
    if (tokens.empty()) {
      error = "dangling co-issue marker: " + line;
      return false;
    }
    (void)coissue;  // The pairs write disjoint masks; the sequential
                    // GLSL statements preserve the semantics.

    std::string op = tokens[0].text;
    std::string lower;
    for (const char c : op)
      lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

    if (lower.rfind("ps.", 0) == 0) {
      if (lower == "ps.1.1")
        version = Version::Ps11;
      else if (lower == "ps.1.4")
        version = Version::Ps14;
      else {
        error = "unsupported pixel shader version: " + op;
        return false;
      }
      sawVersion = true;
      continue;
    }
    if (lower == "ps_2_0") {
      error = "ps_2_0 shaders are outside the port's assembler (the "
              "shipped 2.2 gfx effects carry them)";
      return false;
    }
    if (!sawVersion) {
      error = "instruction before the version line: " + line;
      return false;
    }
    if (lower == "phase" || lower == "nop")
      continue;
    if (lower.rfind("dcl", 0) == 0)
      continue;  // The ps.1.x samplers are implied by tex/texld.

    const int rMax = version == Version::Ps11 ? 2 : 6;
    const int tMax = version == Version::Ps11 ? 4 : 6;

    if (lower == "def") {
      // def cN, x, y, z, w — the constant's initial value.
      if (tokens.size() != 6) {
        error = "def arity: " + line;
        return false;
      }
      const std::string &reg = tokens[1].text;
      if (reg.size() < 2 || reg[0] != 'c') {
        error = "def destination: " + line;
        return false;
      }
      const int index = std::atoi(reg.c_str() + 1);
      if (index < 0 || index >= 8) {
        error = "def constant out of range: " + line;
        return false;
      }
      if (defs) {
        PixelShaderDef entry;
        entry.reg = index;
        for (int i = 0; i < 4; ++i)
          entry.value[i] = std::strtof(tokens[2 + i].text.c_str(), nullptr);
        defs->push_back(entry);
      }
      used.c = true;
      continue;
    }

    // The texture instructions.
    if (lower == "tex" || lower == "texcoord" || lower == "texbem" ||
        lower == "texbeml" || lower == "texkill") {
      if (tokens.size() < 2) {
        error = "texture instruction arity: " + line;
        return false;
      }
      const std::string &reg = tokens[1].text;
      // texkill kills on a texture register in ps.1.1 and on a
      // temporary in ps.1.4; the other texture instructions write tN.
      const bool killDest = lower == "texkill" && !reg.empty() &&
                            (reg[0] == 't' || reg[0] == 'r');
      if (reg.size() < 2 || (!killDest && reg[0] != 't')) {
        error = "texture instruction destination: " + line;
        return false;
      }
      const int index = std::atoi(reg.c_str() + 1);
      const int indexMax = reg[0] == 'r' ? rMax : tMax;
      if (index < 0 || index >= indexMax) {
        error = "texture register out of range: " + line;
        return false;
      }
      if (reg[0] == 'r')
        used.r[index] = true;
      else
        used.t[index] = true;
      if (lower == "texkill") {
        body.push_back("if (any(lessThan(" + reg.substr(0, reg.find('.')) +
                       ", vec4(0.0)))) discard;");
        continue;
      }
      if (index >= 8) {
        error = "sampler stage out of range: " + line;
        return false;
      }
      if (lower == "tex")
        used.sampler[index] = true;  // texcoord reads the coords only.
      if (lower == "tex") {
        body.push_back("t" + std::to_string(index) + " = texture(s" +
                       std::to_string(index) + ", oT" + std::to_string(index) +
                       ".xy);");
        continue;
      }
      if (lower == "texcoord") {
        body.push_back("t" + std::to_string(index) + " = oT" +
                       std::to_string(index) + ";");
        continue;
      }
      // texbem/texbeml: the bump-env-mapped sample.
      if (tokens.size() < 3) {
        error = "texbem arity: " + line;
        return false;
      }
      const std::string &src = tokens[2].text;
      if (src.size() < 2 || src[0] != 't') {
        error = "texbem source: " + line;
        return false;
      }
      const int srcIndex = std::atoi(src.c_str() + 1);
      if (srcIndex < 0 || srcIndex >= tMax) {
        error = "texbem source out of range: " + line;
        return false;
      }
      used.t[srcIndex] = true;
      used.bump = true;
      const std::string t = "t" + std::to_string(index);
      const std::string s = "t" + std::to_string(srcIndex);
      const std::string n = std::to_string(index);
      std::string stmt =
          t + " = texture(s" + n + ", oT" + n + ".xy + vec2(u_bump[" + n +
          "].x * " + s + ".x + u_bump[" + n + "].y * " + s + ".y, u_bump[" +
          n + "].z * " + s + ".x + u_bump[" + n + "].w * " + s + ".y));";
      if (lower == "texbeml") {
        used.bumpL = true;
        stmt += " " + t + ".a += u_bumpL[" + n + "] * " + s + ".b;";
      }
      body.push_back(stmt);
      continue;
    }

    if (lower == "texld" || lower == "texcrd") {
      if (version != Version::Ps14) {
        error = op + " requires ps.1.4: " + line;
        return false;
      }
      if (tokens.size() < 3) {
        error = op + " arity: " + line;
        return false;
      }
      std::string destReg, destMask;
      if (!dest_register(tokens[1].text, destReg, destMask, error))
        return false;
      const int destIndex = std::atoi(destReg.c_str() + 1);
      if (destIndex < 0 || destIndex >= rMax) {
        error = "temporary register out of range: " + line;
        return false;
      }
      used.r[destIndex] = true;
      const std::string letters = dest_letters(destMask, error, line);
      if (letters.empty())
        return false;
      // The source: the coordinate register (with the optional _dw
      // projection and swizzle) and, for texld, the optional sampler.
      std::string sampler;
      if (lower == "texld" && tokens.size() >= 4) {
        const std::string &s = tokens[3].text;
        if (s.size() < 2 || s[0] != 's') {
          error = "texld sampler: " + line;
          return false;
        }
        sampler = s;
      }
      Source src;
      if (!parse_source(tokens[2].text, src, error))
        return false;
      mark_source(tokens[2].text, used, error);
      if (!error.empty())
        return false;
      const std::string srcReg = source_register_name(tokens[2].text);
      const int srcIndex = std::atoi(srcReg.c_str() + 1);
      // The coordinate source: a texture register, or (the ps.1.4
      // phase-2 form) a temporary holding computed coordinates. The
      // implicit sampler stage is the source register's number either
      // way (water_expensive_scroll's texld r0, r1 samples the stage-1
      // normal map).
      if ((srcReg[0] != 't' && srcReg[0] != 'r') || srcIndex < 0 ||
          srcIndex >= tMax) {
        error = op + " source register: " + line;
        return false;
      }
      const std::string dest = "r" + std::to_string(destIndex);
      if (lower == "texcrd") {
        // The coordinate data copies through the swizzle.
        const std::string value = vec_of(letters, src.comp);
        body.push_back(dest + (letters == "xyzw" ? "" : "." + letters) +
                       " = " + apply_mods(value, OpMods{}) + ";");
        continue;
      }
      // texld: the coordinates are the source's first two components.
      int stage = srcIndex;
      if (!sampler.empty()) {
        stage = std::atoi(sampler.c_str() + 1);
        if (stage < 0 || stage >= 8) {
          error = "sampler stage out of range: " + line;
          return false;
        }
      }
      used.sampler[stage] = true;
      const std::string sample = "texture(s" + std::to_string(stage) +
                                 ", vec2(" + src.comp[0] + ", " + src.comp[1] +
                                 "))";
      body.push_back(dest + (letters == "xyzw" ? "" : "." + letters) + " = " +
                     sample + (letters == "xyzw" ? "" : "." + letters) + ";");
      continue;
    }

    // The arithmetic instructions.
    OpMods mods;
    {
      std::string work = lower;
      parse_op_mods(work, mods);
      lower = work;
    }
    if (lower == "dp3") {
      if (tokens.size() != 4) {
        error = "dp3 arity: " + line;
        return false;
      }
      std::string destReg, destMask;
      if (!dest_register(tokens[1].text, destReg, destMask, error))
        return false;
      const int destIndex = std::atoi(destReg.c_str() + 1);
      if (destIndex < 0 || destIndex >= rMax) {
        error = "temporary register out of range: " + line;
        return false;
      }
      used.r[destIndex] = true;
      const std::string letters = dest_letters(destMask, error, line);
      if (letters.empty())
        return false;
      Source a, b;
      if (!parse_source(tokens[2].text, a, error) ||
          !parse_source(tokens[3].text, b, error))
        return false;
      mark_source(tokens[2].text, used, error);
      mark_source(tokens[3].text, used, error);
      if (!error.empty())
        return false;
      // dp3 dots the rgb of the (modified) sources.
      const std::string av = "vec3(" + a.comp[0] + ", " + a.comp[1] + ", " +
                             a.comp[2] + ")";
      const std::string bv = "vec3(" + b.comp[0] + ", " + b.comp[1] + ", " +
                             b.comp[2] + ")";
      std::string value = "dot(" + av + ", " + bv + ")";
      const std::string dest = "r" + std::to_string(destIndex);
      std::string rhs = apply_mods(value, mods);
      if (letters.size() == 4)
        body.push_back(dest + " = vec4(" + rhs + ");");
      else if (letters.size() == 3)
        body.push_back(dest + ".rgb = vec3(" + rhs + ");");
      else if (letters.size() == 2)
        body.push_back(dest + "." + letters + " = vec2(" + rhs + ");");
      else
        body.push_back(dest + "." + letters + " = " + rhs + ";");
      continue;
    }
    if (lower == "cmp") {
      if (version != Version::Ps14) {
        error = "cmp requires ps.1.4: " + line;
        return false;
      }
      if (tokens.size() != 5) {
        error = "cmp arity: " + line;
        return false;
      }
      std::string destReg, destMask;
      if (!dest_register(tokens[1].text, destReg, destMask, error))
        return false;
      const int destIndex = std::atoi(destReg.c_str() + 1);
      if (destIndex < 0 || destIndex >= rMax) {
        error = "temporary register out of range: " + line;
        return false;
      }
      used.r[destIndex] = true;
      const std::string letters = dest_letters(destMask, error, line);
      if (letters.empty())
        return false;
      Source s0, s1, s2;
      if (!parse_source(tokens[2].text, s0, error) ||
          !parse_source(tokens[3].text, s1, error) ||
          !parse_source(tokens[4].text, s2, error))
        return false;
      for (const auto *tok : {&tokens[2], &tokens[3], &tokens[4]}) {
        mark_source(tok->text, used, error);
        if (!error.empty())
          return false;
      }
      // cmp: per component, src0 >= 0 ? src1 : src2.
      std::string c0[4], c1[4], c2[4];
      for (int i = 0; i < 4; ++i) {
        c0[i] = "(" + s0.comp[i] + " >= 0.0) ? " + s1.comp[i] + " : " +
                s2.comp[i];
      }
      const std::string value = vec_of(letters, c0);
      body.push_back("r" + std::to_string(destIndex) +
                     (letters == "xyzw" ? "" : "." + letters) + " = " +
                     apply_mods(value, mods) + ";");
      continue;
    }
    if (lower == "mov" || lower == "add" || lower == "sub" || lower == "mul" ||
        lower == "mad" || lower == "lrp") {
      const size_t arity = lower == "mad" || lower == "lrp"
                               ? 5
                               : lower == "mov" ? 3 : 4;
      if (tokens.size() != arity) {
        error = op + " arity: " + line;
        return false;
      }
      std::string destReg, destMask;
      if (!dest_register(tokens[1].text, destReg, destMask, error))
        return false;
      const int destIndex = std::atoi(destReg.c_str() + 1);
      if (destIndex < 0 || destIndex >= rMax) {
        error = "temporary register out of range: " + line;
        return false;
      }
      used.r[destIndex] = true;
      const std::string letters = dest_letters(destMask, error, line);
      if (letters.empty())
        return false;
      const int srcCount = arity - 2;
      Source srcs[3];
      for (int s = 0; s < srcCount; ++s) {
        if (!parse_source(tokens[2 + s].text, srcs[s], error))
          return false;
        mark_source(tokens[2 + s].text, used, error);
        if (!error.empty())
          return false;
      }
      // The per-written-component expressions; the vector form keeps
      // the D3D read-before-write semantics for overlapping swizzles.
      std::string comps[4];
      for (size_t i = 0; i < letters.size(); ++i) {
        static const char *all = "xyzw";
        const char *p = std::strchr(all, letters[i]);
        const int slot = int(p - all);
        std::string e;
        if (lower == "mov")
          e = srcs[0].comp[slot];
        else if (lower == "add")
          e = "(" + srcs[0].comp[slot] + " + " + srcs[1].comp[slot] + ")";
        else if (lower == "sub")
          e = "(" + srcs[0].comp[slot] + " - " + srcs[1].comp[slot] + ")";
        else if (lower == "mul")
          e = "(" + srcs[0].comp[slot] + " * " + srcs[1].comp[slot] + ")";
        else if (lower == "mad")
          e = "(" + srcs[0].comp[slot] + " * " + srcs[1].comp[slot] + " + " +
              srcs[2].comp[slot] + ")";
        else  // lrp: src0*src1 + (1-src0)*src2
          e = "mix(" + srcs[2].comp[slot] + ", " + srcs[1].comp[slot] + ", " +
              srcs[0].comp[slot] + ")";
        comps[slot] = e;
      }
      const std::string value = vec_of(letters, comps);
      body.push_back("r" + std::to_string(destIndex) +
                     (letters == "xyzw" ? "" : "." + letters) + " = " +
                     apply_mods(value, mods) + ";");
      continue;
    }

    error = "unsupported pixel shader instruction: " + line;
    return false;
  }

  if (!sawVersion) {
    error = "no ps.1.x version line";
    return false;
  }

  // The header: the varyings, uniforms and samplers the body uses.
  std::string out = "#version 300 es\nprecision highp float;\n";
  for (int i = 0; i < 2; ++i)
    if (used.v[i])
      out += "in vec4 oD" + std::to_string(i) + ";\n";
  for (int i = 0; i < 6; ++i)
    if (used.t[i])
      out += "in vec4 oT" + std::to_string(i) + ";\n";
  if (used.c)
    out += "uniform vec4 pc[8];\n";
  for (int i = 0; i < 8; ++i)
    if (used.sampler[i])
      out += "uniform sampler2D s" + std::to_string(i) + ";\n";
  if (used.bump)
    out += "uniform vec4 u_bump[8];\n";
  if (used.bumpL)
    out += "uniform float u_bumpL[8];\n";
  out += "out vec4 fragColor;\n";
  out += "void main() {\n";
  for (int i = 0; i < 6; ++i)
    if (used.r[i])
      out += "vec4 r" + std::to_string(i) + " = vec4(0.0);\n";
  for (int i = 0; i < 6; ++i) {
    if (!used.t[i])
      continue;
    // ps.1.4 texture registers hold the coordinates; ps.1.1 ones are
    // filled by tex/texcoord.
    out += "vec4 t" + std::to_string(i) + " = " +
           (version == Version::Ps14
                ? "oT" + std::to_string(i)
                : "vec4(0.0)") +
           ";\n";
  }
  for (const std::string &stmt : body)
    out += stmt + "\n";
  out += "fragColor = r0;\n";
  out += "}\n";
  glsl = out;
  return true;
}

}  // namespace pw153::platform
