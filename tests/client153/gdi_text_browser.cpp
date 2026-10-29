#include "Windows.h"
#include <emscripten.h>
#include <cstring>

int main() {
  HDC dc = CreateCompatibleDC(nullptr);
  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = 64; info.bmiHeader.biHeight = -32;
  info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32; info.bmiHeader.biCompression = BI_RGB;
  void *bits = nullptr;
  HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
  HGDIOBJ previous = SelectObject(dc, bitmap);
  HFONT font = CreateFontW(-20, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"sans-serif");
  if (!font) { EM_ASM({document.body.dataset.result='font-create-failed'}); return 1; }
  HGDIOBJ previousFont = SelectObject(dc, font);
  if (!previousFont || DeleteObject(font)) return 6;
  SIZE size{};
  if (!GetTextExtentPoint32W(dc, L"W", 1, &size) || size.cx <= 0 || size.cy <= 0) {
    EM_ASM({document.body.dataset.result='font-measure-failed'}); return 2;
  }
  SetTextColor(dc, RGB(255,255,255)); SetBkColor(dc, RGB(0,0,0));
  if (!ExtTextOutW(dc, 0, 0, ETO_OPAQUE, nullptr, L"W", 1, nullptr)) {
    EM_ASM({document.body.dataset.result='font-draw-failed'}); return 3;
  }
  bool painted = false; auto *pixels = static_cast<unsigned char*>(bits);
  for (int i = 0; i < 64*32*4; i += 4) if (pixels[i] != 0) painted = true;
  if (!painted) { EM_ASM({document.body.dataset.result='font-empty'}); return 4; }
  SIZE accented{};
  if (!GetTextExtentPoint32W(dc, L"Olá", 3, &accented) || accented.cx <= 0 || accented.cy <= 0) return 7;
  if (ExtTextOutW(dc, 0, 0, ETO_CLIPPED, nullptr, L"W", 1, nullptr)) return 8;
  SelectObject(dc, previousFont); SelectObject(dc, previous);
  if (!DeleteObject(font) || !DeleteObject(bitmap) || !DeleteDC(dc)) return 5;
  EM_ASM({document.body.dataset.result='pass'});
}
