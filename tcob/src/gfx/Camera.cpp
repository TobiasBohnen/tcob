// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Camera.hpp"

#include "tcob/core/Rect.hpp"

namespace tcob::gfx {

camera::camera()
{
    Size.Changed.connect([&](auto const&) { update_transform(); });
    Zoom(size_f::One);
    Zoom.Changed.connect([&](auto const&) { update_transform(); });
    Position.Changed.connect([&](auto const&) { update_transform(); });
}

camera::camera(camera const& other) noexcept
{
    *this = other;
}

auto camera::operator=(camera const& other) noexcept -> camera&
{
    Size           = other.Size();
    Zoom           = other.Zoom();
    Position       = other.Position();
    Offset         = other.Offset();
    VisibilityMask = other.VisibilityMask;

    return *this;
}

auto camera::get_matrix() const -> mat4
{
    return _transform.as_matrix4();
}

auto camera::get_viewport() const -> rect_f
{
    return {Offset, Size};
}

auto camera::get_transformed_viewport() const -> rect_f
{
    return convert_screen_to_world(rect_i {get_viewport()});
}

void camera::zoom_by(size_f factor)
{
    Zoom = (Zoom() * factor);
}

void camera::move_by(point_f offset)
{
    Position = (Position() + offset);
}

void camera::look_at(point_f pos)
{
    point_f const offset {get_transformed_viewport().get_local_center() * point_f {Zoom->Width, Zoom->Height}};
    Position = pos - offset;
}

auto camera::get_look_at() const -> point_f
{
    return Position() + get_transformed_viewport().get_local_center() * point_f {Zoom->Width, Zoom->Height};
}

auto camera::convert_world_to_screen(rect_f const& rect) const -> rect_i
{
    point_f const tl {convert_world_to_screen(rect.top_left())};
    point_f const br {convert_world_to_screen(rect.bottom_right())};

    return rect_i::FromLTRB(static_cast<i32>(tl.X), static_cast<i32>(tl.Y), static_cast<i32>(br.X), static_cast<i32>(br.Y));
}

auto camera::convert_world_to_screen(point_f point) const -> point_i
{
    return point_i {_transform * point + Offset()};
}

auto camera::convert_screen_to_world(rect_i const& rect) const -> rect_f
{
    point_f const tl {convert_screen_to_world(rect.top_left())};
    point_f const br {convert_screen_to_world(rect.bottom_right())};

    return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
}

auto camera::convert_screen_to_world(point_i point) const -> point_f
{
    return _transform.as_inverted() * (point_f {point} - Offset());
}

void camera::update_transform()
{
    _transform.to_identity();
    if (Zoom != size_f::One) {
        _transform.scale_at(Zoom, {Size->Width / 2, Size->Height / 2});
    }
    if (Position != point_f::Zero) {
        _transform.translate({-Position->X, -Position->Y});
    }
}
}
