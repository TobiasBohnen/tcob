// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Geometry.hpp"

#include <span>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Transform.hpp"

namespace tcob::gfx {

void geometry_store::clear()
{
    _inds.clear();
    _verts.clear();
}

[[nodiscard]] auto geometry_store::empty() const -> bool
{
    return _inds.empty() && _verts.empty();
}

auto geometry_store::get_indices(isize id) const -> std::span<u32 const>
{
    if (auto it {_inds.find(id)}; it != _inds.end()) {
        return it->second;
    }
    return {};
}

auto geometry_store::get_vertices(isize id) const -> std::span<vertex const>
{
    if (auto it {_verts.find(id)}; it != _verts.end()) {
        return it->second;
    }
    return {};
}

void geometry_store::set_indices(isize id, std::vector<u32> const& values)
{
    _inds[id] = values;
}

void geometry_store::set_vertices(isize id, std::vector<vertex> const& values)
{
    _verts[id] = values;
}

}

namespace tcob::gfx::geometry {

void set_position(quad& q, rect_f const& rect, transform const& xform)
{
    q[0].Position = (xform * rect.top_right());
    q[1].Position = (xform * rect.bottom_right());
    q[2].Position = (xform * rect.bottom_left());
    q[3].Position = (xform * rect.top_left());
}

void set_position(quad& q, rect_f const& rect)
{
    q[0].Position = rect.top_right();
    q[1].Position = rect.bottom_right();
    q[2].Position = rect.bottom_left();
    q[3].Position = rect.top_left();
}

void set_color(quad& q, color c)
{
    q[0].Color = q[1].Color = q[2].Color = q[3].Color = c;
}

void set_color(std::span<vertex> verts, color c)
{
    for (auto& v : verts) { v.Color = c; }
}

void set_texcoords(quad& q, texture_region const& region, bool flipHorizontally, bool flipVertically)
{
    rect_f const rect {region.UVRect};
    f32 const    level {static_cast<f32>(region.Level)};

    uv topRight {.U = rect.right(), .V = rect.top(), .Level = level};
    uv bottomRight {.U = rect.right(), .V = rect.bottom(), .Level = level};
    uv bottomLeft {.U = rect.left(), .V = rect.bottom(), .Level = level};
    uv topLeft {.U = rect.left(), .V = rect.top(), .Level = level};

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

void set_texcoords(quad& q, pass const& pass, string const& region, bool flipHorizontally, bool flipVertically)
{
    if (pass.Texture && pass.Texture->regions().contains(region)) {
        geometry::set_texcoords(q, pass.Texture->regions()[region], flipHorizontally, flipVertically);
    } else {
        geometry::set_texcoords(q, {.UVRect = {0, 0, 1, 1}, .Level = 0});
    }
}

void scroll_texcoords(quad& q, point_f offset)
{
    f32 const left {q[3].TexCoords.U + offset.X};
    f32 const top {q[3].TexCoords.V + offset.Y};
    f32 const right {q[1].TexCoords.U + offset.X};
    f32 const bottom {q[1].TexCoords.V + offset.Y};

    rect_f const rect {left, top, right - left, bottom - top};
    f32 const    level {q[0].TexCoords.Level};

    q[0].TexCoords = {.U = rect.right(), .V = rect.top(), .Level = level};
    q[1].TexCoords = {.U = rect.right(), .V = rect.bottom(), .Level = level};
    q[2].TexCoords = {.U = rect.left(), .V = rect.bottom(), .Level = level};
    q[3].TexCoords = {.U = rect.left(), .V = rect.top(), .Level = level};
}

}
