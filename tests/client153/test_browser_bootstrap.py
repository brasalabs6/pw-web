"""Browser startup must not launch the Windows patcher or enforce its mutex."""
import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[2]
ENTRY = ROOT / 'src/client153/Element/CElementClient/ElementClient.cpp'
class BrowserBootstrapTest(unittest.TestCase):
    def test_browser_skips_windows_launcher(self):
        source = ENTRY.read_text()
        start = source.index('\twhile (true)', source.index('int APIENTRY wWinMain'))
        end = source.index('#ifdef _PROFILE_MEMORY', start)
        start = source.rfind('\tHANDLE hInstMutex = NULL;', 0, start)
        section = source[start:end]
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'bootstrap.cpp'
            src.write_text('''#include <cstddef>
using HANDLE = void*;
int boot() {
 bool bStartPatcher = false, bFirstInstance = true;
''' + section + '''
 return bStartPatcher ? 1 : (bFirstInstance ? 0 : 2);
}
int main() { return boot(); }
''')
            exe = pathlib.Path(tmp) / 'bootstrap'
            subprocess.run(['c++', '-std=c++17', '-D__EMSCRIPTEN__', str(src), '-o', str(exe)], check=True, capture_output=True)
            subprocess.run([str(exe)], check=True)
if __name__ == '__main__': unittest.main()
