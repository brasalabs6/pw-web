#ifndef PW153_GNET_WEB_POLL_H
#define PW153_GNET_WEB_POLL_H

#include <emscripten/eventloop.h>

namespace pw153::platform {
inline int &web_poll_interval() {
  static int id = 0;
  return id;
}

inline bool start_web_poll(void (*callback)(void *), void *context) {
  if (!callback || web_poll_interval() != 0) return false;
  web_poll_interval() = emscripten_set_interval(callback, 10.0, context);
  return web_poll_interval() != 0;
}

inline void stop_web_poll() {
  if (web_poll_interval() != 0) {
    emscripten_clear_interval(web_poll_interval());
    web_poll_interval() = 0;
  }
}
} // namespace pw153::platform
#endif
