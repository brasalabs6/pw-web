#include "Windows.h"
#include <string>

// One browser canvas backs one client window. This registry does not model
// child windows or menus; the input adapter dispatches to the registered
// procedure through pw153_window_procedure().
namespace {
std::u16string name;
WNDPROC procedure = nullptr;
void *canvas = nullptr;
struct Window { WNDPROC procedure; } window{};
bool live = false;
bool active = false;
bool focused = false;
std::u16string copy_name(const wchar_t *value) {
  std::u16string result;
  if (value) while (*value) result.push_back(static_cast<char16_t>(*value++));
  return result;
}
}

extern "C" bool pw153_window_attach(void *host) {
  if (!host || canvas || live) return false;
  canvas = host;
  return true;
}
extern "C" void pw153_window_detach() {
  if (live) DestroyWindow(&window);
  canvas = nullptr;
  procedure = nullptr;
  name.clear();
}
extern "C" bool pw153_window_set_active(bool value) {
  if (!live || !window.procedure) return false;
  if (active != value) {
    active = value;
    window.procedure(&window, WM_ACTIVATEAPP, value ? TRUE : FALSE, 0);
  }
  return true;
}
extern "C" bool pw153_window_set_focus(bool value) {
  if (!live || !window.procedure) return false;
  if (focused != value) {
    focused = value;
    window.procedure(&window, value ? WM_SETFOCUS : WM_KILLFOCUS, 0, 0);
  }
  return true;
}
ATOM RegisterClassExW(const WNDCLASSEXW *cls) {
  if (!cls || cls->cbSize != sizeof(*cls) || !cls->lpfnWndProc ||
      !cls->lpszClassName || !*cls->lpszClassName || procedure) return 0;
  name = copy_name(cls->lpszClassName);
  procedure = cls->lpfnWndProc;
  return 1;
}
HWND CreateWindowExW(DWORD, const wchar_t *className, const wchar_t *windowName,
                     DWORD style, int x, int y, int width, int height,
                     HWND parent, HMENU menu, HINSTANCE instance, void *param) {
  if (!canvas || live || !className || copy_name(className) != name || !procedure || parent)
    return nullptr;
  window.procedure = procedure;
  live = true;
  CREATESTRUCTW data{};
  data.lpCreateParams = param; data.hInstance = instance; data.hMenu = menu;
  data.hwndParent = parent; data.cy = height; data.cx = width; data.y = y; data.x = x;
  data.style = static_cast<LONG>(style); data.lpszName = windowName; data.lpszClass = className;
  if (window.procedure(&window, WM_CREATE, 0, reinterpret_cast<LPARAM>(&data)) == -1) {
    live = false; window.procedure = nullptr; return nullptr;
  }
  return live ? &window : nullptr;
}
BOOL DestroyWindow(HWND handle) {
  if (!live || handle != &window) return FALSE;
  live = false;
  active = false;
  focused = false;
  WNDPROC callback = window.procedure;
  window.procedure = nullptr;
  callback(handle, WM_DESTROY, 0, 0);
  return TRUE;
}

//	The input adapter's dispatch: the message queue's entries carry no
//	HWND (the single client window is implicit), so the queue hands the
//	message here and the registered procedure receives it.
extern "C" LRESULT pw153_window_dispatch(unsigned message, unsigned wParam, unsigned lParam) {
  if (!live || !window.procedure)
    return 0;
  return window.procedure(&window, message, wParam, lParam);
}
