// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <utility>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Transform.hpp"

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

    auto transform() -> gfx::transform const&;

    void translate_by(point_f offset);
    void rotate_by(degree_f angle);
    void scale_by(size_f factor);
    void skew_by(std::pair<degree_f, degree_f> factor);

    void virtual reset_transform();

protected:
    auto virtual pivot() const -> point_f = 0;
    void virtual on_transform_changed()   = 0;
    void mark_transform_dirty();
    void update_transform();

private:
    gfx::transform _transform {};
    bool           _isDirty {true};
};

}
