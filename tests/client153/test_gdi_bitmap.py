"""Memory DC and DIB lifecycle used by the original font manager."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
PLATFORM = ROOT / 'src/client153/platform/common'
COMMON = ROOT / 'src/client153/Angelica2/AngelicaCommon/Header'

class GdiBitmapTest(unittest.TestCase):
    def test_memory_bitmap_lifecycle_and_invalid_input(self):
        code = r'''#include "Windows.h"
int main() {
  HDC dc = CreateCompatibleDC(nullptr); if (!dc) return 1;
  BITMAPINFO info{}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth=4; info.bmiHeader.biHeight=-4;
  info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=32; info.bmiHeader.biCompression=BI_RGB;
  void *bits=nullptr; HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0);
  if (!bitmap || !bits) return 2;
  auto *pixels=static_cast<unsigned char *>(bits); pixels[0]=42; pixels[63]=81;
  HGDIOBJ old=SelectObject(dc,bitmap); if (!old || DeleteObject(bitmap)) return 3;
  if (SelectObject(dc,old) != bitmap || !DeleteObject(bitmap)) return 4;
  if (!DeleteDC(dc)) return 8;
  if (DeleteObject(bitmap) || DeleteDC(dc)) return 5;
  info.bmiHeader.biWidth=0; bits=reinterpret_cast<void *>(1);
  if (CreateDIBSection(nullptr,&info,DIB_RGB_COLORS,&bits,nullptr,0) || bits) return 6;
  info.bmiHeader.biWidth=2147483647; info.bmiHeader.biHeight=(-2147483647-1);
  if (CreateDIBSection(nullptr,&info,DIB_RGB_COLORS,&bits,nullptr,0) || bits) return 7;
}'''
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'test.cpp'; exe = pathlib.Path(tmp) / 'test'
            src.write_text(code)
            subprocess.run(['c++', '-std=c++17', '-fshort-wchar', '-I', str(PLATFORM),
                            '-I', str(COMMON), str(src), str(PLATFORM / 'gdi_bitmap.cpp'),
                            '-o', str(exe)], check=True, capture_output=True)
            subprocess.run([str(exe)], check=True)

if __name__ == '__main__': unittest.main()
