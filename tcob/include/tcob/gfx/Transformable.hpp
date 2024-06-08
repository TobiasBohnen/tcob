// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

//!@brief base class for transformable objects
class TCOB_API transformable {
public:
    transformable();
    virtual ~transformable() = default;

    prop<point_f>                       Translation;
    prop<degree_f>                      Rotation;
    prop<size_f>                        Scale;
    prop<std::pair<degree_f, degree_f>> Skew;

    auto get_transform() -> transform const&;

    void translate_by(point_f offset);
    void rotate_by(degree_f angle);
    void scale_by(size_f factor);
    void skew_by(std::pair<degree_f, degree_f> factor);

    void virtual reset_transform();

protected:
    auto virtual get_pivot() const -> point_f = 0;
    void virtual on_transform_dirty()         = 0;
    void mark_transform_dirty();
    void update_transform();

private:
    transform _transform {};
    bool      _isDirty {true};
};

////////////////////////////////////////////////////////////

//!@brief base class for rectangular transformable objects
class TCOB_API rect_transformable : public transformable {
public:
    rect_transformable();

    prop_fn<point_f>             Center;
    prop<rect_f>                 Bounds;
    prop<std::optional<point_f>> Pivot;

    auto virtual get_global_position() const -> point_f;
    auto get_global_transform() -> transform;

    void move_by(point_f offset);

    void reset_transform() override;

protected:
    auto get_pivot() const -> point_f override;

    auto virtual get_transform_parent() const -> rect_transformable*;
};

}
