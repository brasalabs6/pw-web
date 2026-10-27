"""The Win32 cursor API reports only positions supplied by the browser host."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
PLATFORM = ROOT / 'src/client153/platform/common'
COMMON = ROOT / 'src/client153/Angelica2/AngelicaCommon/Header'

class CursorPositionTest(unittest.TestCase):
    def test_host_position(self):
        code = '''#include "Windows.h"
extern "C" void pw153_set_cursor_position(LONG, LONG);
int main() {
  POINT p{77, 88};
  if (GetCursorPos(&p) || p.x != 77 || p.y != 88) return 1;
  pw153_set_cursor_position(-120, 347);
  if (!GetCursorPos(&p) || p.x != -120 || p.y != 347) return 2;
  if (GetCursorPos(nullptr)) return 3;
  pw153_set_cursor_position(42, 12);
  if (!GetCursorPos(&p) || p.x != 42 || p.y != 12) return 4;
}'''
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'cursor.cpp'
            exe = pathlib.Path(tmp) / 'cursor'
            src.write_text(code)
            subprocess.run(['c++', '-std=c++17', '-fshort-wchar', '-I', str(PLATFORM),
                            '-I', str(COMMON), str(src), str(PLATFORM / 'keystate.cpp'),
                            '-o', str(exe)], check=True, capture_output=True)
            subprocess.run([str(exe)], check=True)

if __name__ == '__main__':
    unittest.main()
