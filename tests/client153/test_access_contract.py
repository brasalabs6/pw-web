"""The original client's file-existence check consults the real filesystem."""
import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[2]
class AccessContractTest(unittest.TestCase):
    def test_existing_and_missing_file(self):
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'test.cpp'
            exe = pathlib.Path(tmp) / 'test'
            file = pathlib.Path(tmp) / 'present'
            file.write_text('test')
            file.chmod(0o700)
            nested = pathlib.Path(tmp) / 'sub'
            nested.mkdir()
            (nested / 'delete.log').write_text('log')
            src.write_text('''#include "Windows.h"
static_assert(sizeof(ATOM) == 2);
static_assert(MB_ICONSTOP == MB_ICONERROR);
int main(int, char **argv) { return _access(argv[1], 0) != 0 || _access(argv[2], 0) != -1 || _access(argv[1], 1) != -1 || _access(nullptr, 0) != -1 || !DeleteFileA(argv[3]) || DeleteFileA(argv[3]) || DeleteFileA(argv[4]) || DeleteFileA(nullptr); }
''')
            dirs = ['src/client153/platform/common', 'src/client153/Angelica2/AngelicaCommon/Header']
            subprocess.run(['c++', '-std=c++17', '-fshort-wchar', *(f'-I{ROOT / d}' for d in dirs), str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe), str(file), str(file) + '-missing', str(nested) + r'\delete.log', str(nested)], check=True)
if __name__ == '__main__':
    unittest.main()
