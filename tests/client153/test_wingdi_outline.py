"""Win32 glyph-outline wire layouts and explicit unsupported GDI operation."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class GlyphOutlineContractTest(unittest.TestCase):
    def test_layout_and_unavailable_outline(self):
        code = '''#include "Windows.h"
#include "wingdi.h"
#include <cstddef>
static_assert(sizeof(POINTFX) == 8);
static_assert(sizeof(TTPOLYCURVE) == 12);
static_assert(offsetof(TTPOLYCURVE, apfx) == 4);
static_assert(sizeof(TTPOLYGONHEADER) == 16);
static_assert(TT_PRIM_LINE == 1 && TT_PRIM_QSPLINE == 2 && TT_PRIM_CSPLINE == 3);
int main() {
  GLYPHMETRICS metrics{};
  MAT2 transform{};
  return GetGlyphOutline(nullptr, 65, GGO_NATIVE, &metrics, 0, nullptr, &transform) == GDI_ERROR ? 0 : 1;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source = pathlib.Path(tmp) / 'outline.cpp'
            binary = pathlib.Path(tmp) / 'outline'
            source.write_text(code)
            includes = ['src/client153/platform/common', 'src/client153/Angelica2/AngelicaCommon/Header']
            subprocess.run(['c++', '-std=c++17', '-fshort-wchar', *(f'-I{ROOT / d}' for d in includes), str(source), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
