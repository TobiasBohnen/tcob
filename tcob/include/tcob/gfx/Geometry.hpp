// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <span>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Transform.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct vertex final {
    point_f Position {};  // x, y
    color   Color {};     // r, g, b, a
    uv      TexCoords {}; // u, v, level

    auto operator==(vertex const& other) const -> bool = default;
};

using triangle = std::array<vertex, 3>;
using quad     = std::array<vertex, 4>;

struct geometry_data {
    std::span<vertex const> Vertices;
    std::span<u32 const>    Indices;
    primitive_type          Type {};
};

namespace geometry {
    TCOB_API void set_position(quad& q, rect_f const& rect);
    TCOB_API void set_position(quad& q, rect_f const& rect, transform const& xform);

    TCOB_API void set_color(quad& q, color c);
    TCOB_API void set_color(std::span<vertex> verts, color c);

    TCOB_API void set_texcoords(quad& q, texture_region const& region, bool flipHorizontally = false, bool flipVertically = false);

    TCOB_API void scroll_texcoords(quad& q, point_f offset);
}

}
