#include "ATime.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
int main() {
  static_assert(sizeof(DWORD) == 4);
  static_assert(sizeof(WORD) == 2);
  static_assert(sizeof(LONG) == 4);
  DWORD before = a_GetTime();
  const auto start = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start <
         std::chrono::milliseconds(5)) {
  }
  DWORD after = a_GetTime();
  CHECK(DWORD(after - before) >= 1);
  CHECK(DWORD(after - before) < 10000);
  for (int i = 0; i < 1000; i++) {
    DWORD now = a_GetTime();
    CHECK(DWORD(now - after) < 10000);
    after = now;
  }
  WORD year, month, day, weekday, hour, minute, second;
  a_GetSystemTime(&year, &month, &day, &weekday, &hour, &minute, &second);
  CHECK(year >= 1970);
  CHECK(month >= 1 && month <= 12);
  CHECK(day >= 1 && day <= 31);
  CHECK(weekday <= 6);
  CHECK(hour <= 23);
  CHECK(minute <= 59);
  CHECK(second <= 60);
  a_GetSystemTime(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                  nullptr);
  std::cout << "PASS actual monotonic elapsed time, fixed-width ABI, local "
               "calendar and optional outputs\n";
}
