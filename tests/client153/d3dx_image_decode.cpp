// D3DX image decode regression test (node/WASM). Validates the formats the
// original client's UI and world assets use against known-answer vectors:
// DXT1 (1-bit alpha), DXT5 (interpolated alpha palette), 24/32-bit BMP with
// the colour-key contract. The destination surface is a CPU image surface
// (pw153_create_image_surface needs no WebGL context in node).
#include "d3d8.h"
#include "d3d8_surface.h"
#include "d3dx8.h"
#include <cstdio>
#include <cstring>

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

bool pixel_is(const unsigned char *base, int pitch, int x, int y,
              unsigned char b, unsigned char g, unsigned char r, unsigned char a) {
  const unsigned char *p = base + size_t(y) * pitch + size_t(x) * 4;
  return p[0] == b && p[1] == g && p[2] == r && p[3] == a;
}

void put32(unsigned char *out, unsigned value) {
  out[0] = value & 255;
  out[1] = (value >> 8) & 255;
  out[2] = (value >> 16) & 255;
  out[3] = (value >> 24) & 255;
}

}  // namespace

int main() {
  // ---- DXT1: 4x4, c0=red c1=blue (c0 > c1: opaque 4-colour mode) ----
  {
    unsigned char dds[136]{};
    std::memcpy(dds, "DDS ", 4);
    put32(dds + 4, 124);
    put32(dds + 12, 4);   // height
    put32(dds + 16, 4);   // width
    put32(dds + 76, 32);  // pixel format size
    put32(dds + 80, 0x4); // DDPF_FOURCC
    put32(dds + 84, D3DFMT_DXT1);
    // Block: c0=0xF800 (red), c1=0x001F (blue), indices 0,1,2,3.
    dds[128] = 0x00; dds[129] = 0xF8;
    dds[130] = 0x1F; dds[131] = 0x00;
    dds[132] = 0xE4;  // 2-bit indices: 0,1,2,3
    IDirect3DSurface8 *surface = pw153_create_image_surface(4, 4, D3DFMT_A8R8G8B8);
    check("dxt1 create image surface", surface != nullptr);
    if (surface) {
      check("dxt1 decode",
            D3DXLoadSurfaceFromFileInMemory(surface, nullptr, nullptr, dds, sizeof(dds),
                                            nullptr, 0, 0, nullptr) == D3D_OK);
      D3DLOCKED_RECT lock{};
      check("dxt1 lock", surface->LockRect(&lock, nullptr, D3DLOCK_READONLY) == D3D_OK);
      const auto *base = static_cast<const unsigned char *>(lock.pBits);
      check("dxt1 pixel0 red", pixel_is(base, lock.Pitch, 0, 0, 0, 0, 255, 255));
      check("dxt1 pixel1 blue", pixel_is(base, lock.Pitch, 1, 0, 255, 0, 0, 255));
      check("dxt1 pixel2 mix 2:1", pixel_is(base, lock.Pitch, 2, 0, 85, 0, 170, 255));
      check("dxt1 pixel3 mix 1:2", pixel_is(base, lock.Pitch, 3, 0, 170, 0, 85, 255));
      check("dxt1 row2 red", pixel_is(base, lock.Pitch, 0, 2, 0, 0, 255, 255));
      surface->UnlockRect();
      surface->Release();
    }
  }
  // ---- DXT5: 4x4, alpha endpoints 255/0, colour all red ----
  {
    unsigned char dds[144]{};
    std::memcpy(dds, "DDS ", 4);
    put32(dds + 4, 124);
    put32(dds + 12, 4);
    put32(dds + 16, 4);
    put32(dds + 76, 32);
    put32(dds + 80, 0x4);
    put32(dds + 84, D3DFMT_DXT5);
    // Alpha block: a0=255, a1=0, 3-bit indices 0,1,6,7 for the first pixels.
    dds[128] = 255; dds[129] = 0;
    dds[130] = 136; dds[131] = 15;  // indices: p0=0(255) p1=1(0) p2=6(0) p3=7(255)
    // Colour block at 8: c0=c1=red, indices all 0.
    dds[136] = 0x00; dds[137] = 0xF8;
    dds[138] = 0x00; dds[139] = 0xF8;
    IDirect3DSurface8 *surface = nullptr;
    surface = pw153_create_image_surface(4, 4, D3DFMT_A8R8G8B8);
    if (surface) {
      check("dxt5 decode",
            D3DXLoadSurfaceFromFileInMemory(surface, nullptr, nullptr, dds, sizeof(dds),
                                            nullptr, 0, 0, nullptr) == D3D_OK);
      D3DLOCKED_RECT lock{};
      check("dxt5 lock", surface->LockRect(&lock, nullptr, D3DLOCK_READONLY) == D3D_OK);
      const auto *base = static_cast<const unsigned char *>(lock.pBits);
      check("dxt5 pixel0 red opaque", pixel_is(base, lock.Pitch, 0, 0, 0, 0, 255, 255));
      check("dxt5 pixel1 red transparent", pixel_is(base, lock.Pitch, 1, 0, 0, 0, 255, 0));
      check("dxt5 pixel2 red alpha72", pixel_is(base, lock.Pitch, 2, 0, 0, 0, 255, 72));
      check("dxt5 pixel3 red alpha36", pixel_is(base, lock.Pitch, 3, 0, 0, 0, 255, 36));
      surface->UnlockRect();
      surface->Release();
    }
  }
  // ---- BMP 24-bit: 3x3, top red / middle green / bottom blue (bottom-up) ----
  {
    const unsigned width = 3, height = 3;
    const unsigned rowBytes = (width * 3 + 3) / 4 * 4;  // 9 -> 12
    const unsigned pixelOffset = 14 + 40;
    unsigned char bmp[54 + rowBytes * height]{};
    bmp[0] = 'B'; bmp[1] = 'M';
    put32(bmp + 2, sizeof(bmp));
    put32(bmp + 10, pixelOffset);
    put32(bmp + 14, 40);
    put32(bmp + 18, width);
    put32(bmp + 22, height);
    bmp[28] = 24;
    // Bottom-up rows: bottom blue, middle green, top red.
    for (unsigned row = 0; row < height; ++row) {
      unsigned char *rowBase = bmp + pixelOffset + row * rowBytes;
      const unsigned char colour[3] = {static_cast<unsigned char>(row == 0 ? 255u : 0u),   // B
                                       static_cast<unsigned char>(row == 1 ? 255u : 0u),   // G
                                       static_cast<unsigned char>(row == 2 ? 255u : 0u)};  // R
      for (unsigned x = 0; x < width; ++x) {
        rowBase[x * 3] = colour[0];
        rowBase[x * 3 + 1] = colour[1];
        rowBase[x * 3 + 2] = colour[2];
      }
    }
    D3DXIMAGE_INFO info{};
    check("bmp24 info", D3DXGetImageInfoFromFileInMemory(bmp, sizeof(bmp), &info) == D3D_OK &&
                            info.Width == 3 && info.Height == 3 &&
                            info.ImageFileFormat == D3DXIFF_BMP &&
                            info.Format == D3DFMT_X8R8G8B8);
    IDirect3DSurface8 *surface = nullptr;
    surface = pw153_create_image_surface(3, 3, D3DFMT_A8R8G8B8);
    if (surface) {
      check("bmp24 decode",
            D3DXLoadSurfaceFromFileInMemory(surface, nullptr, nullptr, bmp, sizeof(bmp),
                                            nullptr, 0, 0, nullptr) == D3D_OK);
      D3DLOCKED_RECT lock{};
      check("bmp24 lock", surface->LockRect(&lock, nullptr, D3DLOCK_READONLY) == D3D_OK);
      const auto *base = static_cast<const unsigned char *>(lock.pBits);
      check("bmp24 top red", pixel_is(base, lock.Pitch, 0, 0, 0, 0, 255, 255));
      check("bmp24 middle green", pixel_is(base, lock.Pitch, 1, 1, 0, 255, 0, 255));
      check("bmp24 bottom blue", pixel_is(base, lock.Pitch, 2, 2, 255, 0, 0, 255));
      surface->UnlockRect();
      surface->Release();
    }
  }
  // ---- BMP 32-bit with colour key: 2x2, one keyed pixel ----
  {
    const unsigned width = 2, height = 2;
    unsigned char bmp[54 + width * height * 4]{};
    bmp[0] = 'B'; bmp[1] = 'M';
    put32(bmp + 2, sizeof(bmp));
    put32(bmp + 10, 54);
    put32(bmp + 14, 40);
    put32(bmp + 18, width);
    put32(bmp + 22, height);
    bmp[28] = 32;
    // Bottom-up: row0 (bottom) = keyed blue pixels; row1 (top) = opaque red.
    put32(bmp + 54, 0x00FF0000);      // B=255,G=0,R=0 -> keyed
    put32(bmp + 58, 0x00FF0000);
    put32(bmp + 62, 0xFF0000FF);      // opaque red
    put32(bmp + 66, 0xFF0000FF);
    IDirect3DSurface8 *surface = nullptr;
    surface = pw153_create_image_surface(2, 2, D3DFMT_A8R8G8B8);
    if (surface) {
      // The client's colour key is the A8R8G8B8 value 0x00FF0000 (blue).
      check("bmp32 decode keyed",
            D3DXLoadSurfaceFromFileInMemory(surface, nullptr, nullptr, bmp, sizeof(bmp),
                                            nullptr, 0, 0x00FF0000, nullptr) == D3D_OK);
      D3DLOCKED_RECT lock{};
      check("bmp32 lock", surface->LockRect(&lock, nullptr, D3DLOCK_READONLY) == D3D_OK);
      const auto *base = static_cast<const unsigned char *>(lock.pBits);
      check("bmp32 keyed bottom transparent", pixel_is(base, lock.Pitch, 0, 1, 0, 0, 255, 0));
      check("bmp32 top blue opaque", pixel_is(base, lock.Pitch, 0, 0, 255, 0, 0, 255));
      surface->UnlockRect();
      surface->Release();
    }
  }
  if (failures) {
    std::printf("%d failures\n", failures);
    return 1;
  }
  std::printf("all d3dx image decode checks passed\n");
  return 0;
}
