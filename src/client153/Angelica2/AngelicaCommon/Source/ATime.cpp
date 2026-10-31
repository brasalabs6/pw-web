/* Adapted from Angelica Common ATime.cpp, source revision in
 * source-manifest.json. Copyright (c) 2001 Archosaur Studio, All Rights
 * Reserved. Platform clock operations are owned by platform/common/clock.cpp.
 */
#include "ATime.h"
#include "clock.h"
void a_GetSystemTime(WORD *year, WORD *month, WORD *day, WORD *weekday,
                     WORD *hour, WORD *minute, WORD *second) {
  const auto value = pw153::platform::local_calendar();
  if (year)
    *year = value.year;
  if (month)
    *month = value.month;
  if (day)
    *day = value.day;
  if (weekday)
    *weekday = value.weekday;
  if (hour)
    *hour = value.hour;
  if (minute)
    *minute = value.minute;
  if (second)
    *second = value.second;
}
DWORD a_GetTime() {
  return static_cast<DWORD>(pw153::platform::monotonic_microseconds() / 1000);
}
