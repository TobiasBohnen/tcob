// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Camera.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {

camera::camera(render_target& parent)
    : _parent {parent}
{
}

auto camera::get_matrix() const -> mat4
{
    return get_transform().as_matrix4();
}

auto camera::get_viewport() const -> rect_f
{
    return {ViewOffset, size_f {_parent.Size()}};
}

auto camera::get_transformed_viewport() const -> rect_f
{
    return convert_screen_to_world(rect_i {get_viewport()});
}

void camera::zoom_by(size_f factor)
{
    Zoom = (Zoom * factor);
}

void camera::move_by(point_f offset)
{
    Position = (Position + offset);
}

void camera::look_at(point_f pos)
{
    point_f const offset {get_transformed_viewport().local_center() * point_f {Zoom.Width, Zoom.Height}};
    Position = pos - offset;
}

auto camera::get_look_at() const -> point_f
{
    return Position + get_transformed_viewport().local_center() * point_f {Zoom.Width, Zoom.Height};
}

auto camera::convert_world_to_screen(rect_f const& rect) const -> rect_i
{
    point_f const tl {convert_world_to_screen(rect.top_left())};
    point_f const br {convert_world_to_screen(rect.bottom_right())};

    return rect_i::FromLTRB(static_cast<i32>(tl.X), static_cast<i32>(tl.Y), static_cast<i32>(br.X), static_cast<i32>(br.Y));
}

auto camera::convert_world_to_screen(point_f point) const -> point_i
{
    return point_i {get_transform() * point + ViewOffset};
}

auto camera::convert_screen_to_world(rect_i const& rect) const -> rect_f
{
    point_f const tl {convert_screen_to_world(rect.top_left())};
    point_f const br {convert_screen_to_world(rect.bottom_right())};

    return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
}

auto camera::convert_screen_to_world(point_i point) const -> point_f
{
    return get_transform().as_inverted() * (point_f {point} - ViewOffset);
}

void camera::push_state()
{
    _states.emplace(Zoom, Position);
    Zoom     = size_f::One;
    Position = point_f::Zero;
}

void camera::pop_state()
{
    if (_states.empty()) { return; }
    Zoom     = _states.top().Zoom;
    Position = _states.top().Position;
    _states.pop();
}

auto camera::get_transform() const -> transform
{
    transform xform;
    if (Zoom != size_f::One) {
        auto const size {_parent.Size()};
        xform.scale_at(Zoom, {size.Width / 2.f, size.Height / 2.f});
    }
    if (Position != point_f::Zero) {
        xform.translate({-Position.X, -Position.Y});
    }
    return xform;
}

}
