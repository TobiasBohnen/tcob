// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"

namespace tcob::physics {
////////////////////////////////////////////////////////////

enum class body_type : u8 {
    Static,
    Kinematic,
    Dynamic
};

struct circle {
    /// The local center
    point_f Center;

    /// The radius
    f32 Radius {0.0f};
};

struct segment {
    /// The first point
    point_f Point1;

    /// The second point
    point_f Point2;
};

struct chain_segment {
    /// The tail ghost vertex
    point_f Ghost1;

    /// The line segment
    segment Segment;

    /// The head ghost vertex
    point_f Ghost2;
};

struct filter {
    u64 CategoryBits {1};
    u64 MaskBits {0xffffffffffffffff};
    i32 GroupIndex {0};
};

struct surface_material {
    /// The Coulomb (dry) friction coefficient, usually in the range [0,1].
    f32 Friction {0.6f};

    /// The coefficient of restitution (bounce) usually in the range [0,1].
    f32 Restitution {0.0f};

    /// The rolling resistance usually in the range [0,1].
    f32 RollingResistance {0.0f};

    /// The tangent speed for conveyor belts
    f32 TangentSpeed {0.0f};

    /// Custom debug draw color.
    color CustomColor;
};

struct mass_data {
    /// The mass of the shape, usually in kilograms.
    f32 Mass {0};

    /// The position of the shape's centroid relative to the shape's origin.
    point_f Center;

    /// The rotational inertia of the shape about the local origin.
    f32 RotationalInertia {0};
};

class world;
class body;
class shape;
class joint;
class debug_draw;

namespace detail {
    class b2d_world;
    class b2d_body;
    class b2d_chain;
    class b2d_shape;
    class b2d_joint;
    class b2d_debug_draw;

    template <typename T, auto Getter, auto Setter, typename Parent>
    auto constexpr make_prop(Parent* owner) -> prop_fn<T>
    {
        return prop_fn<T> {{owner,
                            [](void* ctx) { return (static_cast<Parent*>(ctx)->get_impl()->*Getter)(); },
                            [](void* ctx, T const& value) { (static_cast<Parent*>(ctx)->get_impl()->*Setter)(value); }}};
    }
}

}
