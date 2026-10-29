#include "Windows.h"
#include "dsound.h"
#include "shlwapi.h"
#include "d3dx8.h"
#include <cstddef>
#include <stdexcept>

static_assert(sizeof(WAVEFORMAT) == 14);
static_assert(sizeof(PCMWAVEFORMAT) == 16);
static_assert(offsetof(PCMWAVEFORMAT, wBitsPerSample) == 14);
static_assert(sizeof(WAVEFORMATEX) == 18);

int main() {
  if (stricmp("Login", "lOgIn") != 0) throw std::runtime_error("case-insensitive equality");
  if (stricmp("alpha", "BETA") >= 0 || stricmp("zeta", "Beta") <= 0)
    throw std::runtime_error("case-insensitive ordering");
  if (strcmpi("Align", "aLIGN") != 0 || strcmpi("Alpha", "beta") >= 0)
    throw std::runtime_error("strcmpi case-insensitive comparison");
  if (!PathFileExistsA(".") || PathFileExistsA("") || PathFileExistsA(nullptr))
    throw std::runtime_error("ANSI path existence");
  if (!PathFileExistsA("..\\..") || !PathFileExistsW(L"..\\.."))
    throw std::runtime_error("Windows separators in path existence");
  unsigned char tga[] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 32, 0x28,
                         0, 0, 255, 255, 0, 255, 0, 255};
  D3DXIMAGE_INFO info{};
  if (D3DXGetImageInfoFromFileInMemory(tga, sizeof(tga), &info) != D3D_OK ||
      info.Width != 2 || info.Height != 1 || info.ImageFileFormat != D3DXIFF_TGA ||
      info.Format != D3DFMT_A8R8G8B8)
    throw std::runtime_error("TGA image information");
  if (D3DXGetImageInfoFromFileInMemory(tga, 17, &info) == D3D_OK)
    throw std::runtime_error("truncated image information");
  if (D3DXGetImageInfoFromFileInMemory(tga, sizeof(tga) - 1, &info) == D3D_OK)
    throw std::runtime_error("truncated TGA pixels");
  unsigned char rle[] = {0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 32, 0x28,
                         0x81, 0, 0, 255, 255};
  if (D3DXGetImageInfoFromFileInMemory(rle, sizeof(rle), &info) != D3D_OK)
    throw std::runtime_error("RLE TGA image information");
  rle[18] = 0x82;
  if (D3DXGetImageInfoFromFileInMemory(rle, sizeof(rle), &info) == D3D_OK)
    throw std::runtime_error("TGA packet exceeds dimensions");
  unsigned char dds[144]{};
  auto put32 = [&](int offset, unsigned value) {
    for (int i = 0; i < 4; ++i) dds[offset + i] = (value >> (i * 8)) & 255;
  };
  std::memcpy(dds, "DDS ", 4); put32(4, 124); put32(12, 4); put32(16, 4);
  put32(76, 32); put32(80, 4); put32(84, D3DFMT_DXT3);
  if (D3DXGetImageInfoFromFileInMemory(dds, sizeof(dds), &info) != D3D_OK ||
      info.Width != 4 || info.Height != 4 || info.Format != D3DFMT_DXT3 || info.ImageFileFormat != D3DXIFF_DDS)
    throw std::runtime_error("DXT3 DDS image information");
}
