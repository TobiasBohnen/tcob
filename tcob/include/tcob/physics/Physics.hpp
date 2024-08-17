// Copyright (c) 2024 Tobias Bohnen
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
