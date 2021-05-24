// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/core/data/Color.hpp>
#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/core/data/Transform.hpp>

namespace tcob {
#pragma pack(push, 1)
struct Vertex final {
    vec2 Position; // x, y
    std::array<u8, 4> Color; // r, g, b, a
    vec3 TexCoords; // u, v, level
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Quad final {
    void color(const Color& color);

    void texcoords(const TextureRegion& region);
    void scroll_texcoords(const PointF& region);

    void position(const RectF& rect, const Transform& trans);
    void position(const RectF& rect);

    Vertex TopRight;
    Vertex BottomRight;
    Vertex BottomLeft;
    Vertex TopLeft;
};
#pragma pack(pop)

}