// Run against the original FTDriver wrapper and an authorized TTF. The caller
// supplies and mounts the font; proprietary data is never committed here.
#include "FTInterface.h"
#include <cstdio>

int main(int argc, char **argv) {
  if (argc != 2) return 1;
  IFTManager *manager = CreateFTManager(32, 128, 512 * 1024);
  if (!manager) return 2;
  IFTFont *font = manager->CreateFont(argv[1], 0, 120, STYLE_NORMAL, 0);
  if (!font) { manager->Release(); return 3; }
  FTDSIZE offset{}, size{}, bitmap{};
  if (!font->GetCharExtent(L'A', &offset, &size, &bitmap) || bitmap.cx <= 0 || bitmap.cy <= 0) return 4;
  unsigned char pixels[64 * 64]{};
  if (!font->DrawChar(L'A', pixels, 64, 64)) return 5;
  unsigned coverage = 0;
  for (auto value : pixels) coverage += value;
  FTDSIZE textSize{};
  int fit = 0;
  if (!font->GetTextExtentExPoint(L"AB", 2, 64, &fit, nullptr, &textSize) || fit != 2 || textSize.cx <= 0) return 7;
  std::printf("glyph %dx%d coverage=%u\n", bitmap.cx, bitmap.cy, coverage);
  font->Release();
  manager->Release();
  return coverage ? 0 : 6;
}
