// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <span>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/core/Size.hpp"
    #include "tcob/physics/box2d/B2D.hpp"

namespace tcob::physics::box2d {
////////////////////////////////////////////////////////////

class TCOB_API shape : public non_copyable {
    friend auto detail::get_impl(shape const* b) -> b2Shape*;

protected:
    explicit shape(std::unique_ptr<b2Shape> shape);
    ~shape();

    template <typename T>
    auto get_impl() const -> T*
    {
        return static_cast<T*>(_shape.get());
    }

private:
    std::unique_ptr<b2Shape> _shape;
};

////////////////////////////////////////////////////////////

class TCOB_API polygon_shape final : public shape {
public:
    polygon_shape();

    void set(std::span<point_f const> vecs) const;
    void set_as_box(size_f extents) const;
    void set_as_box(rect_f const& extents, radian_f angle) const;
};

////////////////////////////////////////////////////////////

class TCOB_API circle_shape final : public shape {
public:
    circle_shape();
    circle_shape(f32 radius);

    prop_fn<f32> Radius;
};

////////////////////////////////////////////////////////////

class TCOB_API edge_shape final : public shape {
public:
    edge_shape();

    void set_one_sided(point_f v0, point_f v1, point_f v2, point_f v3);
    void set_two_sided(point_f v1, point_f v2);
};

}

#endif
