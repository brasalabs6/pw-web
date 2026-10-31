/*
 * FILE: vs_assembler.h (platform seam)
 *
 * DESCRIPTION: Assembles the original D3D8 vertex-shader assembly
 * (vs.1.1 text, the format the shipped shaders.pck stores) into GLSL ES
 * 300 source the WebGL2 device compiles. The translator covers the
 * instruction set the shipped corpus uses (mov, add, sub, mul, mad, dp3,
 * dp4, min, max, slt, sge, rcp, rsq, exp, lit, dst, m3x2, m3x3, m4x3,
 * m4x4) with the D3D8 register model: v0-v15 inputs, c0-c95 constants
 * with a0 relative addressing, r0-r15 temporaries, oPos/oD0/oD1/oT0-oT3
 * outputs.
 */

#ifndef PW153_PLATFORM_VS_ASSEMBLER_H
#define PW153_PLATFORM_VS_ASSEMBLER_H

#include <string>

namespace pw153::platform {

// Assembles a vs.1.1 shader body into GLSL ES 300 source. The source is
// the exact srcData/srcDataLen pair the D3DXAssembleShader contract
// carries: the parse is bounded by length (the device buffer is the
// package's exact-size read, with no NUL terminator). Returns true and
// fills glsl on success; returns false and fills error with the first
// unparsable construct on failure (never a silent skip).
bool AssembleVertexShader(const char *source, size_t length,
                          std::string &glsl, std::string &error);

}  // namespace pw153::platform

#endif // PW153_PLATFORM_VS_ASSEMBLER_H
