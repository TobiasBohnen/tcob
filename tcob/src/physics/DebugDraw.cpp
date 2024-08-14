// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/DebugDraw.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include "B2D.hpp"

namespace tcob::physics {

debug_draw::debug_draw(debug_draw_settings settings)
    : _impl {std::make_unique<detail::b2d_debug_draw>(this, settings)}
{
}

debug_draw::~debug_draw() = default;

}

#endif
