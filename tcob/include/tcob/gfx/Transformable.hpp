// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <optional>

#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/core/data/Transform.hpp>

namespace tcob {

//!@addtogroup gfx
//!@{

//!@brief base class for transformable objects
class Transformable {
public:
    Transformable() = default;
    virtual ~Transformable() = default;

    //!@brief Gets the size of an object.
    auto size() const -> SizeF;
    //!@brief Sets the size of an object
    void size(const SizeF& size);

    //!@brief Gets the position of an object.
    auto position() const -> PointF;
    //!@brief Sets the position of an object.
    void position(const PointF& position);
    //!@brief Moves the object by a given offset.
    void move_by(const PointF& offset);

    //!@brief Gets the translation of an object.
    auto translation() const -> const PointF&;
    //!@brief Sets the translation of an object.
    void translation(const PointF& translation);
    //!@brief Modifies the current translation by a given offset.
    void translate_by(const PointF& offset);

    //!@brief
    auto rotation() const -> f32;
    //!@brief
    void rotation(f32 angle);
    //!@brief
    void rotate_by(f32 angle);

    //!@brief
    auto scale() const -> const SizeF&;
    //!@brief
    void scale(const SizeF& scale);
    //!@brief
    void scale_by(const SizeF& factor);

    //!@brief
    auto skew() const -> const PointF&;
    //!@brief
    void skew(const PointF& skew);
    //!@brief
    void skew_by(const PointF& factor);

    //!@brief
    auto bounds() const -> const RectF&;
    void bounds(const RectF& rect);

    //!@brief
    auto pivot() const -> PointF;
    //!@brief
    //!@param pivot
    //!@param local
    void pivot(const PointF& pivot, bool local = true);

    //!@brief Resets the current transformation to an identity matrix.
    void reset_transform();

protected:
    auto transform() -> const Transform&;
    auto is_transform_dirty() const -> bool;
    void update_transform();

private:
    Transform _transform;
    f32 _rotation { 0 };
    SizeF _scale { SizeF::One };
    PointF _translation { PointF::Zero };
    PointF _skew { PointF::Zero };
    RectF _rect { RectF::Zero };
    std::optional<PointF> _pivot {};
    bool _isDirty { false };
};

//! @}
}