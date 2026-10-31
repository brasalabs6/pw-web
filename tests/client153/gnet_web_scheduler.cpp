#include "gnet_web_poll.h"
#include <emscripten/eventloop.h>
#include <emscripten/emscripten.h>
#include <cstdlib>
#include <cstdio>

static int calls = 0;
static void tick(void*) { ++calls; }
static void verify(void*) {
  if (calls < 2) std::exit(1);
  pw153::platform::stop_web_poll();
  const int at_stop = calls;
  emscripten_set_timeout([](void* data) {
    const int expected = *static_cast<int*>(data);
    delete static_cast<int*>(data);
    if (calls != expected) std::exit(2);
    std::puts("web poll callbacks stopped");
    emscripten_force_exit(0);
  }, 50, new int(at_stop));
}
int main() {
  if (!pw153::platform::start_web_poll(tick, nullptr)) return 3;
  if (pw153::platform::start_web_poll(tick, nullptr)) return 4;
  emscripten_set_timeout(verify, 80, nullptr);
  emscripten_exit_with_live_runtime();
}
