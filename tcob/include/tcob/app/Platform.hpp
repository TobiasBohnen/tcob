// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>

#include "tcob/app/Game.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/data/ConfigFile.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

struct locale {
    string Language;
    string Country;
};

////////////////////////////////////////////////////////////

class TCOB_API platform final : public non_copyable {
public:
    platform(game* game, game::init const& ginit);
    ~platform();

    auto has_window() const -> bool;
    auto get_window() const -> gfx::window&;
    auto get_default_target() const -> gfx::default_render_target&;

    void process_events() const;

    auto get_preferred_locales() const -> std::vector<locale> const&;

    auto static HeadlessInit(char const* argv0, path logFile = "") -> platform;

    auto static IsRunningOnWine() -> bool;

    static inline char const* service_name {"platform"};

private:
    void init_locales();
    void init_render_system(data::config_file const& config);

    void remove_services() const;

    void on_key_down(input::keyboard::event& ev);

    void static InitSDL();
    void static InitSignatures();
    void static InitConfigFormats();
    void static InitImageCodecs();
    void static InitAudioCodecs();
    void static InitFontEngines();

    game*               _game {nullptr};
    std::vector<locale> _locales {};

    std::unique_ptr<gfx::window>                _window;
    std::unique_ptr<gfx::default_render_target> _defaultTarget;
};

////////////////////////////////////////////////////////////

class TCOB_API single_instance {
public:
    single_instance(string const& name);
    ~single_instance();
    operator bool() const;

private:
    std::any _handle;
    bool     _locked {false};
};

}
