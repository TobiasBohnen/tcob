// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Camera.hpp"

#include "tcob/core/Rect.hpp"

namespace tcob::gfx {

auto camera::get_matrix() -> mat4
{
    update_transform();
    return _transform.as_matrix4();
}

auto camera::get_viewport() const -> rect_f
{
    return {_offset, _targetSize};
}

auto camera::get_transformed_viewport() -> rect_f
{
    return convert_screen_to_world(rect_i {get_viewport()});
}

auto camera::get_size() const -> size_f
{
    return _targetSize;
}

void camera::set_size(size_f size)
{
    if (_targetSize != size) {
        _targetSize     = size;
        _transformDirty = true;
    }
}

auto camera::get_offset() const -> point_f
{
    return _offset;
}

void camera::set_offset(point_f pos)
{
    _offset = pos;
}

auto camera::get_zoom() const -> size_f
{
    return _zoom;
}

void camera::set_zoom(size_f zoom)
{
    if (_zoom != zoom) {
        _zoom           = zoom;
        _transformDirty = true;
    }
}

void camera::zoom_by(size_f factor)
{
    set_zoom(_zoom * factor);
}

auto camera::get_position() const -> point_f
{
    return _position;
}

void camera::set_position(point_f pos)
{
    if (_position != pos) {
        _position       = pos;
        _transformDirty = true;
    }
}

void camera::move_by(point_f offset)
{
    set_position(_position + offset);
}

void camera::look_at(point_f pos)
{
    point_f const offset {get_transformed_viewport().get_local_center() * point_f {_zoom.Width, _zoom.Height}};
    set_position(pos - offset);
}

auto camera::convert_world_to_screen(rect_f const& rect) -> rect_i
{
    update_transform();

    point_f const tl {convert_world_to_screen(rect.top_left())};
    point_f const br {convert_world_to_screen(rect.bottom_right())};

    return rect_i::FromLTRB(static_cast<i32>(tl.X), static_cast<i32>(tl.Y), static_cast<i32>(br.X), static_cast<i32>(br.Y));
}

auto camera::convert_world_to_screen(point_f point) -> point_i
{
    update_transform();

    return point_i {_transform * point + _offset};
}

auto camera::convert_screen_to_world(rect_i const& rect) -> rect_f
{
    update_transform();

    point_f const tl {convert_screen_to_world(rect.top_left())};
    point_f const br {convert_screen_to_world(rect.bottom_right())};

    return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
}

auto camera::convert_screen_to_world(point_i point) -> point_f
{
    update_transform();

    return _transform.as_inverted() * (point_f {point} - _offset);
}

void camera::update_transform()
{
    if (_transformDirty) {
        _transform.to_identity();
        if (_zoom != size_f::One) {
            _transform.scale_at(_zoom, {_targetSize.Width / 2, _targetSize.Height / 2});
        }
        if (_position != point_f::Zero) {
            _transform.translate({-_position.X, -_position.Y});
        }

        _transformDirty = false;
    }
}
}
