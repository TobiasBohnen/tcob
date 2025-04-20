// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Point.hpp"

namespace tcob::physics {
////////////////////////////////////////////////////////////

enum class body_type : u8 {
    Static,
    Kinematic,
    Dynamic
};

struct AABB {
    point_f LowerBounds;
    point_f UpperBounds;
};

struct filter {
    u64 CategoryBits {1};
    u64 MaskBits {0xffffffffffffffff};
    i32 GroupIndex {0};
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
    class b2d_shape;
    class b2d_joint;
    class b2d_debug_draw;
}

}
