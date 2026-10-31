"""The client window must be backed by an attached host and deliver lifecycle events."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
PLATFORM = ROOT / 'src/client153/platform/common'
COMMON = ROOT / 'src/client153/Angelica2/AngelicaCommon/Header'

class BrowserWindowTest(unittest.TestCase):
    def test_window_requires_host_and_delivers_lifecycle(self):
        code = r'''#include "Windows.h"
extern "C" bool pw153_window_attach(void *host);
extern "C" void pw153_window_detach();
extern "C" bool pw153_window_set_active(bool active);
extern "C" bool pw153_window_set_focus(bool focused);
static int created, destroyed, activation, focus;
static LRESULT callback(HWND, UINT message, WPARAM value, LPARAM) {
  if (message == WM_CREATE) ++created;
  if (message == WM_DESTROY) ++destroyed;
  if (message == WM_ACTIVATEAPP) activation += value ? 1 : 10;
  if (message == WM_SETFOCUS) ++focus;
  if (message == WM_KILLFOCUS) focus += 10;
  return 0;
}
int main() {
  WNDCLASSEXW cls{}; cls.cbSize = sizeof(cls); cls.lpfnWndProc = callback;
  cls.lpszClassName = L"PWWindow";
  if (!RegisterClassExW(&cls)) return 1;
  if (RegisterClassExW(&cls)) return 2;
  if (CreateWindowExW(0, L"PWWindow", L"PW", 0, 0, 0, 800, 600, nullptr, nullptr, nullptr, nullptr)) return 3;
  int host = 0;
  if (!pw153_window_attach(&host)) return 4;
  HWND window = CreateWindowExW(0, L"PWWindow", L"PW", 0, 0, 0, 800, 600, nullptr, nullptr, nullptr, nullptr);
  if (!window || created != 1) return 5;
  if (!pw153_window_set_active(true) || !pw153_window_set_active(true) || activation != 1) return 8;
  if (!pw153_window_set_focus(true) || focus != 1) return 9;
  if (!pw153_window_set_focus(false) || focus != 11) return 10;
  if (!pw153_window_set_active(false) || activation != 11) return 11;
  if (!DestroyWindow(window) || destroyed != 1 || DestroyWindow(window)) return 6;
  pw153_window_detach();
  if (!RegisterClassExW(&cls)) return 7;
}'''
        with tempfile.TemporaryDirectory() as tmp:
            src = pathlib.Path(tmp) / 'window.cpp'
            exe = pathlib.Path(tmp) / 'window'
            src.write_text(code)
            subprocess.run(['c++', '-std=c++17', '-fshort-wchar', '-I', str(PLATFORM),
                            '-I', str(COMMON), str(src), str(PLATFORM / 'window.cpp'),
                            '-o', str(exe)], check=True, capture_output=True)
            subprocess.run([str(exe)], check=True)

if __name__ == '__main__':
    unittest.main()
