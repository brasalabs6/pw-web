// Browser platform adapter for the portable original Angelica resource loaders.
#include "dds.h"
#include "mox.h"
#include <GLES3/gl3.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <emscripten/html5.h>
#include <limits>
#include <string>
#include <vector>
namespace {
pw153::Model model;
std::string error;
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = 0;
bool ready = false, lost = false;
GLuint program = 0;
struct GpuMesh {
  GLuint vao = 0, vertices = 0, indices = 0, texture = 0;
  bool textured = false;
};
std::vector<GpuMesh> gpu;
std::array<float, 3> center{};
float scale = 1;
void release_meshes() {
  for (auto &m : gpu) {
    glDeleteVertexArrays(1, &m.vao);
    glDeleteBuffers(1, &m.vertices);
    glDeleteBuffers(1, &m.indices);
    glDeleteTextures(1, &m.texture);
  }
  gpu.clear();
}
int fail(const std::string &message) {
  error = message;
  return 0;
}
EM_BOOL on_lost(int, const void *, void *) {
  lost = true;
  ready = false;
  gpu.clear();
  program = 0;
  return EM_TRUE;
}
EM_BOOL on_restored(int, const void *, void *) {
  lost = false;
  return EM_FALSE;
}
GLuint shader(GLenum type, const char *source) {
  auto id = glCreateShader(type);
  glShaderSource(id, 1, &source, nullptr);
  glCompileShader(id);
  GLint ok = 0;
  glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[2048];
    glGetShaderInfoLog(id, sizeof(log), nullptr, log);
    error = log;
    glDeleteShader(id);
    return 0;
  }
  return id;
}
pw153::Matrix multiply(const pw153::Matrix &a, const pw153::Matrix &b) {
  pw153::Matrix c{};
  for (int row = 0; row < 4; row++)
    for (int col = 0; col < 4; col++)
      for (int k = 0; k < 4; k++)
        c[row * 4 + col] += a[row * 4 + k] * b[k * 4 + col];
  return c;
}
} // namespace
extern "C" {
const char *client_error() { return error.c_str(); }
int client_init() {
  error.clear();
  if (lost)
    return fail("WebGL context is lost");
  if (ready)
    return 1;
  if (!context) {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.depth = true;
    attrs.antialias = false;
    attrs.preserveDrawingBuffer = true;
    context = emscripten_webgl_create_context("#canvas", &attrs);
  }
  if (context <= 0 || emscripten_webgl_make_context_current(context) !=
                          EMSCRIPTEN_RESULT_SUCCESS)
    return fail("WebGL2 context unavailable");
  emscripten_set_webglcontextlost_callback("#canvas", nullptr, true, on_lost);
  emscripten_set_webglcontextrestored_callback("#canvas", nullptr, true,
                                               on_restored);
  if (!emscripten_webgl_enable_extension(context,
                                         "WEBGL_compressed_texture_s3tc"))
    return fail("S3TC extension required for original DXT textures");
  const char *vs = R"(#version 300 es
 precision highp float;
 layout(location=0) in vec3 position;
 layout(location=1) in vec2 uv;
 uniform vec3 center;
 uniform float modelScale;
 uniform vec4 camera;
 out vec2 texcoord;
 void main(){
  vec3 p=(position-center)*modelScale;
  float cy=cos(camera.x),sy=sin(camera.x),cx=cos(camera.y),sx=sin(camera.y);
  p=vec3(cy*p.x+sy*p.z,p.y,-sy*p.x+cy*p.z);
  p=vec3(p.x,cx*p.y-sx*p.z,sx*p.y+cx*p.z);
  p.z-=camera.z;
  float f=2.41421356237;
  gl_Position=vec4(p.x*f/camera.w,p.y*f,-1.002002*p.z-0.2002002,-p.z);
  texcoord=uv;
 })";
  const char *fs = R"(#version 300 es
 precision highp float;
 in vec2 texcoord;
 uniform sampler2D image;
 out vec4 color;
 void main(){color=texture(image,texcoord);})";
  auto v = shader(GL_VERTEX_SHADER, vs);
  if (!v)
    return 0;
  auto f = shader(GL_FRAGMENT_SHADER, fs);
  if (!f) {
    glDeleteShader(v);
    return 0;
  }
  program = glCreateProgram();
  glAttachShader(program, v);
  glAttachShader(program, f);
  glLinkProgram(program);
  glDeleteShader(v);
  glDeleteShader(f);
  GLint ok = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (!ok) {
    glDeleteProgram(program);
    program = 0;
    return fail("Angelica resource shader link failed");
  }
  ready = true;
  return 1;
}
int client_load(const unsigned char *bytes, unsigned length) {
  if (!ready || lost)
    return fail("renderer not ready");
  pw153::Model next;
  if (!pw153::load_mox(bytes, length, next, error))
    return 0;
  for (const auto &mesh : next.meshes)
    if (!mesh.detail_texture.empty() || !mesh.lightmap.empty())
      return fail("detail/lightmap shader not implemented for this model");
  release_meshes();
  model = std::move(next);
  std::vector<pw153::Matrix> worlds;
  for (const auto &frame : model.frames) {
    auto world = frame.transforms[0];
    if (frame.parent >= 0)
      world = multiply(world, worlds.at(frame.parent));
    worlds.push_back(world);
  }
  std::array<float, 3> minimum{INFINITY, INFINITY, INFINITY},
      maximum{-INFINITY, -INFINITY, -INFINITY};
  for (const auto &mesh : model.meshes) {
    auto vertices = mesh.vertices[0];
    const auto &m = worlds.at(mesh.frame);
    for (auto &v : vertices) {
      const auto x = v.x, y = v.y, z = v.z;
      v.x = x * m[0] + y * m[4] + z * m[8] + m[12];
      v.y = x * m[1] + y * m[5] + z * m[9] + m[13];
      v.z = x * m[2] + y * m[6] + z * m[10] + m[14];
      const float p[]{v.x, v.y, v.z};
      for (int i = 0; i < 3; i++) {
        if (!std::isfinite(p[i]))
          return fail("world transform overflow");
        minimum[i] = std::min(minimum[i], p[i]);
        maximum[i] = std::max(maximum[i], p[i]);
      }
    }
    GpuMesh g;
    glGenVertexArrays(1, &g.vao);
    glBindVertexArray(g.vao);
    glGenBuffers(1, &g.vertices);
    glGenBuffers(1, &g.indices);
    glGenTextures(1, &g.texture);
    if (!g.vao || !g.vertices || !g.indices || !g.texture) {
      gpu.push_back(g);
      release_meshes();
      model = {};
      return fail("mesh GPU allocation failed");
    }
    glBindBuffer(GL_ARRAY_BUFFER, g.vertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(pw153::Vertex),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g.indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.size() * sizeof(std::uint16_t),
                 mesh.indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(pw153::Vertex),
                          nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(pw153::Vertex),
                          reinterpret_cast<const void *>(24));
    gpu.push_back(g);
  }
  if (gpu.empty())
    return fail("model contains no drawable geometry");
  float extent = 0;
  for (int i = 0; i < 3; i++) {
    center[i] = (minimum[i] + maximum[i]) * .5f;
    extent = std::max(extent, maximum[i] - minimum[i]);
  }
  if (!std::isfinite(extent) || extent <= 0)
    return fail("invalid model bounds");
  scale = 2.f / extent;
  glBindVertexArray(0);
  if (glGetError() != GL_NO_ERROR)
    return fail("mesh GPU upload failed");
  return int(gpu.size());
}
int client_mesh_count() { return int(model.meshes.size()); }
const char *client_texture_name(int mesh) {
  if (mesh < 0 || std::size_t(mesh) >= model.meshes.size())
    return nullptr;
  return model.meshes[mesh].texture.c_str();
}
int client_texture_name_size(int mesh) {
  if (mesh < 0 || std::size_t(mesh) >= model.meshes.size())
    return 0;
  return int(model.meshes[mesh].texture.size());
}
int client_texture(int index, const unsigned char *data, unsigned length) {
  if (!ready || lost || index < 0 || std::size_t(index) >= gpu.size())
    return fail("invalid texture owner/context");
  pw153::Dds dds;
  if (!pw153::load_dds(data, length, dds, error))
    return 0;
  GLint limit = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limit);
  if (dds.width > unsigned(limit) || dds.height > unsigned(limit))
    return fail("texture exceeds GPU size limit");
  glBindTexture(GL_TEXTURE_2D, gpu[index].texture);
  for (unsigned level = 0; level < dds.levels.size(); level++) {
    const auto &l = dds.levels[level];
    glCompressedTexImage2D(GL_TEXTURE_2D, level, dds.gl_format, l.width,
                           l.height, 0, l.bytes, data + l.offset);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,
                  int(dds.levels.size()) - 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  dds.levels.size() > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (glGetError() != GL_NO_ERROR)
    return fail("original DDS GPU upload failed");
  gpu[index].textured = true;
  return int(dds.levels.size());
}
int client_draw(float yaw, float pitch, float distance, int width, int height) {
  if (!ready || lost || gpu.empty() || width < 1 || height < 1 ||
      !std::isfinite(yaw) || !std::isfinite(pitch) ||
      !std::isfinite(distance) || distance < 2)
    return fail("invalid render state/camera");
  for (const auto &g : gpu)
    if (!g.textured)
      return fail("mesh texture not loaded");
  glViewport(0, 0, width, height);
  glClearColor(.06f, .075f, .10f, 1);
  glClearDepthf(1);
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(program);
  glUniform3fv(glGetUniformLocation(program, "center"), 1, center.data());
  glUniform1f(glGetUniformLocation(program, "modelScale"), scale);
  glUniform4f(glGetUniformLocation(program, "camera"), yaw, pitch, distance,
              float(width) / height);
  glUniform1i(glGetUniformLocation(program, "image"), 0);
  glActiveTexture(GL_TEXTURE0);
  for (std::size_t i = 0; i < gpu.size(); i++) {
    if (model.meshes[i].material.two_sided)
      glDisable(GL_CULL_FACE);
    else {
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CW);
      glCullFace(GL_BACK);
    }
    glBindTexture(GL_TEXTURE_2D, gpu[i].texture);
    glBindVertexArray(gpu[i].vao);
    glDrawElements(model.meshes[i].wire ? GL_LINES : GL_TRIANGLES,
                   model.meshes[i].indices.size(), GL_UNSIGNED_SHORT, nullptr);
  }
  glBindVertexArray(0);
  if (glGetError() != GL_NO_ERROR)
    return fail("original model draw failed");
  return int(gpu.size());
}
void client_dispose() {
  if (ready && !lost) {
    release_meshes();
    if (program)
      glDeleteProgram(program);
  }
  gpu.clear();
  program = 0;
  ready = false;
  model = {};
}
}
