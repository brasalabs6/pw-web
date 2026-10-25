#include "clip.h"

#include <cmath>

namespace pw::render {

RenderStatus to_webgl_clip(const Matrix4& matrix, const VertexPC& vertex,
                          Clip4& output) noexcept {
    if (!std::isfinite(vertex.x) || !std::isfinite(vertex.y) || !std::isfinite(vertex.z))
        return RenderStatus::InvalidInput;
    for (float value : matrix)
        if (!std::isfinite(value)) return RenderStatus::InvalidInput;

    Clip4 clip{};
    const Clip4 point{vertex.x, vertex.y, vertex.z, 1};
    for (std::size_t j = 0; j < 4; ++j) {
        for (std::size_t i = 0; i < 4; ++i)
            clip[j] += point[i] * matrix[i * 4 + j];
    }
    clip[2] = 2 * clip[2] - clip[3];
    for (float value : clip)
        if (!std::isfinite(value)) return RenderStatus::InvalidInput;
    output = clip;
    return RenderStatus::Ok;
}

std::array<std::uint8_t, 4> argb_to_rgba(std::uint32_t argb) noexcept {
    return {static_cast<std::uint8_t>((argb >> 16) & 0xffU),
            static_cast<std::uint8_t>((argb >> 8) & 0xffU),
            static_cast<std::uint8_t>(argb & 0xffU),
            static_cast<std::uint8_t>(argb >> 24)};
}

RenderStatus validate_draw(const DrawView& draw, Viewport maximum) noexcept {
    if (!draw.vertices || !draw.indices ||
        draw.vertex_count < 1 || draw.vertex_count > 65536 ||
        draw.index_count < 3 || draw.index_count > 196608 || draw.index_count % 3 != 0 ||
        maximum.width == 0 || maximum.height == 0 ||
        draw.viewport.width == 0 || draw.viewport.height == 0 ||
        draw.viewport.width > maximum.width || draw.viewport.height > maximum.height)
        return RenderStatus::InvalidInput;

    Clip4 clip{};
    for (std::uint32_t i = 0; i < draw.vertex_count; ++i)
        if (to_webgl_clip(draw.matrix, draw.vertices[i], clip) != RenderStatus::Ok)
            return RenderStatus::InvalidInput;
    for (std::uint32_t i = 0; i < draw.index_count; ++i)
        if (draw.indices[i] >= draw.vertex_count) return RenderStatus::InvalidInput;
    return RenderStatus::Ok;
}

} // namespace pw::render
