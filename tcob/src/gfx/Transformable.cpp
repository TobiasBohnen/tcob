// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Transformable.hpp"

namespace tcob::gfx {

transformable::transformable()
{
    Translation.Changed.connect([&](auto const&) { mark_transform_dirty(); });
    Rotation.Changed.connect([&](auto const&) { mark_transform_dirty(); });
    Scale.Changed.connect([&](auto const&) { mark_transform_dirty(); });
    Scale(size_f::One);
    Skew.Changed.connect([&](auto const&) { mark_transform_dirty(); });
}

auto transformable::get_transform() -> transform const&
{
    update_transform();
    return _transform;
}

void transformable::translate_by(point_f offset)
{
    Translation += offset;
}

void transformable::rotate_by(degree_f angle)
{
    Rotation += angle;
}

void transformable::scale_by(size_f factor)
{
    Scale *= factor;
}

void transformable::skew_by(std::pair<degree_f, degree_f> factor)
{
    Skew = {Skew->first + factor.first, Skew->second + factor.second};
}

void transformable::mark_transform_dirty()
{
    if (_isDirty) { return; }

    _isDirty = true;
    on_transform_changed();
}

void transformable::update_transform()
{
    if (!_isDirty) { return; }
    _isDirty = false;

    point_f const p {pivot()};
    _transform.to_identity();

    if (Scale != size_f::One) { _transform.scale_at(Scale(), p); }
    if (Rotation != degree_f {0}) { _transform.rotate_at(Rotation(), p); }
    if (Skew->first != 0 || Skew->second != 0) { _transform.skew_at(Skew(), p); }
    if (Translation != point_f::Zero) { _transform.translate(Translation()); }
}

void transformable::reset_transform()
{
    _transform.to_identity();
    Rotation    = degree_f {0};
    Scale       = size_f::One;
    Translation = point_f::Zero;
    Skew        = {degree_f {0.0f}, degree_f {0.0f}};
    mark_transform_dirty();
}

}
