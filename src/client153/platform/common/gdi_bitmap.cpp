#include "Windows.h"
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <new>
#include <unordered_set>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Memory-backed DCs and 32-bit DIBs. Browser text rendering supports the
// subset used by the original font atlas; it does not load font files.
namespace {
struct Bitmap { void *data; unsigned selected = 0; bool stock = false; int width = 1, height = 1; bool topDown = true; };
struct Font { int height; DWORD weight, italic; wchar_t face[128]{}; unsigned selected = 0; bool stock = false; };
struct MemoryDC { Bitmap *selected = nullptr; Bitmap *stock = nullptr; Font *font = nullptr; Font *stockFont = nullptr; COLORREF text = 0, background = 0xffffff; UINT align = TA_TOP; };
std::mutex guard;
std::unordered_set<MemoryDC *> contexts;
std::unordered_set<Bitmap *> bitmaps;
std::unordered_set<Font *> fonts;
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
  dc->stockFont = new (std::nothrow) Font{16, FW_NORMAL, 0};
  if (!dc->stockFont) { std::free(dc->stock->data); delete dc->stock; delete dc; return nullptr; }
  dc->stockFont->stock = true; dc->stockFont->selected = 1;
  dc->font = dc->stockFont; fonts.insert(dc->stockFont);
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
  if (dc->font) --dc->font->selected;
  fonts.erase(dc->stockFont); delete dc->stockFont;
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
  bitmap->width = static_cast<int>(width); bitmap->height = static_cast<int>(height);
  bitmap->topDown = info->bmiHeader.biHeight < 0;
  bitmaps.insert(bitmap);
  *bits = bitmap->data;
  return bitmap;
}
HGDIOBJ SelectObject(HDC handle, HGDIOBJ object) {
  std::lock_guard<std::mutex> lock(guard);
  auto *dc = static_cast<MemoryDC *>(handle);
  auto *bitmap = static_cast<Bitmap *>(object);
  auto *font = static_cast<Font *>(object);
  if (contexts.count(dc) && fonts.count(font)) {
    if (font->stock && font != dc->stockFont) return nullptr;
    auto *old = dc->font; --old->selected; dc->font = font; ++font->selected; return old;
  }
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
  auto *font = static_cast<Font *>(object);
  if (fonts.count(font)) {
    if (font->selected || font->stock) return FALSE;
    fonts.erase(font); delete font; return TRUE;
  }
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

#ifdef __EMSCRIPTEN__
EM_JS(int, pw153_canvas_text, (const wchar_t *face, int height, int weight, int italic,
    const wchar_t *text, int count, int width, int rows, int x, int y,
    unsigned textColor, unsigned bgColor, unsigned char *pixels, int topDown, SIZE *size), {
  try {
    const read16 = (ptr, n) => { let s=""; for (let i=0; i<n; ++i) s += String.fromCharCode(HEAPU16[(ptr >> 1) + i]); return s; };
    let faceLen=0; while (faceLen < 127 && HEAPU16[(face >> 1) + faceLen]) ++faceLen;
    const family = read16(face, faceLen).split(String.fromCharCode(34)).join("").split(String.fromCharCode(92)).join("");
    const canvas = new OffscreenCanvas(Math.max(width,1), Math.max(rows,1));
    const ctx = canvas.getContext('2d', {willReadFrequently:true});
    if (!ctx) return 0;
    ctx.font = (italic ? 'italic ' : "") + weight + ' ' + height + 'px ' + (family ? '"'+family+'"' : 'sans-serif');
    ctx.textBaseline = 'top';
    const value = read16(text, count);
    if (size) { HEAP32[size >> 2] = Math.ceil(ctx.measureText(value).width); HEAP32[(size >> 2)+1] = height; }
    if (!pixels) return 1;
    const color = c => 'rgb('+(c&255)+','+((c>>>8)&255)+','+((c>>>16)&255)+')';
    ctx.fillStyle = color(bgColor); ctx.fillRect(0,0,width,rows);
    ctx.fillStyle = color(textColor); ctx.fillText(value,x,y);
    const rgba = ctx.getImageData(0,0,width,rows).data;
    for (let row=0; row<rows; ++row) for (let col=0; col<width; ++col) {
      const src=(row*width+col)*4, dst=pixels+((topDown ? row : rows-1-row)*width+col)*4;
      HEAPU8[dst]=rgba[src+2]; HEAPU8[dst+1]=rgba[src+1]; HEAPU8[dst+2]=rgba[src]; HEAPU8[dst+3]=0;
    }
    return 1;
  } catch (_) { return 0; }
});
#endif

HFONT CreateFontW(int height, int width, int escapement, int orientation, DWORD weight,
                  DWORD italic, DWORD underline, DWORD strikeOut, DWORD, DWORD, DWORD,
                  DWORD, DWORD, const wchar_t *faceName) {
#ifdef __EMSCRIPTEN__
  if (!height || height == INT32_MIN || width || escapement || orientation || underline || strikeOut || !faceName) return nullptr;
  auto *font = new (std::nothrow) Font{height < 0 ? -height : height, weight, italic};
  if (!font) return nullptr;
  int i=0; while (i<127 && faceName[i]) { font->face[i]=faceName[i]; ++i; }
  if (faceName[i]) { delete font; return nullptr; }
  std::lock_guard<std::mutex> lock(guard); fonts.insert(font); return font;
#else
  (void)height; (void)width; (void)escapement; (void)orientation; (void)weight; (void)italic;
  (void)underline; (void)strikeOut; (void)faceName; return nullptr;
#endif
}
BOOL GetTextExtentPoint32W(HDC handle, const wchar_t *text, int count, SIZE *size) {
  if (size) *size = {};
  if (!text || count < 0 || !size) return FALSE;
  std::lock_guard<std::mutex> lock(guard); auto *dc=static_cast<MemoryDC *>(handle);
  if (!contexts.count(dc)) return FALSE;
#ifdef __EMSCRIPTEN__
  return pw153_canvas_text(dc->font->face, dc->font->height, dc->font->weight, dc->font->italic,
                           text, count, 1, 1, 0, 0, 0, 0, nullptr, 1, size) ? TRUE : FALSE;
#else
  return FALSE;
#endif
}
COLORREF SetTextColor(HDC handle, COLORREF color) {
  std::lock_guard<std::mutex> lock(guard); auto *dc=static_cast<MemoryDC *>(handle);
  if (!contexts.count(dc)) return CLR_INVALID;
  auto old=dc->text; dc->text=color; return old;
}
COLORREF SetBkColor(HDC handle, COLORREF color) {
  std::lock_guard<std::mutex> lock(guard); auto *dc=static_cast<MemoryDC *>(handle);
  if (!contexts.count(dc)) return CLR_INVALID;
  auto old=dc->background; dc->background=color; return old;
}
UINT SetTextAlign(HDC handle, UINT align) {
  std::lock_guard<std::mutex> lock(guard); auto *dc=static_cast<MemoryDC *>(handle);
  if (!contexts.count(dc) || align != TA_TOP) return CLR_INVALID;
  auto old=dc->align; dc->align=align; return old;
}
BOOL ExtTextOutW(HDC handle, int x, int y, UINT options, const RECT *rect,
                 const wchar_t *text, UINT count, const int *dx) {
  if (!text || count > INT32_MAX || options != ETO_OPAQUE || rect || dx) return FALSE;
  std::lock_guard<std::mutex> lock(guard); auto *dc=static_cast<MemoryDC *>(handle);
  if (!contexts.count(dc) || !dc->selected || dc->selected->stock) return FALSE;
#ifdef __EMSCRIPTEN__
  auto *bmp=dc->selected;
  return pw153_canvas_text(dc->font->face, dc->font->height, dc->font->weight, dc->font->italic,
                           text, count, bmp->width, bmp->height, x, y, dc->text, dc->background,
                           static_cast<unsigned char *>(bmp->data), bmp->topDown, nullptr) ? TRUE : FALSE;
#else
  (void)x; (void)y; return FALSE;
#endif
}
