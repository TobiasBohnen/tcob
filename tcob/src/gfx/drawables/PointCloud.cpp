// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/PointCloud.hpp"

#include <iterator>
#include <span>

#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

point_cloud::point_cloud(i32 reservedSize)
{
    _points.reserve(reservedSize);
}

auto point_cloud::create_point() -> vertex&
{
    return _points.emplace_back();
}

void point_cloud::clear()
{
    _points.clear();
}

auto point_cloud::size() const -> i32
{
    return static_cast<i32>(std::ssize(_points));
}

auto point_cloud::get_point_at(i32 index) -> vertex&
{
    return _points.at(index);
}

auto point_cloud::can_draw() const -> bool
{
    return !(*Material).is_expired() && !_points.empty();
}

void point_cloud::on_draw_to(render_target& target)
{
    for (isize i {0}; i < Material->pass_count(); ++i) {
        auto const& pass {Material->get_pass(i)};

        _renderer.set_geometry(_points, &pass);
        _renderer.render_to_target(target);
    }
}

}
