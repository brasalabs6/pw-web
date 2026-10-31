#include "Windows.h"
#include <cstddef>
#include <type_traits>

static_assert(std::is_same<LPRECT, RECT *>::value, "LPRECT");
static_assert(std::is_same<COLORREF, DWORD>::value, "COLORREF");
static_assert(std::is_same<WNDENUMPROC, BOOL (*)(HWND, LPARAM)>::value, "WNDENUMPROC");
static_assert(offsetof(GUITHREADINFO, rcCaret) == 8 + 6 * sizeof(HWND), "GUITHREADINFO layout");
static_assert(sizeof(GUITHREADINFO) == 24 + 6 * sizeof(HWND), "GUITHREADINFO size");
int main() {
  RECT a{0, 0, 10, 10}, b{5, 2, 12, 8}, result{};
  if (!IntersectRect(&result, &a, &b) || result.left != 5 || result.top != 2 ||
      result.right != 10 || result.bottom != 8) return 1;
  RECT edge{10, 0, 20, 10};
  if (IntersectRect(&result, &a, &edge) || result.left || result.top ||
      result.right || result.bottom) return 2;
  if (!IntersectRect(&a, &a, &b) || a.left != 5 || a.right != 10) return 3;
}
