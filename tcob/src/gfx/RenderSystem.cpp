// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/RenderSystem.hpp"

#include <memory>

#include "tcob/core/Stats.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob::gfx {

render_system::render_system() = default;

render_system::~render_system()
{
    _window        = nullptr;
    _defaultTarget = nullptr;
}

auto render_system::stats() -> statistics&
{
    return _stats;
}

auto render_system::window() const -> gfx::window&
{
    return *_window;
}

auto render_system::default_target() const -> gfx::default_render_target&
{
    return *_defaultTarget;
}
}
