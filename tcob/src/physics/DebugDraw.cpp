// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/DebugDraw.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <memory>

    #include "B2D.hpp"

namespace tcob::physics {

debug_draw::debug_draw()
    : _impl {std::make_unique<detail::b2d_debug_draw>(this)}
{
}

debug_draw::~debug_draw() = default;

}

#endif
