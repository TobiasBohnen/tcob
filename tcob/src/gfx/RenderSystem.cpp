// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/RenderSystem.hpp"

#include <cassert>
#include <memory>

#include "tcob/core/Size.hpp"
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

auto render_system::init_window(video_config const& config, string const& windowTitle, size_i desktopResolution) -> gfx::window&
{
    size_i const resolution {config.UseDesktopResolution ? desktopResolution : config.Resolution};
    _window = create_window(resolution);
    _window->FullScreen(config.FullScreen || config.UseDesktopResolution);
    _window->VSync(config.VSync);
    _window->Title = windowTitle;
    assert(_window->Size == resolution);

    _defaultTarget = std::make_unique<gfx::default_render_target>(_window.get());

    _window->clear();
    _window->draw_to(*_defaultTarget);
    _window->swap_buffer();

    return *_window;
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
