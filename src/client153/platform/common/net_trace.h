/*
 * FILE: net_trace.h (platform seam)
 *
 * DESCRIPTION: A small ring buffer for the browser network bring-up
 * traces. The DevTools console drops history under the boot's message
 * rate, so the GNET poll/recv traces land here instead and are dumped
 * on demand through the pw153_net_trace_dump export.
 */

#ifndef PW153_PLATFORM_NET_TRACE_H
#define PW153_PLATFORM_NET_TRACE_H

#include <cstdio>
#include <cstdarg>

namespace pw153::nettrace {

constexpr int kCapacity = 128;
constexpr int kLineSize = 192;
inline char lines[kCapacity][kLineSize];
inline int total = 0;

inline void push(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(lines[total % kCapacity], kLineSize, fmt, ap);
  va_end(ap);
  ++total;
}

}  // namespace pw153::nettrace

// The on-demand dump lives in gnstatic.cpp (a single definition the
// entry exports).
extern "C" int pw153_net_trace_dump(void);

#endif // PW153_PLATFORM_NET_TRACE_H
