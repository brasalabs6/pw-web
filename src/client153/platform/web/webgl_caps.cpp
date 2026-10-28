#include "webgl_caps.h"
#include <GLES3/gl3.h>
#include <emscripten/html5.h>

bool pw153_check_webgl_capabilities(const char **error) {
  if (error) *error = nullptr;
  auto fail = [error](const char *message) {
    if (error) *error = message;
    return false;
  };
  const auto context = emscripten_webgl_get_current_context();
  if (context <= 0) return fail("WebGL2 context is not current");
  EmscriptenWebGLContextAttributes attributes;
  if (emscripten_webgl_get_context_attributes(context, &attributes) != EMSCRIPTEN_RESULT_SUCCESS ||
      attributes.majorVersion < 2) return fail("WebGL2 context is required");
  GLint textureUnits = 0;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureUnits);
  if (textureUnits < 2) return fail("At least two texture units are required");
  if (!emscripten_webgl_enable_extension(context, "WEBGL_compressed_texture_s3tc"))
    return fail("S3TC texture support is required");
  return true;
}
