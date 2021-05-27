// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/Transformable.hpp>

namespace tcob {

auto Transformable::translation() const -> const PointF&
{
    return _translation;
}

void Transformable::translation(const PointF& translation)
{
    if (_translation != translation) {
        _translation = translation;
        _isDirty = true;
    }
}

void Transformable::translate_by(const PointF& offset)
{
    translation(_translation + offset);
}

auto Transformable::rotation() const -> f32
{
    return _rotation;
}

void Transformable::rotation(f32 angle)
{
    f32 newRotation { static_cast<f32>(std::fmod(angle, 360)) };
    if (newRotation < 0)
        newRotation += 360.f;

    if (_rotation != newRotation) {
        _rotation = newRotation;
        _isDirty = true;
    }
}

void Transformable::rotate_by(f32 angle)
{
    rotation(angle + _rotation);
}

auto Transformable::scale() const -> const SizeF&
{
    return _scale;
}

void Transformable::scale(const SizeF& scale)
{
    if (_scale != scale) {
        _scale = scale;
        _isDirty = true;
    }
}

void Transformable::scale_by(const SizeF& factor)
{
    scale(_scale * factor);
}

auto Transformable::skew() const -> const PointF&
{
    return _skew;
}

void Transformable::skew(const PointF& skew)
{
    PointF newSkew;
    newSkew.X = static_cast<f32>(std::fmod(skew.X, 180));
    newSkew.Y = static_cast<f32>(std::fmod(skew.Y, 180));
    if (_skew != newSkew) {
        _skew = newSkew;
        _isDirty = true;
    }
}

void Transformable::skew_by(const PointF& factor)
{
    skew(_skew + factor);
}

auto Transformable::transform() -> const Transform&
{
    update_transform();
    return _transform;
}

auto Transformable::is_transform_dirty() const -> bool
{
    return _isDirty;
}

void Transformable::transform_dirty(bool dirty)
{
    _isDirty = dirty;
}

void Transformable::update_transform()
{
    if (_isDirty) {
        const PointF origin { pivot() };
        _transform.to_identity();

        if (_scale != SizeF::One)
            _transform.scale_at(_scale, origin);
        if (_rotation != 0)
            _transform.rotate_at(360 - _rotation, origin);
        if (_skew != PointF::Zero)
            _transform.skew_at(_skew, origin);
        if (_translation != PointF::Zero)
            _transform.translate(_translation);
        _isDirty = false;
    }
}

void Transformable::reset_transform()
{
    _transform.to_identity();
    _rotation = { 0 };
    _scale = { SizeF::One };
    _translation = { PointF::Zero };
    _skew = { PointF::Zero };
    _isDirty = { true };
}

////////////////////////////////////////////////////////////

auto RectTransformable::size() const -> SizeF
{
    return _rect.size();
}

void RectTransformable::size(const SizeF& size)
{
    if (_rect.size() != size) {

        _rect.size(size);
        transform_dirty(true);
    }
}

auto RectTransformable::position() const -> PointF
{
    return _rect.position();
}

void RectTransformable::position(const PointF& position)
{
    if (_rect.position() != position) {
        _rect.position(position);
        transform_dirty(true);
    }
}

void RectTransformable::move_by(const PointF& offset)
{
    position(_rect.position() + offset);
}

auto RectTransformable::bounds() const -> const RectF&
{
    return _rect;
}

void RectTransformable::bounds(const RectF& rect)
{
    if (_rect != rect) {
        _rect = rect;
        transform_dirty(true);
    }
}

auto RectTransformable::pivot() const -> PointF
{
    return _pivot.value_or(_rect.center());
}

void RectTransformable::pivot(const PointF& pivot, bool local)
{
    if (local)
        _pivot = _rect.top_left() + pivot;
    else
        _pivot = pivot;
    transform_dirty(true);
}

void RectTransformable::reset_transform()
{
    Transformable::reset_transform();
    _rect = { RectF::Zero };
}

}