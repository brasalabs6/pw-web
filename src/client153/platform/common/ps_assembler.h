/*
 * FILE: ps_assembler.h (platform seam)
 *
 * DESCRIPTION: Assembles the original D3D8 pixel-shader assembly
 * (ps.1.1 and ps.1.4 text, the format the shipped shaders.pck stores)
 * into GLSL ES 300 source the WebGL2 device compiles. The translator
 * covers the instruction set the shipped corpus uses (tex, texcoord,
 * texld, texcrd, texkill, texbem, texbeml, mov, add, sub, mul, mad,
 * lrp, dp3, cmp) with the D3D8 register model: t0-t5 texture
 * registers, r0-r5 temporaries, c0-c7 constants, v0-v1 color
 * interpolants, the co-issue (+) pairs and the _sat/_x2/_x4/_x8/_d2
 * instruction and _bias/_bx2/_comp/_x2/_dw/1- source modifiers.
 */

#ifndef PW153_PLATFORM_PS_ASSEMBLER_H
#define PW153_PLATFORM_PS_ASSEMBLER_H

#include <string>
#include <vector>

namespace pw153::platform {

// One def instruction's constant initial value (the D3D def sets the
// constant at shader creation; the device uploads it on first use).
struct PixelShaderDef {
  int reg = 0;
  float value[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

// Assembles a ps.1.1/ps.1.4 shader body into GLSL ES 300 source. The
// source is the exact srcData/srcDataLen pair the D3DXAssembleShader
// contract carries: the parse is bounded by length (the device buffer
// is the package's exact-size read, with no NUL terminator). Returns
// true and fills glsl on success; returns false and fills error with
// the first unparsable construct on failure (never a silent skip).
// defs, when given, receives the def instructions' constant values.
bool AssemblePixelShader(const char *source, size_t length,
                         std::string &glsl, std::string &error,
                         std::vector<PixelShaderDef> *defs = nullptr);

}  // namespace pw153::platform

#endif // PW153_PLATFORM_PS_ASSEMBLER_H
