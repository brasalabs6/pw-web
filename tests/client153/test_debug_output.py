"""The generic Win32 debug API selects ANSI or Unicode at compile time."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
PLATFORM = ROOT / 'src/client153/platform/common'
COMMON = ROOT / 'src/client153/Angelica2/AngelicaCommon/Header'


class DebugOutputTest(unittest.TestCase):
    def test_generic_debug_output(self):
        code = '''#include "Windows.h"
int main() {
#ifdef UNICODE
  OutputDebugString(L"pw \\u00e1");
#else
  OutputDebugString("pw ansi");
#endif
}'''
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'test.cpp'
            exe = pathlib.Path(tmp) / 'test'
            src.write_text(code)
            for unicode, expected in ((False, b'pw ansi'), (True, 'pw á'.encode())):
                command = ['c++', '-std=c++17', '-fshort-wchar', '-I', str(PLATFORM),
                           '-I', str(COMMON), str(src), str(PLATFORM / 'utf16.cpp'), '-o', str(exe)]
                if unicode:
                    command.insert(1, '-DUNICODE')
                subprocess.run(command, check=True, capture_output=True)
                result = subprocess.run([str(exe)], check=True, capture_output=True)
                self.assertEqual(result.stderr, expected)


if __name__ == '__main__':
    unittest.main()
