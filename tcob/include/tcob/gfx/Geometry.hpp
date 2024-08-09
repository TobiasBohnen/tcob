// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct vertex final {
    std::array<f32, 2> Position;  // x, y
    std::array<u8, 4>  Color;     // r, g, b, a
    std::array<f32, 3> TexCoords; // u, v, level
};

inline auto operator==(vertex const& left, vertex const& right) -> bool
{
    return left.Color == right.Color && left.Position == right.Position && left.TexCoords == right.TexCoords;
}

using triangle = std::array<vertex, 3>;
using quad     = std::array<vertex, 4>;

struct geometry_data {
    std::span<vertex> Vertices;
    std::span<u32>    Indices;
};

namespace geometry {
    TCOB_API void set_position(quad& q, rect_f const& rect);
    TCOB_API void set_position(quad& q, rect_f const& rect, transform const& xform);
    TCOB_API void set_color(quad& q, color c);
    TCOB_API void set_texcoords(quad& q, texture_region const& region, bool flipHorizontally = false, bool flipVertically = false);
    TCOB_API void scroll_texcoords(quad& q, point_f offset);
}

}
