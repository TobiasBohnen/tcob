// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <map>
#include <vector>

#include "tcob/app/Game.hpp"
#include "tcob/app/Platform.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API sdl_platform final : public platform {
public:
    sdl_platform(bool headless, game::init const& ginit);
    ~sdl_platform() override;

    auto displays() const -> std::map<i32, gfx::display> override;
    auto get_desktop_mode(i32 display) const -> gfx::display_mode override;

    auto preferred_locales() const -> std::vector<locale> const& override;

    auto window_freezed() const -> bool override; // WINDOWS: true if window was dragged

    auto process_events() const -> bool override;

private:
    void init_locales();

    void init_audio_system();
    void init_render_system(string const& windowTitle);
    void init_input_system();

    void static InitSDL();

    std::vector<locale> _locales {};

    bool _wasPaused {false};
};

}
