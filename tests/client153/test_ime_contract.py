"""Win32 UTF-16 length and message values used by the original UI manager."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
PLATFORM = ROOT / 'src/client153/platform/common'
COMMON = ROOT / 'src/client153/Angelica2/AngelicaCommon/Header'

class ImeContractTest(unittest.TestCase):
    def test_win32_text_contract(self):
        code = '''#include "Windows.h"
static_assert(WM_INPUTLANGCHANGE == 0x0051);
static_assert(WM_IME_STARTCOMPOSITION == 0x010d);
static_assert(WM_IME_ENDCOMPOSITION == 0x010e);
static_assert(WM_IME_COMPOSITION == 0x010f);
static_assert(WM_IME_SETCONTEXT == 0x0281);
static_assert(WM_IME_NOTIFY == 0x0282);
static_assert(WM_IME_CHAR == 0x0286);
int main() {
  const wchar_t s[] = {L'A', 0xd83d, 0xde00, 0};
  return lstrlenW(s) != 3 || lstrlenW(L"") != 0 || lstrlenW(nullptr) != 0;
}'''
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'contract.cpp'
            exe = pathlib.Path(tmp) / 'contract'
            src.write_text(code)
            subprocess.run(['c++', '-std=c++17', '-fshort-wchar', '-I', str(PLATFORM),
                            '-I', str(COMMON), str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

if __name__ == '__main__':
    unittest.main()
