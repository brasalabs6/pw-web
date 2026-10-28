#include "webgl_caps.h"
#include <emscripten/html5.h>
#include <emscripten.h>
#include <stdexcept>
#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(#condition); } while (false)
int main() {
  const char *error = nullptr;
  CHECK(!pw153_check_webgl_capabilities(&error));
  CHECK(error && *error);
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  auto context = emscripten_webgl_create_context("#canvas", &attrs);
  CHECK(context > 0);
  CHECK(emscripten_webgl_make_context_current(context) == EMSCRIPTEN_RESULT_SUCCESS);
  CHECK(pw153_check_webgl_capabilities(&error));
  CHECK(error == nullptr);
  emscripten_webgl_destroy_context(context);
  EM_ASM({ window.__capsTestPassed = true; });
  return 0;
}
