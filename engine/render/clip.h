#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace pw::render {

struct VertexPC { float x, y, z; std::uint32_t argb; };
static_assert(sizeof(float) == 4);
static_assert(sizeof(VertexPC) == 16);
static_assert(offsetof(VertexPC, x) == 0);
static_assert(offsetof(VertexPC, argb) == 12);

using Matrix4 = std::array<float, 16>;
using Clip4 = std::array<float, 4>;
struct Viewport { std::uint32_t width, height; };
enum class RenderStatus { Ok, InvalidInput, UnsupportedFeature, ContextLost, GpuError };
struct DrawView {
    const VertexPC* vertices;
    std::uint32_t vertex_count;
    const std::uint16_t* indices;
    std::uint32_t index_count;
    Matrix4 matrix;
    Viewport viewport;
};

RenderStatus to_webgl_clip(const Matrix4&, const VertexPC&, Clip4& output) noexcept;
std::array<std::uint8_t, 4> argb_to_rgba(std::uint32_t) noexcept;
RenderStatus validate_draw(const DrawView&, Viewport maximum) noexcept;

} // namespace pw::render
