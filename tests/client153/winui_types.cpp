#include "Windows.h"
#include <cstddef>
#include <type_traits>

static_assert(std::is_same<LPRECT, RECT *>::value, "LPRECT");
static_assert(std::is_same<COLORREF, DWORD>::value, "COLORREF");
static_assert(std::is_same<WNDENUMPROC, BOOL (*)(HWND, LPARAM)>::value, "WNDENUMPROC");
static_assert(offsetof(GUITHREADINFO, rcCaret) == 8 + 6 * sizeof(HWND), "GUITHREADINFO layout");
static_assert(sizeof(GUITHREADINFO) == 24 + 6 * sizeof(HWND), "GUITHREADINFO size");
int main() {}
