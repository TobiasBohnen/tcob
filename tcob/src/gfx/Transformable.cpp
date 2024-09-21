// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Transformable.hpp"

namespace tcob::gfx {

transformable::transformable()
{
    Translation.Changed.connect([&]() { mark_transform_dirty(); });
    Rotation.Changed.connect([&]() { mark_transform_dirty(); });
    Scale.Changed.connect([&]() { mark_transform_dirty(); });
    Scale(size_f::One);
    Skew.Changed.connect([&]() { mark_transform_dirty(); });
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

    point_f const pivot {get_pivot()};
    _transform.to_identity();

    if (Scale != size_f::One) { _transform.scale_at(Scale(), pivot); }
    if (Rotation != degree_f {0}) { _transform.rotate_at(Rotation(), pivot); }
    if (Skew->first != 0 || Skew->second != 0) { _transform.skew_at(Skew(), pivot); }
    if (Translation != point_f::Zero) { _transform.translate(Translation()); }

    _isDirty = false;
}

void transformable::reset_transform()
{
    _transform.to_identity();
    Rotation    = degree_f {0};
    Scale       = size_f::One;
    Translation = point_f::Zero;
    Skew        = {0.0f, 0.0f};
    mark_transform_dirty();
}

////////////////////////////////////////////////////////////

rect_transformable::rect_transformable()
    : Center {{[&]() { return Bounds->get_center(); },
               [&](point_f const& value) { Bounds = Bounds->as_moved_to({value.X - Bounds->Width / 2.0f, value.Y - Bounds->Height / 2.0f}); }}}
{
    Bounds.Changed.connect([&]() { mark_transform_dirty(); });
    Pivot.Changed.connect([&]() { mark_transform_dirty(); });
}

void rect_transformable::move_by(point_f offset)
{
    Bounds = {Bounds->get_position() + offset, Bounds->get_size()};
}

auto rect_transformable::get_pivot() const -> point_f
{
    if (Pivot().has_value()) {
        return Bounds->top_left() + *Pivot();
    }

    return Center();
}

}
