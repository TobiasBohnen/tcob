// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/Camera.hpp>

#include <tcob/core/data/Rect.hpp>

namespace tcob {
auto Camera::position() const -> PointF
{
    return _position;
}

void Camera::position(const PointF& position)
{
    _position = position;
    _transformDirty = true;
}

void Camera::move_by(const PointF& offset)
{
    position(_position + offset);
}

void Camera::look_at(const PointF& pos)
{
    const PointF offset = frustum().center_local() * PointF { _scale.Width, _scale.Height };
    position(pos - offset);
}

auto Camera::rotation() const -> f32
{
    return _rotation;
}

void Camera::rotation(f32 angle)
{
    _rotation = static_cast<f32>(std::fmod(angle, 360));
    if (_rotation < 0)
        _rotation += 360.f;
    _transformDirty = true;
}

void Camera::rotate_by(f32 angle)
{
    rotation(angle + _rotation);
}

auto Camera::zoom() const -> SizeF
{
    return _scale;
}

void Camera::zoom(const SizeF& zoom)
{
    _scale = zoom;
    _transformDirty = true;
}

void Camera::zoom_by(const SizeF& factor)
{
    zoom(_scale * factor);
}

void Camera::update_matrix()
{
    if (_transformDirty) {
        _transform.to_identity();
        if (_rotation != 0)
            _transform.rotate_at(_rotation, PointF::Zero);
        if (_scale != SizeF::One)
            _transform.scale_at(_scale, PointF::Zero);
        if (_position != PointF::Zero)
            _transform.translate({ -(_position.X * 2), (_position.Y * 2) });

        _transformDirty = false;
    }
}

auto Camera::matrix() -> mat4
{
    update_matrix();
    return _transform.matrix4();
}

auto Camera::convert_world_to_screen(const PointF& point) -> PointF
{
    const RectF fr { frustum() };
    const f32 x { (point.X - fr.Left) / fr.Width };
    const f32 y { (point.Y - fr.Top) / fr.Height };

    return { x, y };
}

auto Camera::convert_world_to_screen(const RectF& rect) -> RectF
{
    update_matrix();

    const PointF tl { _transform * rect.top_left() };
    const PointF br { _transform * rect.bottom_right() };

    const f32 width { br.X - tl.X };
    const f32 height { br.Y - tl.Y };

    const RectF fr { frustum() };
    const f32 x { (rect.Left - fr.Left) / fr.Width };
    const f32 y { (rect.Top - fr.Top) / fr.Height };

    return { { x, y }, { width, height } };
}

auto Camera::convert_screen_to_world(const PointF& point) -> PointF
{
    const RectF fr { frustum() };
    const f32 x { point.X * fr.Width + fr.Left };
    const f32 y { point.Y * fr.Height + fr.Top };

    return { x, y };
}

auto Camera::convert_screen_to_world(const RectF& rect) -> RectF
{
    const RectF fr { frustum() };
    const f32 x { rect.Left * fr.Width + fr.Left };
    const f32 y { rect.Top * fr.Height + fr.Top };

    const PointF tl { _transform.inverse() * rect.top_left() };
    const PointF br { _transform.inverse() * rect.bottom_right() };

    const f32 width { br.X - tl.X };
    const f32 height { br.Y - tl.Y };

    return { { x, y }, { width, height } };
}

auto Camera::frustum() const -> RectF
{
    const SizeF size { 1 / _scale.Width, 1 / _scale.Height };
    const PointF pos { position().X + (0.5f - (size.Width / 2)), position().Y + (0.5f - (size.Height / 2)) };
    return { pos, size };
}

auto Camera::aspect_ratio() -> f32
{
    return _aspectRatio;
}

void Camera::aspect_ratio(f32 asp)
{
    zoom({ 1 / asp, 1 });
    move_by({ -frustum().Left, 0 });
    _aspectRatio = asp;
}
}