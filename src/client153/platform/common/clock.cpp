// Browser/native C++ platform backend. No Win32 time or CPU-counter emulation.
#include "clock.h"
#include <chrono>
#include <ctime>
#include <stdexcept>
namespace pw153::platform {
std::uint64_t monotonic_microseconds() {
  using Clock = std::chrono::steady_clock;
  static_assert(Clock::is_steady);
  static const auto origin = Clock::now();
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() -
                                                            origin)
          .count());
}
Calendar local_calendar() {
  const auto now = std::time(nullptr);
  std::tm value{};
  if (now == std::time_t(-1) || !localtime_r(&now, &value))
    throw std::runtime_error("local calendar unavailable");
  return {static_cast<std::uint16_t>(value.tm_year + 1900),
          static_cast<std::uint16_t>(value.tm_mon + 1),
          static_cast<std::uint16_t>(value.tm_mday),
          static_cast<std::uint16_t>(value.tm_wday),
          static_cast<std::uint16_t>(value.tm_hour),
          static_cast<std::uint16_t>(value.tm_min),
          static_cast<std::uint16_t>(value.tm_sec)};
}
} // namespace pw153::platform
