// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob::physics {
////////////////////////////////////////////////////////////

enum class body_type : u8 {
    Static,
    Kinematic,
    Dynamic
};

}
