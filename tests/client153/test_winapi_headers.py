"""The client entry headers compile independently and shell failure is explicit."""
import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[2]
class ClientWinApiHeaderTest(unittest.TestCase):
    def test_header_and_shell_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'test.cpp'
            exe = pathlib.Path(tmp) / 'test'
            src.write_text('''#include "EC_WinAPI.h"
int main() { char path[] = "a"; return !FAILED(SHGetFolderPathA(nullptr, 0, nullptr, 0, path)) || path[0] != 0; }
''')
            includes = ['src/client153/platform/common', 'src/client153/Angelica2/AngelicaCommon/Header', 'src/client153/Element/CElementClient']
            subprocess.run(['c++', '-std=c++17', '-fshort-wchar', *(f'-I{ROOT / i}' for i in includes), str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
if __name__ == '__main__':
    unittest.main()
