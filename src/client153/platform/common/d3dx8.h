/*
 * FILE: d3dx8.h (platform seam)
 *
 * DESCRIPTION: D3DX math aliases - the ported A3D math carries the same
 * layouts the original D3DX headers wrapped. See d3d8.h. The D3D8 vertex
 * shader declarator tokens, texture stage states and render states the
 * client sources name are the documented d3d8types.h values.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */
#ifndef PW153_PLATFORM_D3DX8_H
#define PW153_PLATFORM_D3DX8_H
#include "d3d8.h"
#include "vs_assembler.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

//	D3D8 vertex shader declarator tokens (documented d3d8types.h values).
#define D3DVSD_TOKENTYPESHIFT 29
#define D3DVSD_DATATYPESHIFT 16
#define D3DVSD_TOKEN_STREAM 1
#define D3DVSD_TOKEN_STREAMDATA 2
#define D3DVSD_MAKETOKENTYPE(TokenType) \
  (((unsigned)(TokenType) << D3DVSD_TOKENTYPESHIFT) & \
   (0x7u << D3DVSD_TOKENTYPESHIFT))
#define D3DVSD_STREAM(StreamNumber) \
  (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM) | (StreamNumber))
#define D3DVSD_REG(VertexRegister, Type) \
  (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) | \
   ((Type) << D3DVSD_DATATYPESHIFT) | (VertexRegister))
#define D3DVSD_END() 0xFFFFFFFF
#define D3DVSDT_FLOAT1 0x00
#define D3DVSDT_FLOAT2 0x01
#define D3DVSDT_FLOAT3 0x02
#define D3DVSDT_FLOAT4 0x03
#define D3DVSDT_D3DCOLOR 0x04
#define D3DVSDT_UBYTE4 0x05
#define D3DVSDE_POSITION 0
#define D3DVSDE_BLENDWEIGHT 1
#define D3DVSDE_BLENDINDICES 2
#define D3DVSDE_NORMAL 3
#define D3DVSDE_PSIZE 4
#define D3DVSDE_DIFFUSE 5
#define D3DVSDE_SPECULAR 6
#define D3DVSDE_TEXCOORD0 7
#define D3DVSDE_TEXCOORD1 8
#define D3DVSDE_TEXCOORD2 9
#define D3DVSDE_TEXCOORD3 10
#define D3DVSDE_TEXCOORD4 11
#define D3DVSDE_TEXCOORD5 12
#define D3DVSDE_TEXCOORD6 13
#define D3DVSDE_TEXCOORD7 14

//	The image header record (the documented D3DXIMAGE_INFO layout; the
//	format field carries the documented D3DFORMAT enum now that the
//	graphics cycle defines it, the resource/file-format fields stay
//	unsigned ints until their enums land).
typedef struct _D3DXIMAGE_INFO {
  UINT Width;
  UINT Height;
  UINT Depth;
  UINT MipLevels;
  D3DFORMAT Format;
  UINT ResourceType;
  UINT ImageFileFormat;
} D3DXIMAGE_INFO;

#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL 0x8876086C
#endif

//	The image file formats (the documented D3DXIMAGE_FILEFORMAT values).
#define D3DXIFF_BMP 0
#define D3DXIFF_JPG 1
#define D3DXIFF_DDS 2
#define D3DXIFF_DIB 3
#define D3DXIFF_HDR 4
#define D3DXIFF_PFM 5
#define D3DXIFF_PNG 6
#define D3DXIFF_PPM 7
#define D3DXIFF_TGA 8

//	The texture save: the render-target textures do not exist on the
//	browser target yet (the WebGL backend owns the real surface when it
//	registers); the save fails explicitly and the caller's error path
//	handles it.
inline HRESULT D3DXSaveTextureToFileA(const char *file, DWORD format,
                                     IDirect3DBaseTexture8 *texture,
                                     const PALETTEENTRY *palette) {
  (void)file;
  (void)format;
  (void)texture;
  (void)palette;
  return D3DERR_INVALIDCALL;
}

//	The surface save: the same explicit failure (no real surfaces on
//	the browser target yet).
inline HRESULT D3DXSaveSurfaceToFileA(const char *file, DWORD format,
                                      IDirect3DSurface8 *surface,
                                      const PALETTEENTRY *palette,
                                      const void *reserved = nullptr) {
  (void)file;
  (void)format;
  (void)surface;
  (void)palette;
  (void)reserved;
  return D3DERR_INVALIDCALL;
}
inline HRESULT D3DXSaveSurfaceToFileW(const wchar_t *file, DWORD format,
                                      IDirect3DSurface8 *surface,
                                      const PALETTEENTRY *palette,
                                      const void *reserved = nullptr) {
  (void)file;
  (void)format;
  (void)surface;
  (void)palette;
  (void)reserved;
  return D3DERR_INVALIDCALL;
}
inline HRESULT D3DXSaveSurfaceToFile(const wchar_t *file, DWORD format,
                                     IDirect3DSurface8 *surface,
                                     const PALETTEENTRY *palette,
                                     const void *reserved = nullptr) {
  return D3DXSaveSurfaceToFileW(file, format, surface, palette, reserved);
}

//	The surface copy/filter: the same explicit failure (no real
//	surfaces on the browser target yet).
inline HRESULT D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *destination,
                                          const PALETTEENTRY *destinationPalette,
                                          const RECT *destinationRect,
                                          IDirect3DSurface8 *source,
                                          const PALETTEENTRY *sourcePalette,
                                          const RECT *sourceRect,
                                          DWORD filter, DWORD colorKey) {
  (void)destination;
  (void)destinationPalette;
  (void)destinationRect;
  (void)source;
  (void)sourcePalette;
  (void)sourceRect;
  (void)filter;
  (void)colorKey;
  return D3DERR_INVALIDCALL;
}

inline HRESULT D3DXGetImageInfoFromFileInMemory(const void *, UINT, D3DXIMAGE_INFO *);

// Decode the true-colour TGA images used by the original UI into a lockable surface.
inline HRESULT D3DXLoadSurfaceFromFileInMemory(
    IDirect3DSurface8 *destinationSurface, const PALETTEENTRY *destinationPalette,
    const RECT *destinationRect, const void *srcData, UINT srcDataSize,
    const RECT *srcRect, DWORD filter, D3DCOLOR colorKey,
    D3DXIMAGE_INFO *srcInfo) {
  (void)filter;
  D3DXIMAGE_INFO image{};
  if (!destinationSurface || destinationPalette ||
      D3DXGetImageInfoFromFileInMemory(srcData, srcDataSize, &image) != D3D_OK ||
      (image.ImageFileFormat != D3DXIFF_TGA && image.ImageFileFormat != D3DXIFF_BMP &&
       !(image.ImageFileFormat == D3DXIFF_DDS &&
         (image.Format == D3DFMT_DXT1 || image.Format == D3DFMT_DXT3 || image.Format == D3DFMT_DXT5 ||
          image.Format == D3DFMT_A8R8G8B8)))) return D3DERR_INVALIDCALL;
  D3DSURFACE_DESC desc{};
  if (destinationSurface->GetDesc(&desc) != D3D_OK) return D3DERR_INVALIDCALL;
  RECT source{0, 0, static_cast<LONG>(image.Width), static_cast<LONG>(image.Height)};
  RECT target{0, 0, static_cast<LONG>(desc.Width), static_cast<LONG>(desc.Height)};
  if (srcRect) source = *srcRect;
  if (destinationRect) target = *destinationRect;
  if (source.left < 0 || source.top < 0 || source.right <= source.left || source.bottom <= source.top ||
      source.right > static_cast<LONG>(image.Width) || source.bottom > static_cast<LONG>(image.Height) ||
      target.left < 0 || target.top < 0 || target.right <= target.left || target.bottom <= target.top ||
      target.right > static_cast<LONG>(desc.Width) || target.bottom > static_cast<LONG>(desc.Height) ||
      source.right - source.left != target.right - target.left ||
      source.bottom - source.top != target.bottom - target.top) return D3DERR_INVALIDCALL;
  if (desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8 &&
      desc.Format != D3DFMT_A4R4G4B4 && desc.Format != D3DFMT_A1R5G5B5 &&
      desc.Format != D3DFMT_R5G6B5) return D3DERR_INVALIDCALL;
  const auto *data = static_cast<const unsigned char *>(srcData);
  const size_t pixels = size_t(image.Width) * image.Height;
  std::vector<unsigned char> bgra(pixels * 4);
  if (image.ImageFileFormat == D3DXIFF_BMP) {
    //	24/32-bit BI_RGB rows, bottom-up (negative height = top-down),
    //	each row padded to 4 bytes. The colour key matches the TGA path.
    const unsigned bits = image.Format == D3DFMT_A8R8G8B8 ? 32 : 24;
    const size_t rowBytes = (size_t(image.Width) * bits + 31) / 32 * 4;
    const unsigned pixelOffset = unsigned(data[10]) | unsigned(data[11]) << 8 |
                                  unsigned(data[12]) << 16 | unsigned(data[13]) << 24;
    const int storedHeight = static_cast<int>(unsigned(data[22]) | unsigned(data[23]) << 8 |
                                              unsigned(data[24]) << 16 | unsigned(data[25]) << 24);
    const bool topDown = storedHeight < 0;
    for (unsigned y = 0; y < image.Height; ++y) {
      const size_t sourceRow = topDown ? y : image.Height - 1 - y;
      const auto *row = data + pixelOffset + sourceRow * rowBytes;
      for (unsigned x = 0; x < image.Width; ++x) {
        auto *out = bgra.data() + (size_t(y) * image.Width + x) * 4;
        out[0] = row[x * (bits / 8)];
        out[1] = row[x * (bits / 8) + 1];
        out[2] = row[x * (bits / 8) + 2];
        out[3] = bits == 32 ? row[x * 4 + 3] : 255;
        if (colorKey && (0xff000000u | unsigned(out[2]) << 16 | unsigned(out[1]) << 8 | out[0]) ==
                            (0xff000000u | (colorKey & 0xffffffu))) out[3] = 0;
      }
    }
  } else if (image.ImageFileFormat == D3DXIFF_DDS && image.Format == D3DFMT_A8R8G8B8) {
    //	Uncompressed 32-bit BGRA surface; only mip level 0 (the loader fills
    //	the top level of a one-level texture).
    const auto *surface = data + 128;
    for (size_t i = 0; i < pixels; ++i) {
      auto *out = bgra.data() + i * 4;
      out[0] = surface[i * 4]; out[1] = surface[i * 4 + 1];
      out[2] = surface[i * 4 + 2]; out[3] = surface[i * 4 + 3];
      if (colorKey && (0xff000000u | unsigned(out[2]) << 16 | unsigned(out[1]) << 8 | out[0]) ==
                          (0xff000000u | (colorKey & 0xffffffu))) out[3] = 0;
    }
  } else if (image.ImageFileFormat == D3DXIFF_DDS) {
    //	Compressed BC1/BC2/BC3 (DXT1/DXT3/DXT5) blocks. DXT1 packs colour
    //	words and 2-bit indices into 8 bytes with 1-bit alpha (index 3 is
    //	transparent when c0 <= c1); DXT3 adds explicit 4-bit alpha; DXT5
    //	interpolates an 8-entry alpha palette from two endpoints.
    const bool isDXT1 = image.Format == D3DFMT_DXT1;
    const bool isDXT5 = image.Format == D3DFMT_DXT5;
    const unsigned blockSize = isDXT1 ? 8 : 16;
    const unsigned blocksX = (image.Width + 3) / 4, blocksY = (image.Height + 3) / 4;
    for (unsigned by = 0; by < blocksY; ++by) for (unsigned bx = 0; bx < blocksX; ++bx) {
      const unsigned char *block = data + 128 + (size_t(by) * blocksX + bx) * blockSize;
      const unsigned char *colorBlock = isDXT1 ? block : block + 8;
      unsigned c0 = unsigned(colorBlock[0]) | unsigned(colorBlock[1]) << 8;
      unsigned c1 = unsigned(colorBlock[2]) | unsigned(colorBlock[3]) << 8;
      unsigned colors[4][3]{};
      for (unsigned i = 0; i < 2; ++i) {
        unsigned c = i ? c1 : c0;
        unsigned r = (c >> 11) & 31, g = (c >> 5) & 63, b = c & 31;
        colors[i][0] = (b << 3) | (b >> 2);
        colors[i][1] = (g << 2) | (g >> 4);
        colors[i][2] = (r << 3) | (r >> 2);
      }
      const bool fourColor = !isDXT1 || c0 > c1;
      for (unsigned k = 0; k < 3; ++k) {
        colors[2][k] = (2 * colors[0][k] + colors[1][k]) / 3;
        colors[3][k] = fourColor ? (colors[0][k] + 2 * colors[1][k]) / 3 : 0;
      }
      unsigned indices = unsigned(colorBlock[4]) | unsigned(colorBlock[5]) << 8 |
                         unsigned(colorBlock[6]) << 16 | unsigned(colorBlock[7]) << 24;
      for (unsigned y = 0; y < 4; ++y) for (unsigned x = 0; x < 4; ++x) {
        const unsigned pixel = y * 4 + x, px = bx * 4 + x, py = by * 4 + y;
        if (px >= image.Width || py >= image.Height) continue;
        auto *out = bgra.data() + (size_t(py) * image.Width + px) * 4;
        const unsigned index = (indices >> (pixel * 2)) & 3;
        const unsigned *rgb = colors[index];
        out[0] = rgb[0]; out[1] = rgb[1]; out[2] = rgb[2];
        if (isDXT1) {
          out[3] = fourColor || index != 3 ? 255 : 0;
        } else if (isDXT5) {
          //	Alpha palette: two endpoints + 48 bits of 3-bit indices
          //	(block[2..7], little-endian bit stream).
          const unsigned a0 = block[0], a1 = block[1];
          unsigned alpha[8] = {a0, a1, 0, 0, 0, 0, 0, 0};
          if (a0 > a1) {
            for (unsigned i = 0; i < 6; ++i)
              alpha[2 + i] = ((6 - i) * a0 + (1 + i) * a1) / 7;
          } else {
            for (unsigned i = 0; i < 4; ++i)
              alpha[2 + i] = ((4 - i) * a0 + (1 + i) * a1) / 5;
            alpha[6] = 0;
            alpha[7] = 255;
          }
          const unsigned bit = pixel * 3;
          const unsigned value = (unsigned(block[2 + bit / 8]) >> (bit % 8)) |
                                 (bit % 8 > 5 ? unsigned(block[3 + bit / 8]) << (8 - bit % 8) : 0);
          out[3] = alpha[value & 7];
        } else {
          out[3] = ((block[pixel / 2] >> ((pixel & 1) * 4)) & 15) * 17;
        }
      }
    }
  } else {
  const unsigned bytes = data[16] / 8;
  size_t pos = 18u + data[0], index = 0;
  while (index < pixels) {
    unsigned packet = data[2] == 10 ? data[pos++] : 0;
    size_t count = data[2] == 10 ? (packet & 127u) + 1u : 1u;
    for (size_t i = 0; i < count; ++i) {
      const size_t pixelPos = pos + ((packet & 128u) ? 0 : i * bytes);
      const size_t x = index % image.Width, y = index / image.Width;
      const size_t dx = (data[17] & 0x10) ? image.Width - 1 - x : x;
      const size_t dy = (data[17] & 0x20) ? y : image.Height - 1 - y;
      auto *out = bgra.data() + (dy * image.Width + dx) * 4;
      out[0] = data[pixelPos]; out[1] = data[pixelPos + 1]; out[2] = data[pixelPos + 2];
      out[3] = bytes == 4 ? data[pixelPos + 3] : 255;
      if (colorKey && (0xff000000u | unsigned(out[2]) << 16 | unsigned(out[1]) << 8 | out[0]) ==
                          (0xff000000u | (colorKey & 0xffffffu))) out[3] = 0;
      ++index;
    }
    pos += ((packet & 128u) ? 1 : count) * bytes;
  }
  }
  D3DLOCKED_RECT lock{};
  if (destinationSurface->LockRect(&lock, &target, 0) != D3D_OK) return D3DERR_INVALIDCALL;
  for (LONG y = 0; y < target.bottom - target.top; ++y) {
    auto *row = static_cast<unsigned char *>(lock.pBits) + size_t(y) * lock.Pitch;
    for (LONG x = 0; x < target.right - target.left; ++x) {
      const auto *p = bgra.data() + (size_t(source.top + y) * image.Width + source.left + x) * 4;
      if (desc.Format == D3DFMT_A8R8G8B8 || desc.Format == D3DFMT_X8R8G8B8) {
        std::memcpy(row + x * 4, p, 4);
        if (desc.Format == D3DFMT_X8R8G8B8) row[x * 4 + 3] = 255;
      } else {
        unsigned value = desc.Format == D3DFMT_A4R4G4B4 ?
            ((p[3] >> 4) << 12) | ((p[2] >> 4) << 8) | ((p[1] >> 4) << 4) | (p[0] >> 4) :
            desc.Format == D3DFMT_A1R5G5B5 ?
            ((p[3] >= 128) << 15) | ((p[2] >> 3) << 10) | ((p[1] >> 3) << 5) | (p[0] >> 3) :
            ((p[2] >> 3) << 11) | ((p[1] >> 2) << 5) | (p[0] >> 3);
        row[x * 2] = value & 255; row[x * 2 + 1] = value >> 8;
      }
    }
  }
  if (destinationSurface->UnlockRect() != D3D_OK) return D3DERR_INVALIDCALL;
  if (srcInfo) *srcInfo = image;
  return D3D_OK;
}

//	The D3DX default sentinel (the documented d3dx8 value).
#define D3DX_DEFAULT 0xFFFFFFFF

// Create one-level textures for formats the surface decoder supports.
inline HRESULT D3DXCreateTextureFromFileInMemoryEx(
    IDirect3DDevice8 *device, const void *srcData, UINT srcDataSize,
    UINT width, UINT height, UINT mipLevels, DWORD usage, D3DFORMAT format,
    D3DPOOL pool, DWORD filter, DWORD mipFilter, D3DCOLOR colorKey,
    D3DXIMAGE_INFO *srcInfo, PALETTEENTRY *palette,
    IDirect3DTexture8 **texture) {
  (void)filter;
  (void)mipFilter;
  if (!texture) return D3DERR_INVALIDCALL;
  *texture = nullptr;
  if (!device || palette) return D3DERR_INVALIDCALL;
  D3DXIMAGE_INFO image{};
  if (D3DXGetImageInfoFromFileInMemory(srcData, srcDataSize, &image) != D3D_OK) return D3DERR_INVALIDCALL;
  if (width == D3DX_DEFAULT) width = image.Width;
  if (height == D3DX_DEFAULT) height = image.Height;
  if (width != image.Width || height != image.Height) return D3DERR_INVALIDCALL;
  //	Mip levels: D3DX_DEFAULT takes the file's chain; explicit counts are
	//	honoured up to what the image carries (the adapter generates the
	//	GPU chain from the decoded level 0).
  UINT levels = mipLevels == D3DX_DEFAULT ? (image.MipLevels ? image.MipLevels : 1) : mipLevels;
  if (!levels || levels > (image.MipLevels ? image.MipLevels : 1)) return D3DERR_INVALIDCALL;
  if (format == D3DFMT_UNKNOWN) format = D3DFMT_A8R8G8B8;
  IDirect3DTexture8 *result = nullptr;
  if (device->CreateTexture(width, height, levels, usage, format, pool, &result) != D3D_OK || !result)
    return D3DERR_INVALIDCALL;
  IDirect3DSurface8 *surface = nullptr;
  if (result->GetSurfaceLevel(0, &surface) != D3D_OK || !surface) {
    result->Release(); return D3DERR_INVALIDCALL;
  }
  HRESULT status = D3DXLoadSurfaceFromFileInMemory(surface, nullptr, nullptr, srcData, srcDataSize,
                                                    nullptr, filter, colorKey, nullptr);
  surface->Release();
  if (status != D3D_OK) { result->Release(); return status; }
  if (srcInfo) *srcInfo = image;
  *texture = result;
  return D3D_OK;
}

//	The error string: the documented D3DXGetErrorStringA contract is a
//	text description of the HRESULT; the port renders the code itself
//	(the real D3D error table is not part of the seam).
inline HRESULT D3DXGetErrorStringA(HRESULT hr, char *buffer, UINT size) {
  if (buffer && size)
    std::snprintf(buffer, size, "D3D error 0x%08x",
                  static_cast<unsigned int>(hr));
  return D3D_OK;
}

//	The shader byte buffer (the documented D3DXCreateBuffer contract: a
//	ref-counted memory block). Pure data allocation that works on any
//	target, unlike the device shader calls which fail explicitly.
struct D3DXShaderBuffer : ID3DXBuffer {
  D3DXShaderBuffer() : data_(nullptr), size_(0), refs_(1) {}
  ~D3DXShaderBuffer() { std::free(data_); }
  HRESULT QueryInterface(const void *riid, void **ppvObject) override {
    (void)riid;
    if (!ppvObject)
      return E_POINTER;
    *ppvObject = this;
    AddRef();
    return D3D_OK;
  }
  ULONG AddRef() override { return ++refs_; }
  ULONG Release() override {
    const ULONG refs = --refs_;
    if (!refs)
      delete this;
    return refs;
  }
  void *GetBufferPointer() override { return data_; }
  DWORD GetBufferSize() override { return size_; }
  void *data_;
  DWORD size_;
  ULONG refs_;
};

inline HRESULT D3DXCreateBuffer(UINT numBytes, ID3DXBuffer **ppBuffer) {
  if (!ppBuffer)
    return D3DERR_INVALIDCALL;
  *ppBuffer = nullptr;
  D3DXShaderBuffer *buffer = new D3DXShaderBuffer;
  if (numBytes) {
    buffer->data_ = std::malloc(numBytes);
    if (!buffer->data_) {
      delete buffer;
      return E_OUTOFMEMORY;
    }
    std::memset(buffer->data_, 0, numBytes);
  }
  buffer->size_ = numBytes;
  *ppBuffer = buffer;
  return D3D_OK;
}

//	The shader assembler: real D3D shader assembly does not exist on the
//	browser target (the shader programs land with the graphics cycle's
//	translation strategy); the call fails explicitly and the loader's
//	error path handles it (the documented DX8.1 signature).
inline HRESULT D3DXAssembleShader(const void *srcData, UINT srcDataLen,
                                  DWORD flags, ID3DXBuffer **constants,
                                  ID3DXBuffer **compiledShader,
                                  ID3DXBuffer **compilationErrors) {
  (void)flags;
  if (constants)
    *constants = nullptr;
  if (compilationErrors)
    *compilationErrors = nullptr;
  if (compiledShader)
    *compiledShader = nullptr;
  // The shipped shaders are vs.1.1 assembly; the port assembles them
  // into GLSL ES 300 source (the WebGL2 device compiles it). The buffer
  // carries the GLSL text where the original carried the compiled
  // tokens — CreateVertexShader consumes it through the same contract.
  if (!srcData || !srcDataLen || !compiledShader)
    return D3DERR_INVALIDCALL;
  std::string glsl, error;
  if (!pw153::platform::AssembleVertexShader(
          static_cast<const char *>(srcData), glsl, error))
    return D3DERR_INVALIDCALL;
  auto *buffer = new D3DXShaderBuffer();
  buffer->data_ = std::malloc(glsl.size() + 1);
  if (!buffer->data_) {
    buffer->Release();
    return E_OUTOFMEMORY;
  }
  std::memcpy(buffer->data_, glsl.c_str(), glsl.size() + 1);
  buffer->size_ = (DWORD)glsl.size();
  *compiledShader = buffer;
  return D3D_OK;
}

//	The D3DX texture filters (the documented d3dx8tex values).
#define D3DX_FILTER_NONE (1 << 0)
#define D3DX_FILTER_POINT (2 << 0)
#define D3DX_FILTER_LINEAR (3 << 0)
#define D3DX_FILTER_TRIANGLE (4 << 0)
#define D3DX_FILTER_BOX (5 << 0)

//	The image header read: the BBS download path has no files on the
//	browser target yet (the download seam fails explicitly), so the
//	read fails with D3DERR_INVALIDCALL and the caller's documented
//	fallback (the placeholder size) applies. The real header parse
//	lands with the graphics cycle when the download path delivers
//	real files.
inline HRESULT D3DXGetImageInfoFromFileA(const char *file,
                                         D3DXIMAGE_INFO *info) {
  (void)file;
  if (info) {
    info->Width = 0;
    info->Height = 0;
    info->Depth = 0;
    info->MipLevels = 0;
    info->Format = D3DFMT_UNKNOWN;
    info->ResourceType = 0;
    info->ImageFileFormat = 0;
  }
  return D3DERR_INVALIDCALL;
}

//	The image header read from memory: the same explicit failure (the
//	real image decode lands with the graphics cycle).
inline HRESULT D3DXGetImageInfoFromFileInMemory(const void *srcData,
                                                UINT srcDataSize,
                                                D3DXIMAGE_INFO *info) {
  if (info) {
    info->Width = 0;
    info->Height = 0;
    info->Depth = 0;
    info->MipLevels = 0;
    info->Format = D3DFMT_UNKNOWN;
    info->ResourceType = 0;
    info->ImageFileFormat = 0;
  }
  if (!srcData || !info || srcDataSize < 18) return D3DERR_INVALIDCALL;
  const auto *bytes = static_cast<const unsigned char *>(srcData);
  auto read32 = [&](size_t offset) -> unsigned {
    return unsigned(bytes[offset]) | (unsigned(bytes[offset + 1]) << 8) |
           (unsigned(bytes[offset + 2]) << 16) | (unsigned(bytes[offset + 3]) << 24);
  };
  if (srcDataSize >= 128 && std::memcmp(bytes, "DDS ", 4) == 0) {
    const unsigned width = read32(16), height = read32(12), fourcc = read32(84);
    const unsigned pfFlags = read32(80), bitCount = read32(88);
    const bool compressed = pfFlags & 0x4;  //	DDPF_FOURCC
    const bool rgb = pfFlags & 0x40;       //	DDPF_RGB
    if (read32(4) != 124 || read32(76) != 32 || !width || !height || width > 8192 || height > 8192 ||
        read32(112) || read32(24) > 1) return D3DERR_INVALIDCALL;
    if (compressed && fourcc != D3DFMT_DXT1 && fourcc != D3DFMT_DXT3 && fourcc != D3DFMT_DXT5)
      return D3DERR_INVALIDCALL;
    //	Uncompressed surfaces: only the 32-bit BGRA layout the original UI
    //	assets use (e.g. Listbox\列表框_反显.dds); reject anything else
    //	instead of guessing a layout.
    if (!compressed && (!rgb || bitCount != 32 || read32(92) != 0xFF0000u || read32(96) != 0xFF00u ||
                        read32(100) != 0xFFu)) return D3DERR_INVALIDCALL;
    unsigned levels = read32(28); if (!levels) levels = 1;
    unsigned maxLevels = 1;
    for (unsigned extent = width > height ? width : height; extent > 1; extent >>= 1) ++maxLevels;
    if (levels > maxLevels) return D3DERR_INVALIDCALL;
    size_t pos = 128;
    unsigned w = width, h = height;
    for (unsigned i = 0; i < levels; ++i) {
      const size_t levelBytes = compressed
          ? size_t((w + 3) / 4) * ((h + 3) / 4) * (fourcc == D3DFMT_DXT1 ? 8 : 16)
          : size_t(w) * h * 4;
      if (levelBytes > srcDataSize - pos) return D3DERR_INVALIDCALL;
      pos += levelBytes; w = w > 1 ? w / 2 : 1; h = h > 1 ? h / 2 : 1;
    }
    info->Width = width; info->Height = height; info->Depth = 1; info->MipLevels = levels;
    info->Format = compressed ? static_cast<D3DFORMAT>(fourcc) : D3DFMT_A8R8G8B8;
    info->ResourceType = D3DRTYPE_TEXTURE;
    info->ImageFileFormat = D3DXIFF_DDS;
    return D3D_OK;
  }
  //	Windows bitmap: the original UI stores dialog colour-key and face
  //	sprites as 24/32-bit BI_RGB .bmp files (e.g. Window\对话框_透明色.bmp).
  if (srcDataSize >= 54 && bytes[0] == 'B' && bytes[1] == 'M') {
    const unsigned headerSize = read32(14);
    const int width = static_cast<int>(read32(18));
    const int height = static_cast<int>(read32(22));
    const unsigned bitCount = read32(28) & 0xFFFF;
    const unsigned compression = read32(30);
    const unsigned pixelOffset = read32(10);
    if (headerSize < 40 || compression != 0 || (bitCount != 24 && bitCount != 32) ||
        width <= 0 || height == 0 || width > 8192 || (height < -8192 || height > 8192) ||
        (height < 0 && bitCount != 32)) return D3DERR_INVALIDCALL;
    const size_t rowBytes = (size_t(width) * bitCount + 31) / 32 * 4;
    if (pixelOffset > srcDataSize || rowBytes * size_t(height < 0 ? -height : height) > srcDataSize - pixelOffset)
      return D3DERR_INVALIDCALL;
    info->Width = width; info->Height = height < 0 ? -height : height;
    info->Depth = 1; info->MipLevels = 1;
    info->Format = bitCount == 32 ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
    info->ResourceType = D3DRTYPE_TEXTURE;
    info->ImageFileFormat = D3DXIFF_BMP;
    return D3D_OK;
  }
  const unsigned type = bytes[2], bits = bytes[16];
  if (bytes[1] != 0 || (type != 2 && type != 10) ||
      (bits != 24 && bits != 32)) return D3DERR_INVALIDCALL;
  const unsigned width = bytes[12] | (unsigned(bytes[13]) << 8);
  const unsigned height = bytes[14] | (unsigned(bytes[15]) << 8);
  if (!width || !height || width > 8192 || height > 8192) return D3DERR_INVALIDCALL;
  const size_t offset = 18u + bytes[0];
  if (offset > srcDataSize) return D3DERR_INVALIDCALL;
  size_t remaining = size_t(width) * height;
  size_t cursor = offset;
  if (type == 2) {
    if (remaining > (srcDataSize - cursor) / (bits / 8)) return D3DERR_INVALIDCALL;
  } else {
    while (remaining) {
      if (cursor >= srcDataSize) return D3DERR_INVALIDCALL;
      const unsigned packet = bytes[cursor++];
      const size_t count = (packet & 127u) + 1u;
      if (count > remaining) return D3DERR_INVALIDCALL;
      const size_t payload = (packet & 128u ? 1u : count) * (bits / 8);
      if (payload > srcDataSize - cursor) return D3DERR_INVALIDCALL;
      cursor += payload;
      remaining -= count;
    }
  }
  info->Width = width;
  info->Height = height;
  info->Depth = 1;
  info->MipLevels = 1;
  info->Format = bits == 32 ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
  info->ResourceType = D3DRTYPE_TEXTURE;
  info->ImageFileFormat = D3DXIFF_TGA;
  return D3D_OK;
}

//	D3D8 texture stage states (documented d3d8types.h values).
#define D3DTSS_TCI_CAMERASPACEPOSITION 0x20000
#define D3DTSS_MIPMAPLODBIAS 19

//	D3D8 fixed-function states the grass render reaches (documented
//	d3d8types.h values).
#define D3DFVF_NORMAL 0x0010
#define D3DRS_CLIPPLANEENABLE 152
#define D3DCLIPPLANE0 (1 << 0)

inline D3DXMATRIX *D3DXMatrixOrthoOffCenterLH(D3DXMATRIX *out, float left,
                                              float right, float bottom,
                                              float top, float nearZ, float farZ) {
  *out = D3DXMATRIX(2.f / (right - left), 0.f, 0.f, 0.f,
                    0.f, 2.f / (top - bottom), 0.f, 0.f,
                    0.f, 0.f, 1.f / (farZ - nearZ), 0.f,
                    (left + right) / (left - right),
                    (top + bottom) / (bottom - top),
                    nearZ / (nearZ - farZ), 1.f);
  return out;
}

//	D3DXDeclaratorFromFVF: builds the vertex shader declarator from an
//	FVF code per the documented DX8 D3DX contract
//	(https://learn.microsoft.com/en-us/troubleshoot/win32/directx/flexible-vertex-format
//	and the DX8 SDK D3DX reference): a D3DVSD_STREAM(0) token first, then
//	one D3DVSD_REG per FVF component in vertex-data order (position,
//	blend weights, blend indices, normal, point size, diffuse, specular,
//	texture coordinates) bound to the fixed-function input registers
//	(D3DVSDE_*), terminated by D3DVSD_END(). The texture-size bits follow
//	the documented D3DFVF_TEXCOORDSIZEn encoding (2 bits per coordinate
//	set at bit 16; 0=FLOAT2, 1=FLOAT3, 2=FLOAT4, 3=FLOAT1). FVF codes
//	without a position component fail explicitly (E_FAIL); the remainder
//	of the declarator array is zero-filled.
inline HRESULT D3DXDeclaratorFromFVF(DWORD FVF, DWORD *pDeclarator) {
  if (!pDeclarator)
    return E_INVALIDARG;
  const int texCount = (int)((FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT);
  // Direct3D 8 defines at most eight texture-coordinate sets. Reject the
  // other encodings before calculating shifts or writing any output tokens.
  if (texCount > 8)
    return E_FAIL;
  int i = 0;
  pDeclarator[i++] = D3DVSD_STREAM(0);
  //	The position field is bits 1-3 and the encodings are alternatives
  //	(XYZ, XYZRHW, XYZB1..XYZB5 - XYZBn does NOT include the XYZ bit).
  const DWORD positionBits = FVF & D3DFVF_POSITION_MASK;
  if (positionBits == D3DFVF_XYZRHW) {
    pDeclarator[i++] = D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT4);
  } else if (positionBits != 0) {
    pDeclarator[i++] = D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3);
    //	D3DFVF_XYZB1..XYZB5 encode n blend weights; with a LASTBETA flag
    //	the last DWORD is the blend indices (UBYTE4/D3DCOLOR) instead, and
    //	plain XYZB5's fifth DWORD is UBYTE4 blend indices (the documented
    //	DX8 FVF semantics).
    const int numWeights =
        positionBits >= D3DFVF_XYZB1
            ? (int)((positionBits - D3DFVF_XYZB1) / 2) + 1
            : 0;
    if (numWeights > 0) {
      if (FVF & D3DFVF_LASTBETA_UBYTE4) {
        if (numWeights >= 2)
          pDeclarator[i++] = D3DVSD_REG(D3DVSDE_BLENDWEIGHT,
                                        D3DVSDT_FLOAT1 + (numWeights - 2));
        pDeclarator[i++] = D3DVSD_REG(D3DVSDE_BLENDINDICES, D3DVSDT_UBYTE4);
      } else if (FVF & D3DFVF_LASTBETA_D3DCOLOR) {
        if (numWeights >= 2)
          pDeclarator[i++] = D3DVSD_REG(D3DVSDE_BLENDWEIGHT,
                                        D3DVSDT_FLOAT1 + (numWeights - 2));
        pDeclarator[i++] = D3DVSD_REG(D3DVSDE_BLENDINDICES, D3DVSDT_D3DCOLOR);
      } else if (numWeights == 5) {
        pDeclarator[i++] = D3DVSD_REG(D3DVSDE_BLENDWEIGHT, D3DVSDT_FLOAT4);
        pDeclarator[i++] = D3DVSD_REG(D3DVSDE_BLENDINDICES, D3DVSDT_UBYTE4);
      } else {
        pDeclarator[i++] = D3DVSD_REG(D3DVSDE_BLENDWEIGHT,
                                      D3DVSDT_FLOAT1 + (numWeights - 1));
      }
    }
  } else {
    return E_FAIL;
  }
  if (FVF & D3DFVF_NORMAL)
    pDeclarator[i++] = D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3);
  if (FVF & D3DFVF_PSIZE)
    pDeclarator[i++] = D3DVSD_REG(D3DVSDE_PSIZE, D3DVSDT_FLOAT1);
  if (FVF & D3DFVF_DIFFUSE)
    pDeclarator[i++] = D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR);
  if (FVF & D3DFVF_SPECULAR)
    pDeclarator[i++] = D3DVSD_REG(D3DVSDE_SPECULAR, D3DVSDT_D3DCOLOR);
  for (int t = 0; t < texCount; ++t) {
    const DWORD sizeBits = (FVF >> (t * 2 + 16)) & 0x3;
    DWORD type = D3DVSDT_FLOAT2;
    if (sizeBits == 3)
      type = D3DVSDT_FLOAT1;
    else if (sizeBits == 1)
      type = D3DVSDT_FLOAT3;
    else if (sizeBits == 2)
      type = D3DVSDT_FLOAT4;
    pDeclarator[i++] = D3DVSD_REG(D3DVSDE_TEXCOORD0 + t, type);
  }
  if (i >= MAX_FVF_DECL_SIZE - 1)
    return E_FAIL;
  pDeclarator[i++] = D3DVSD_END();
  for (; i < MAX_FVF_DECL_SIZE; ++i)
    pDeclarator[i] = 0;
  return D3D_OK;
}

//	The D3DX 4-component vector (the ported A3DVECTOR4 carries the same
//	layout the original D3DX header wrapped).
typedef A3DVECTOR4 D3DXVECTOR4;

//	D3DXMatrixShadow: the flattening matrix per the published Microsoft
//	formula (https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixshadow):
//	P = normalize(Plane); L = Light; d = -dot(P, L);
//	M[row][col] = P[row] * L[col] + (row == col ? d : 0).
inline D3DXMATRIX *D3DXMatrixShadow(D3DXMATRIX *out, const D3DXVECTOR4 *light,
                                    const D3DXPLANE *plane) {
  const float len = std::sqrt(plane->a * plane->a + plane->b * plane->b +
                              plane->c * plane->c);
  const float p[4] = {plane->a / len, plane->b / len, plane->c / len,
                      plane->d / len};
  const float l[4] = {light->x, light->y, light->z, light->w};
  const float d = -(p[0] * l[0] + p[1] * l[1] + p[2] * l[2] + p[3] * l[3]);
  for (int row = 0; row < 4; ++row)
    for (int col = 0; col < 4; ++col)
      out->m[row][col] = p[row] * l[col] + (row == col ? d : 0.0f);
  return out;
}

//	D3DXVec3TransformCoord: the row-vector transform of (x, y, z, 1)
//	with the perspective divide back into w = 1 (the documented
//	D3DXVec3TransformCoord contract).
inline D3DXVECTOR3 *D3DXVec3TransformCoord(D3DXVECTOR3 *out,
                                           const D3DXVECTOR3 *v,
                                           const D3DXMATRIX *m) {
  const float x = v->x, y = v->y, z = v->z;
  const float tx = x * m->_11 + y * m->_21 + z * m->_31 + m->_41;
  const float ty = x * m->_12 + y * m->_22 + z * m->_32 + m->_42;
  const float tz = x * m->_13 + y * m->_23 + z * m->_33 + m->_43;
  const float tw = x * m->_14 + y * m->_24 + z * m->_34 + m->_44;
  const float invW = 1.0f / tw;
  out->x = tx * invW;
  out->y = ty * invW;
  out->z = tz * invW;
  return out;
}

inline D3DXMATRIX *D3DXMatrixPerspectiveFovLH(D3DXMATRIX *out, float fovy,
                                               float aspect, float zn, float zf) {
  const float yscale = 1.0f / std::tan(fovy * 0.5f);
  for (int row = 0; row < 4; ++row)
    for (int col = 0; col < 4; ++col)
      out->m[row][col] = 0.0f;
  out->_11 = yscale / aspect;
  out->_22 = yscale;
  out->_33 = zf / (zf - zn);
  out->_34 = 1.0f;
  out->_43 = -zn * zf / (zf - zn);
  return out;
}

//	D3DXPlaneTransform: the plane transforms as a row vector by the
//	transposed matrix (the call sites pass the inverse-transposed VP to
//	move a world-space plane into clip space).
inline D3DXPLANE *D3DXPlaneTransform(D3DXPLANE *out, const D3DXPLANE *p,
                                     const D3DXMATRIX *m) {
  const float a = p->a;
  const float b = p->b;
  const float c = p->c;
  const float d = p->d;
  D3DXPLANE r;
  r.a = m->_11 * a + m->_12 * b + m->_13 * c + m->_14 * d;
  r.b = m->_21 * a + m->_22 * b + m->_23 * c + m->_24 * d;
  r.c = m->_31 * a + m->_32 * b + m->_33 * c + m->_34 * d;
  r.d = m->_41 * a + m->_42 * b + m->_43 * c + m->_44 * d;
  *out = r;
  return out;
}
#endif
