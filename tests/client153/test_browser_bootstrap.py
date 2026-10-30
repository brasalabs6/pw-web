"""Browser startup must not launch the Windows patcher or enforce its mutex."""
import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[2]
ENTRY = ROOT / 'src/client153/Element/CElementClient/ElementClient.cpp'
class BrowserBootstrapTest(unittest.TestCase):
    def test_browser_probe_keeps_window_until_game_stops(self):
        source = (ROOT / 'scripts/client153-browser-entry.cpp').read_text()
        start = source.index('extern "C" int pw153_browser_probe()')
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'probe.cpp'
            src.write_text('''
#include <cstdio>
#include <cstdint>
struct EmscriptenWebGLContextAttributes { int majorVersion; bool antialias; };
void emscripten_webgl_init_context_attributes(EmscriptenWebGLContextAttributes*) {}
int emscripten_webgl_create_context(const char*, EmscriptenWebGLContextAttributes*) { return 1; }
int emscripten_webgl_make_context_current(int) { return 0; }
constexpr int EMSCRIPTEN_RESULT_SUCCESS = 0;
bool pw153_window_attach(void*) { return true; }
int detached = 0, tracked = 0, result = 0;
void pw153_window_detach() { ++detached; }
void pw153_track_browser_window() { ++tracked; }
void pw153_vfs_install_casefold() {}
int wWinMain(void*, void*, wchar_t*, int) { return result; }
''' + source[start:] + '''
int main() {
 if (pw153_browser_probe() != 0 || detached || tracked != 1) return 1;
 result = -1; if (pw153_browser_probe() != -1 || detached != 1 || tracked != 1) return 2;
}
''')
            exe = pathlib.Path(tmp) / 'probe'
            subprocess.run(['c++', '-std=c++17', str(src), '-o', str(exe)], check=True, capture_output=True)
            subprocess.run([str(exe)], check=True, capture_output=True)

    def test_browser_frame_runs_game_and_cancels_after_shutdown(self):
        source = ENTRY.read_bytes().decode('utf-8', errors='surrogateescape')
        start = source.index('static void pw153_browser_frame()')
        end = source.index('\n#endif', start)
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'browser_frame.cpp'
            src.write_text('''
#include <cstdio>
int calls = 0, cancelled = 0, detached = 0;
struct Game { bool RunInMainThread() { return ++calls < 2; } } l_theGame;
void emscripten_cancel_main_loop() { ++cancelled; }
void pw153_window_detach() { ++detached; }
''' + source[start:end] + '''
int main() {
 pw153_browser_frame(); if (calls != 1 || cancelled) return 1;
 pw153_browser_frame(); if (calls != 2 || cancelled != 1 || detached != 1) return 2;
}
''')
            exe = pathlib.Path(tmp) / 'browser_frame'
            subprocess.run(['c++', '-std=c++17', str(src), '-o', str(exe)], check=True, capture_output=True)
            subprocess.run([str(exe)], check=True)

    def test_render_dimensions_use_independent_command_line_values(self):
        source = ENTRY.read_bytes().decode('utf-8', errors='surrogateescape')
        start = source.index('\tif( l_CmdParams.iRenderWid == -1 )')
        end = source.index('\n\tif (LOBYTE', start)
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'dimensions.cpp'
            src.write_text('''
struct Settings { int iRndWidth = 1280, iRndHeight = 720; };
struct Config { Settings settings; Settings& GetSystemSettings() { return settings; } } g_GameCfgs;
struct Dimensions { int iRenderWid, iRenderHei; } l_CmdParams, GameInit;
void configure() {
''' + source[start:end] + '''
}
int main() {
 l_CmdParams = {-1, -1}; configure();
 if (GameInit.iRenderWid != 1280 || GameInit.iRenderHei != 720) return 1;
 l_CmdParams = {1920, 1080}; configure();
 if (GameInit.iRenderWid != 1920 || GameInit.iRenderHei != 1080) return 2;
 l_CmdParams = {1024, -1}; configure();
 if (GameInit.iRenderWid != 1024 || GameInit.iRenderHei != 720) return 3;
 return 0;
}
''')
            exe = pathlib.Path(tmp) / 'dimensions'
            subprocess.run(['c++', '-std=c++17', str(src), '-o', str(exe)], check=True, capture_output=True)
            subprocess.run([str(exe)], check=True)

    def test_browser_skips_windows_launcher(self):
        source = ENTRY.read_bytes().decode('utf-8', errors='surrogateescape')
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
