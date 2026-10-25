// Synthetic R0 pixel/lifecycle probe. Not an Angelica renderer or public wire ABI.
#include "clip.h"

#include <GLES3/gl3.h>
#include <emscripten/html5.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {
using pw::render::DrawView;
using pw::render::Matrix4;
using pw::render::RenderStatus;
using pw::render::VertexPC;
using pw::render::Viewport;

enum class State { Uninitialized, Ready, Lost };
struct GpuVertex {
    float x, y, z;
    std::uint8_t rgba[4];
};
static_assert(sizeof(GpuVertex) == 16);
static_assert(offsetof(GpuVertex, rgba) == 12);

struct Probe {
    State state = State::Uninitialized;
    bool restored = false;
    bool callbacks = false;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = 0;
    GLuint program = 0, vao = 0, vbo = 0, ebo = 0;
    GLint matrix_location = -1;
    Viewport maximum{0, 0};
    Viewport canvas{0, 0};
};
Probe probe;

void diagnostic(const char* action) {
    std::fprintf(stderr, "render-probe: %s\n", action);
}

void forget_objects() {
    probe.program = probe.vao = probe.vbo = probe.ebo = 0;
    probe.matrix_location = -1;
    probe.maximum = {0, 0};
    probe.canvas = {0, 0};
}

void release_objects() {
    if (probe.ebo) glDeleteBuffers(1, &probe.ebo);
    if (probe.vbo) glDeleteBuffers(1, &probe.vbo);
    if (probe.vao) glDeleteVertexArrays(1, &probe.vao);
    if (probe.program) glDeleteProgram(probe.program);
    forget_objects();
}

// A failed initialization must never leave an allocated context/callback or
// partially-created GPU objects behind. Keep it separate from user dispose.
void abort_initialization() {
    if (probe.context &&
        emscripten_webgl_make_context_current(probe.context) == EMSCRIPTEN_RESULT_SUCCESS)
        release_objects();
    else forget_objects();
    emscripten_set_webglcontextlost_callback("#canvas", nullptr, true, nullptr);
    emscripten_set_webglcontextrestored_callback("#canvas", nullptr, true, nullptr);
    probe.callbacks = false;
    if (probe.context) emscripten_webgl_destroy_context(probe.context);
    probe.context = 0;
    probe.restored = false;
    probe.state = State::Uninitialized;
}

EM_BOOL context_lost(int, const EmscriptenWebGLContextEvent*, void*) {
    probe.state = State::Lost;
    probe.restored = false;
    // GL objects have already been invalidated by the browser; never delete/reuse them.
    forget_objects();
    return EM_TRUE; // preventDefault, allowing WEBGL_lose_context.restoreContext().
}
EM_BOOL context_restored(int, const EmscriptenWebGLContextEvent*, void*) {
    if (probe.state == State::Lost) probe.restored = true;
    // Still Lost until the caller explicitly invokes probe_init().
    return EM_TRUE;
}

GLuint shader(GLenum type, const char* source) {
    const GLuint result = glCreateShader(type);
    if (!result) return 0;
    glShaderSource(result, 1, &source, nullptr);
    glCompileShader(result);
    GLint success = GL_FALSE;
    glGetShaderiv(result, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        diagnostic(type == GL_VERTEX_SHADER ? "vertex shader compilation failed"
                                           : "fragment shader compilation failed");
        glDeleteShader(result);
        return 0;
    }
    return result;
}

bool create_objects() {
    // The array is row-major for p_row * M; uploading it as column-major with
    // transpose=false produces transpose(M) and therefore M^T * p_column.
    constexpr const char* vertex_source = R"(#version 300 es
precision highp float;
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
uniform mat4 u_matrix;
out vec4 v_color;
void main() {
    vec4 clip_d3d = u_matrix * vec4(a_position, 1.0);
    gl_Position = vec4(clip_d3d.xy, 2.0 * clip_d3d.z - clip_d3d.w, clip_d3d.w);
    v_color = a_color;
}
)";
    constexpr const char* fragment_source = R"(#version 300 es
precision highp float;
in vec4 v_color;
out vec4 fragment_color;
void main() { fragment_color = v_color; }
)";
    GLuint vs = shader(GL_VERTEX_SHADER, vertex_source);
    if (!vs) return false;
    GLuint fs = shader(GL_FRAGMENT_SHADER, fragment_source);
    if (!fs) {
        glDeleteShader(vs);
        return false;
    }
    probe.program = glCreateProgram();
    if (probe.program) {
        glAttachShader(probe.program, vs);
        glAttachShader(probe.program, fs);
        glLinkProgram(probe.program);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!probe.program) {
        diagnostic("glCreateProgram failed");
        return false;
    }
    GLint linked = GL_FALSE;
    glGetProgramiv(probe.program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        diagnostic("shader link failed");
        return false;
    }
    probe.matrix_location = glGetUniformLocation(probe.program, "u_matrix");
    if (probe.matrix_location < 0) {
        diagnostic("u_matrix not found");
        return false;
    }
    glGenVertexArrays(1, &probe.vao);
    glGenBuffers(1, &probe.vbo);
    glGenBuffers(1, &probe.ebo);
    if (!probe.vao || !probe.vbo || !probe.ebo) {
        diagnostic("VAO or buffer allocation failed");
        return false;
    }
    glBindVertexArray(probe.vao);
    glBindBuffer(GL_ARRAY_BUFFER, probe.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, probe.ebo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GpuVertex),
                          reinterpret_cast<const void*>(offsetof(GpuVertex, x)));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GpuVertex),
                          reinterpret_cast<const void*>(offsetof(GpuVertex, rgba)));
    glBindVertexArray(0);
    return glGetError() == GL_NO_ERROR;
}

RenderStatus active() {
    if (probe.state == State::Lost) return RenderStatus::ContextLost;
    if (probe.state != State::Ready) return RenderStatus::GpuError;
    if (emscripten_webgl_make_context_current(probe.context) != EMSCRIPTEN_RESULT_SUCCESS)
        return RenderStatus::GpuError;
    return RenderStatus::Ok;
}

constexpr Matrix4 identity{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
constexpr std::uint16_t triangle[]{0,1,2};
constexpr std::uint16_t bad_triangle[]{0,1,3};

RenderStatus draw(const DrawView& view) {
    const auto status = active();
    if (status != RenderStatus::Ok) return status;
    // Validate *before* changing any GL state, buffers or framebuffer.
    if (pw::render::validate_draw(view, probe.maximum) != RenderStatus::Ok)
        return RenderStatus::InvalidInput;

    std::vector<GpuVertex> packed;
    packed.reserve(view.vertex_count);
    for (std::uint32_t i = 0; i < view.vertex_count; ++i) {
        const auto& v = view.vertices[i];
        const auto rgba = pw::render::argb_to_rgba(v.argb);
        packed.push_back({v.x, v.y, v.z, {rgba[0], rgba[1], rgba[2], rgba[3]}});
    }
    glViewport(0, 0, static_cast<GLsizei>(view.viewport.width),
               static_cast<GLsizei>(view.viewport.height));
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    glUseProgram(probe.program);
    glUniformMatrix4fv(probe.matrix_location, 1, GL_FALSE, view.matrix.data());
    glBindVertexArray(probe.vao);
    glBindBuffer(GL_ARRAY_BUFFER, probe.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(packed.size() * sizeof(GpuVertex)),
                 packed.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, probe.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(view.index_count * sizeof(std::uint16_t)),
                 view.indices, GL_STREAM_DRAW);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(view.index_count),
                   GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
    if (glGetError() != GL_NO_ERROR) {
        diagnostic("drawIndexed GPU error");
        return RenderStatus::GpuError;
    }
    return RenderStatus::Ok;
}

RenderStatus triangle_draw(std::uint32_t argb, float z, const Matrix4& matrix,
                           const std::uint16_t* idx = triangle,
                           Viewport viewport = {64,64}) {
    const VertexPC vertices[]{{-0.5F,-0.5F,z,argb},
                              { 0.5F,-0.5F,z,argb},
                              { 0.0F, 0.5F,z,argb}};
    return draw({vertices, 3, idx, 3, matrix, viewport});
}
int code(RenderStatus status) { return static_cast<int>(status); }
} // namespace

extern "C" {

int probe_init() {
    if (probe.state == State::Ready) return code(RenderStatus::Ok);
    if (probe.state == State::Lost && !probe.restored)
        return code(RenderStatus::ContextLost);

    if (!probe.context) {
        EmscriptenWebGLContextAttributes attrs;
        emscripten_webgl_init_context_attributes(&attrs);
        attrs.majorVersion = 2;
        attrs.minorVersion = 0;
        attrs.depth = EM_TRUE;
        attrs.alpha = EM_TRUE;
        attrs.stencil = EM_FALSE;
        attrs.antialias = EM_FALSE;
        attrs.preserveDrawingBuffer = EM_TRUE; // deterministic asynchronous readback
        probe.context = emscripten_webgl_create_context("#canvas", &attrs);
        if (probe.context <= 0) {
            probe.context = 0;
            diagnostic("WebGL2 context creation failed");
            return code(RenderStatus::GpuError);
        }
    }
    if (emscripten_webgl_make_context_current(probe.context) != EMSCRIPTEN_RESULT_SUCCESS) {
        diagnostic("WebGL2 make-current failed");
        abort_initialization();
        return code(RenderStatus::GpuError);
    }
    if (!probe.callbacks) {
        const auto lost = emscripten_set_webglcontextlost_callback(
            "#canvas", nullptr, true, context_lost);
        const auto restored = emscripten_set_webglcontextrestored_callback(
            "#canvas", nullptr, true, context_restored);
        if (lost != EMSCRIPTEN_RESULT_SUCCESS ||
            restored != EMSCRIPTEN_RESULT_SUCCESS) {
            diagnostic("context callbacks unavailable");
            abort_initialization();
            return code(RenderStatus::GpuError);
        }
        probe.callbacks = true;
    }

    int width = 0, height = 0;
    GLint dims[2] = {0, 0};
    emscripten_get_canvas_element_size("#canvas", &width, &height);
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
    if (width < 1 || height < 1 || dims[0] < width || dims[1] < height ||
        glGetError() != GL_NO_ERROR) {
        diagnostic("canvas exceeds GL_MAX_VIEWPORT_DIMS or is invalid");
        abort_initialization();
        return code(RenderStatus::InvalidInput);
    }
    probe.canvas = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
    probe.maximum = {static_cast<std::uint32_t>(std::min(width, dims[0])),
                     static_cast<std::uint32_t>(std::min(height, dims[1]))};
    if (!create_objects()) {
        diagnostic("partial GPU initialization failed");
        abort_initialization();
        return code(RenderStatus::GpuError);
    }
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);
    glClearDepthf(1.0F);
    glDepthMask(GL_TRUE);
    if (glGetError() != GL_NO_ERROR) {
        diagnostic("initial render state failed");
        abort_initialization();
        return code(RenderStatus::GpuError);
    }
    probe.state = State::Ready;
    probe.restored = false;
    return code(RenderStatus::Ok);
}

int probe_clear() {
    const auto status = active();
    if (status != RenderStatus::Ok) return code(status);
    glViewport(0, 0, static_cast<GLsizei>(probe.canvas.width),
               static_cast<GLsizei>(probe.canvas.height));
    glClearColor(0, 0, 0, 1);
    glClearDepthf(1.0F);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (glGetError() != GL_NO_ERROR) {
        diagnostic("clear GPU error");
        return code(RenderStatus::GpuError);
    }
    return code(RenderStatus::Ok);
}

int probe_draw(int fixture) {
    if (const auto status = active(); status != RenderStatus::Ok) return code(status);
    if (fixture < 0 || fixture > 7) return code(RenderStatus::InvalidInput);
    if (fixture == 5) return code(RenderStatus::UnsupportedFeature);
    if (fixture == 3) return code(triangle_draw(0xffffffffU, .5F, identity, bad_triangle));
    if (fixture == 4) return code(triangle_draw(0xffffffffU, .5F, identity,
                                                triangle, {0,64}));
    if (fixture == 1 || fixture == 2) {
        // Two separate draws share a depth buffer. Each is validated before upload.
        const bool near_first = fixture == 1;
        auto first = triangle_draw(near_first ? 0xffff0000U : 0xff00ff00U,
                                   near_first ? .25F : .75F, identity);
        if (first != RenderStatus::Ok) return code(first);
        return code(triangle_draw(near_first ? 0xff00ff00U : 0xffff0000U,
                                  near_first ? .75F : .25F, identity));
    }
    if (fixture == 6) return code(triangle_draw(0x80402010U, .5F, identity));
    Matrix4 transform = identity;
    if (fixture == 7) transform[12] = .5F;
    return code(triangle_draw(0xffffffffU, .5F, transform));
}

std::uint32_t probe_pixel(int x, int y) {
    if (active() != RenderStatus::Ok || x < 0 || y < 0 ||
        static_cast<std::uint32_t>(x) >= probe.canvas.width ||
        static_cast<std::uint32_t>(y) >= probe.canvas.height) return 0;
    std::uint8_t pixel[4] = {};
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    if (glGetError() != GL_NO_ERROR) {
        diagnostic("readPixels GPU error");
        return 0;
    }
    return (std::uint32_t(pixel[0]) << 24) | (std::uint32_t(pixel[1]) << 16) |
           (std::uint32_t(pixel[2]) << 8) | std::uint32_t(pixel[3]);
}

void probe_dispose() {
    if (probe.context && probe.state == State::Ready &&
        emscripten_webgl_make_context_current(probe.context) == EMSCRIPTEN_RESULT_SUCCESS)
        release_objects();
    else forget_objects();
    if (probe.callbacks) {
        emscripten_set_webglcontextlost_callback("#canvas", nullptr, true, nullptr);
        emscripten_set_webglcontextrestored_callback("#canvas", nullptr, true, nullptr);
        probe.callbacks = false;
    }
    if (probe.context) emscripten_webgl_destroy_context(probe.context);
    probe.context = 0;
    probe.state = State::Uninitialized;
    probe.restored = false;
}
} // extern "C"
