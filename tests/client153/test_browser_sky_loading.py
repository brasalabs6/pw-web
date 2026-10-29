"""Browser sky transitions must load without an unsupported native thread."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'src/client153/Angelica2/Angelica3D/Source/A3DSkySphere.cpp'


class BrowserSkyLoadingTest(unittest.TestCase):
    def test_sky_loads_before_transition_and_reports_failure(self):
        source = SOURCE.read_bytes().decode('utf-8', errors='surrogateescape')
        start = source.index('\tif( m_nTransTime == 0 )', source.index('bool A3DSkySphere::TransSky'))
        end = source.index('\n\treturn true;', start)
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / 'sky.cpp'
            path.write_text('''
#include <cstddef>
using DWORD = unsigned; using HANDLE = void*; using LPVOID = void*;
int loads = 0, switches = 0, threads = 0; bool fail = false;
struct A3DSkySphere { int m_nTransTime = 0; bool m_bTextureLoading = false;
  void SwitchClouds() { ++switches; } bool run(); };
int SkyLoadTextures(A3DSkySphere*) { ++loads; return fail ? -1 : 0; }
HANDLE CreateThread(void*, int, int (*)(A3DSkySphere*), void*, int, DWORD*) { ++threads; return nullptr; }
bool A3DSkySphere::run() {
''' + source[start:end] + '''
 return true;
}
int main() {
 A3DSkySphere sky; sky.m_nTransTime = 20;
 if (!sky.run() || loads != 1 || switches || threads) return 1;
 sky.m_nTransTime = 0; if (!sky.run() || loads != 2 || switches != 1) return 2;
 fail = true; sky.m_nTransTime = 20; if (sky.run() || loads != 3 || threads) return 3;
}
''')
            exe = pathlib.Path(tmp) / 'sky'
            subprocess.run(['c++', '-std=c++17', '-D__EMSCRIPTEN__', str(path), '-o', str(exe)],
                           check=True, capture_output=True)
            subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
