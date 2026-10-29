#include "Windows.h"
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <new>
#include <unordered_set>

// Real memory-backed DCs and 32-bit DIBs only. Font selection and text
// rendering remain unsupported until a font rasterizer is connected.
namespace {
struct Bitmap { void *data; unsigned selected = 0; bool stock = false; };
struct MemoryDC { Bitmap *selected = nullptr; Bitmap *stock = nullptr; };
std::mutex guard;
std::unordered_set<MemoryDC *> contexts;
std::unordered_set<Bitmap *> bitmaps;
}
HDC CreateCompatibleDC(HDC source) {
  std::lock_guard<std::mutex> lock(guard);
  if (source && !contexts.count(static_cast<MemoryDC *>(source))) return nullptr;
  auto *dc = new (std::nothrow) MemoryDC;
  if (!dc) return nullptr;
  dc->stock = new (std::nothrow) Bitmap{};
  if (!dc->stock) { delete dc; return nullptr; }
  dc->stock->data = std::calloc(1, 4);
  if (!dc->stock->data) { delete dc->stock; delete dc; return nullptr; }
  dc->stock->stock = true;
  dc->stock->selected = 1;
  dc->selected = dc->stock;
  bitmaps.insert(dc->stock);
  contexts.insert(dc);
  return dc;
}
BOOL DeleteDC(HDC handle) {
  std::lock_guard<std::mutex> lock(guard);
  auto *dc = static_cast<MemoryDC *>(handle);
  if (!contexts.erase(dc)) return FALSE;
  if (dc->selected) --dc->selected->selected;
  bitmaps.erase(dc->stock);
  std::free(dc->stock->data);
  delete dc->stock;
  delete dc;
  return TRUE;
}
HBITMAP CreateDIBSection(HDC handle, const BITMAPINFO *info, UINT usage,
                         void **bits, HANDLE section, DWORD offset) {
  if (bits) *bits = nullptr;
  if (!bits || !info || usage != DIB_RGB_COLORS || section || offset ||
      info->bmiHeader.biSize != sizeof(BITMAPINFOHEADER) || info->bmiHeader.biWidth <= 0 ||
      !info->bmiHeader.biHeight || info->bmiHeader.biPlanes != 1 ||
      info->bmiHeader.biBitCount != 32 || info->bmiHeader.biCompression != BI_RGB) return nullptr;
  const uint64_t height = info->bmiHeader.biHeight < 0 ?
      -static_cast<int64_t>(info->bmiHeader.biHeight) : info->bmiHeader.biHeight;
  const uint64_t width = static_cast<uint64_t>(info->bmiHeader.biWidth);
  if (height > UINT32_MAX / 4 || width > (UINT32_MAX / 4) / height) return nullptr;
  const uint64_t size = width * height * 4;
  if (size > std::numeric_limits<size_t>::max() || size > UINT32_MAX) return nullptr;
  std::lock_guard<std::mutex> lock(guard);
  if (handle && !contexts.count(static_cast<MemoryDC *>(handle))) return nullptr;
  auto *bitmap = new (std::nothrow) Bitmap{};
  if (!bitmap) return nullptr;
  bitmap->data = std::calloc(1, static_cast<size_t>(size));
  if (!bitmap->data) { delete bitmap; return nullptr; }
  bitmaps.insert(bitmap);
  *bits = bitmap->data;
  return bitmap;
}
HGDIOBJ SelectObject(HDC handle, HGDIOBJ object) {
  std::lock_guard<std::mutex> lock(guard);
  auto *dc = static_cast<MemoryDC *>(handle);
  auto *bitmap = static_cast<Bitmap *>(object);
  if (!contexts.count(dc) || !bitmaps.count(bitmap) ||
      (bitmap->stock && bitmap != dc->stock) || (bitmap->selected && bitmap != dc->selected)) return nullptr;
  auto *old = dc->selected;
  if (old) --old->selected;
  dc->selected = bitmap;
  ++bitmap->selected;
  return old;
}
BOOL DeleteObject(HGDIOBJ object) {
  std::lock_guard<std::mutex> lock(guard);
  auto *bitmap = static_cast<Bitmap *>(object);
  if (!bitmaps.count(bitmap) || bitmap->selected || bitmap->stock) return FALSE;
  bitmaps.erase(bitmap);
  std::free(bitmap->data);
  delete bitmap;
  return TRUE;
}
int SetMapMode(HDC handle, int mode) {
  std::lock_guard<std::mutex> lock(guard);
  return contexts.count(static_cast<MemoryDC *>(handle)) && mode == MM_TEXT ? MM_TEXT : 0;
}
