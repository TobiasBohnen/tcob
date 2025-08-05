// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include <SDL3/SDL.h>

#include "tcob/core/Size.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API sdl_window final : public window {
public:
    explicit sdl_window(std::unique_ptr<render_backend::window_base> win);

    void load_icon(path const& file) override;

    auto has_focus() const -> bool override;
    void grab_input(bool grab) override;

    void process_events(void* ev) override;

protected:
    auto get_size() const -> size_i override;
    void set_size(size_i newsize) override;

private:
    auto get_fullscreen() const -> bool override;
    void set_fullscreen(bool value) override;

    auto get_title() const -> string override;
    void set_title(string const& value) override;

    SDL_Window* _handle;
};

}
