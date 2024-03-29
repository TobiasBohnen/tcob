// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/PointCloud.hpp"

namespace tcob::gfx {

static_point_cloud::static_point_cloud(std::span<vertex> points)
{
    _renderer.set_geometry(points);
    Material.Changed.connect([&](auto const& value) { _renderer.set_material(value); });
}

void static_point_cloud::on_update(milliseconds)
{
    // nothing to do
}

auto static_point_cloud::can_draw() const -> bool
{
    return !Material().is_expired();
}

void static_point_cloud::on_draw_to(render_target& target)
{
    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////

point_cloud::point_cloud(i32 reservedSize)
{
    Material.Changed.connect([&](auto const& value) { _renderer.set_material(value); });
    _points.reserve(reservedSize);
}

auto point_cloud::create_point() -> vertex&
{
    return _points.emplace_back();
}

auto point_cloud::get_point_count() const -> i32
{
    return static_cast<i32>(std::ssize(_points));
}

auto point_cloud::get_point_at(i32 index) -> vertex&
{
    return _points.at(index);
}

void point_cloud::on_update(milliseconds)
{
    // nothing to do
}

auto point_cloud::can_draw() const -> bool
{
    return !Material().is_expired();
}

void point_cloud::on_draw_to(render_target& target)
{
    _renderer.set_geometry(_points);
    _renderer.render_to_target(target);
}

}
