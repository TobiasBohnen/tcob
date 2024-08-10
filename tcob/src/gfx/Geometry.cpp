// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Geometry.hpp"

namespace tcob::gfx::geometry {

void set_position(quad& q, rect_f const& rect, transform const& xform)
{
    point_f const topRight {xform * rect.top_right()};
    point_f const bottomRight {xform * rect.bottom_right()};
    point_f const bottomLeft {xform * rect.bottom_left()};
    point_f const topLeft {xform * rect.top_left()};

    q[0].Position = {topRight.X, topRight.Y};
    q[1].Position = {bottomRight.X, bottomRight.Y};
    q[2].Position = {bottomLeft.X, bottomLeft.Y};
    q[3].Position = {topLeft.X, topLeft.Y};
}

void set_position(quad& q, rect_f const& rect)
{
    point_f const topRight {rect.top_right()};
    point_f const bottomRight {rect.bottom_right()};
    point_f const bottomLeft {rect.bottom_left()};
    point_f const topLeft {rect.top_left()};

    q[0].Position = {topRight.X, topRight.Y};
    q[1].Position = {bottomRight.X, bottomRight.Y};
    q[2].Position = {bottomLeft.X, bottomLeft.Y};
    q[3].Position = {topLeft.X, topLeft.Y};
}

void set_color(quad& q, color c)
{
    q[0].Color = q[1].Color = q[2].Color = q[3].Color = c.as_array();
}

void set_color(std::span<vertex> verts, color c)
{
    for (auto& v : verts) {
        v.Color = c.as_array();
    }
}

void set_texcoords(quad& q, texture_region const& region, bool flipHorizontally, bool flipVertically)
{
    rect_f const rect {region.UVRect};
    f32 const    level {static_cast<f32>(region.Level)};

    vec3 topRight {rect.right(), rect.top(), level};
    vec3 bottomRight {rect.right(), rect.bottom(), level};
    vec3 bottomLeft {rect.left(), rect.bottom(), level};
    vec3 topLeft {rect.left(), rect.top(), level};

    if (flipHorizontally) {
        std::swap(topLeft, topRight);
        std::swap(bottomLeft, bottomRight);
    }
    if (flipVertically) {
        std::swap(topLeft, bottomLeft);
        std::swap(topRight, bottomRight);
    }

    q[0].TexCoords = topRight;
    q[1].TexCoords = bottomRight;
    q[2].TexCoords = bottomLeft;
    q[3].TexCoords = topLeft;
}

void scroll_texcoords(quad& q, point_f offset)
{
    f32 const left {q[3].TexCoords[0] + offset.X};
    f32 const top {q[3].TexCoords[1] + offset.Y};
    f32 const right {q[1].TexCoords[0] + offset.X};
    f32 const bottom {q[1].TexCoords[1] + offset.Y};

    rect_f const rect {left, top, right - left, bottom - top};
    f32 const    level {q[0].TexCoords[2]};

    q[0].TexCoords = {rect.right(), rect.top(), level};
    q[1].TexCoords = {rect.right(), rect.bottom(), level};
    q[2].TexCoords = {rect.left(), rect.bottom(), level};
    q[3].TexCoords = {rect.left(), rect.top(), level};
}

}
