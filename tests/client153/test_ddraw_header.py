"""The original entry point can declare its DirectDraw dependency."""
import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[2]
class DirectDrawHeaderTest(unittest.TestCase):
    def test_interface_contract(self):
        code = '''#include "ddraw.h"
static_assert(sizeof(DDSCAPS) == 4);
static_assert(DDSCAPS_TEXTURE == 0x00001000);
static_assert(DDSCAPS_LOCALVIDMEM == 0x10000000);
int main() { return 0; }
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'test.cpp'
            exe = pathlib.Path(tmp) / 'test'
            src.write_text(code)
            dirs = ['src/client153/platform/common', 'src/client153/Angelica2/AngelicaCommon/Header']
            subprocess.run(['c++', '-std=c++17', '-fshort-wchar', *(f'-I{ROOT / d}' for d in dirs), str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
if __name__ == '__main__':
    unittest.main()
