#pragma once
#include <cstdint>
namespace pw153::platform {
struct Calendar {
  std::uint16_t year, month, day, weekday, hour, minute, second, millisecond;
};
std::uint64_t monotonic_microseconds();
Calendar local_calendar();
} // namespace pw153::platform
