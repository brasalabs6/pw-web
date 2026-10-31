#include "clip.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

using namespace pw::render;

namespace {
int cases = 0;
void require(bool ok, const char* message) {
    if (!ok) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}
const Matrix4 identity{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
const Viewport maximum{4096, 2048};
const VertexPC vertices[]{{-0.5F,-0.5F,0.5F,0xffffffffU},
                          {0.5F,-0.5F,0.5F,0xffffffffU},
                          {0,0.5F,0.5F,0xffffffffU}};
const std::uint16_t indices[]{0,1,2};
DrawView valid() { return {vertices,3,indices,3,identity,{640,480}}; }
void clip(const Matrix4& matrix, const VertexPC& vertex, const Clip4& expected) {
    Clip4 output{9,9,9,9};
    require(to_webgl_clip(matrix, vertex, output) == RenderStatus::Ok, "clip status");
    for (std::size_t i = 0; i < 4; ++i)
        require(std::abs(output[i] - expected[i]) <= 1e-6F, "clip value");
}
void rejects_clip(const Matrix4& matrix, const VertexPC& vertex) {
    Clip4 output{9,8,7,6};
    require(to_webgl_clip(matrix, vertex, output) == RenderStatus::InvalidInput,
            "invalid transform accepted");
    require(output == Clip4{9,8,7,6}, "output changed on error");
}
void rejects(const DrawView& draw, Viewport limit = maximum) {
    require(validate_draw(draw, limit) == RenderStatus::InvalidInput, "invalid draw accepted");
}
}

int main() {
    clip(identity, {0,0,0,0}, {0,0,-1,1});
    ++cases;

    clip(identity, {0,0,0.5F,0}, {0,0,0,1});
    clip(identity, {0,0,1,0}, {0,0,1,1});
    ++cases;

    Matrix4 perspective = identity;
    perspective[15] = 2;
    clip(perspective, {1,2,0.75F,0}, {1,2,-0.5F,2});
    // A non-symmetric matrix also detects an accidental transpose.
    clip({1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15,16},
         {1,2,3,0}, {51,58,58,72});
    ++cases;

    Matrix4 translated = identity;
    translated[12] = 3; translated[13] = 4; translated[14] = 5;
    clip(translated, {1,2,3,0}, {4,6,15,1});
    ++cases;

    require(argb_to_rgba(0x80402010U) == std::array<std::uint8_t,4>{64,32,16,128}, "ARGB");
    require(argb_to_rgba(0) == std::array<std::uint8_t,4>{0,0,0,0}, "transparent black");
    require(argb_to_rgba(0xffffffffU) == std::array<std::uint8_t,4>{255,255,255,255}, "white");
    require(argb_to_rgba(0x00ff0000U) == std::array<std::uint8_t,4>{255,0,0,0}, "red channel");
    ++cases;

    require(validate_draw(valid(), maximum) == RenderStatus::Ok, "triangle");
    ++cases;

    auto draw = valid(); draw.vertices = nullptr; rejects(draw);
    draw = valid(); draw.indices = nullptr; rejects(draw);
    ++cases;

    for (const auto count : {0U,65537U}) { draw = valid(); draw.vertex_count = count; rejects(draw); }
    for (const auto count : {0U,1U,2U,4U,196609U,196611U}) {
        draw = valid(); draw.index_count = count; rejects(draw);
    }
    ++cases;

    const std::uint16_t bad_indices[]{0,1,3};
    draw = valid(); draw.indices = bad_indices; rejects(draw);
    const std::uint16_t last_bad[]{0,1,65535};
    draw.indices = last_bad; rejects(draw);
    ++cases;

    for (const float bad : {std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::infinity(),
                            -std::numeric_limits<float>::infinity()}) {
        for (std::size_t i = 0; i < 16; ++i) {
            draw = valid(); draw.matrix[i] = bad;
            rejects(draw); rejects_clip(draw.matrix, vertices[0]);
        }
    }
    ++cases;

    for (const float bad : {std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::infinity()}) {
        for (int coordinate = 0; coordinate < 3; ++coordinate) {
            VertexPC vertex = vertices[0];
            if (coordinate == 0) vertex.x = bad;
            if (coordinate == 1) vertex.y = bad;
            if (coordinate == 2) vertex.z = bad;
            rejects_clip(identity, vertex);
            VertexPC invalid_vertices[]{vertices[0],vertices[1],vertex};
            draw = valid(); draw.vertices = invalid_vertices; rejects(draw);
        }
    }
    ++cases;

    const float largest = std::numeric_limits<float>::max();
    Matrix4 overflow = identity; overflow[0] = largest;
    rejects_clip(overflow, {2,0,0,0});
    rejects_clip(identity, {0,0,largest,0}); // clip depth conversion overflows
    VertexPC overflow_vertices[]{vertices[0],vertices[1],{0,0,largest,0}};
    draw = valid(); draw.vertices = overflow_vertices; rejects(draw);
    ++cases;

    for (const Viewport viewport : {Viewport{0,1},{1,0},{4097,1},{1,2049}}) {
        draw = valid(); draw.viewport = viewport; rejects(draw);
    }
    rejects(valid(), {0,2048}); rejects(valid(), {4096,0});
    draw = valid(); draw.viewport = {1,1};
    require(validate_draw(draw, maximum) == RenderStatus::Ok, "minimum viewport");
    draw.viewport = maximum;
    require(validate_draw(draw, maximum) == RenderStatus::Ok, "maximum viewport");
    ++cases;

    std::vector<VertexPC> many_vertices(65536, vertices[0]);
    std::vector<std::uint16_t> many_indices(196608, 65535);
    draw = valid(); draw.vertices = many_vertices.data(); draw.vertex_count = 65536;
    draw.indices = many_indices.data(); draw.index_count = 196608;
    require(validate_draw(draw, maximum) == RenderStatus::Ok, "maximum counts and index");
    draw.vertex_count = 65535; rejects(draw);
    draw.vertex_count = 1; draw.indices = indices; draw.index_count = 3; rejects(draw);
    const std::uint16_t repeated[]{0,0,0}; draw.indices = repeated;
    require(validate_draw(draw, maximum) == RenderStatus::Ok, "degenerate valid triangle");
    ++cases;

    require(cases == 14, "fixture count");
    std::cout << "PASS render-clip: " << cases << " cases\n";
}
