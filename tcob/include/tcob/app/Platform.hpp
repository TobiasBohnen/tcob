// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <map>
#include <memory>
#include <vector>

#include "tcob/app/Game.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/data/ConfigFile.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

struct locale {
    string Language;
    string Country;
};

////////////////////////////////////////////////////////////

struct display_mode {
    size_i Size {size_i::Zero};
    i32    RefreshRate {0};
};

struct display {
    std::vector<display_mode> Modes;
    display_mode              DesktopMode;
};

////////////////////////////////////////////////////////////

class TCOB_API platform final : public non_copyable {
public:
    platform(bool headless, game::init const& ginit);
    ~platform();

    signal<path const> DropFile;

    auto config() const -> data::config_file&;

    auto preferred_locales() const -> std::vector<locale> const&;

    auto displays() const -> std::map<i32, display>;
    auto get_desktop_size(i32 display) const -> size_i;

    auto was_paused() const -> bool; // WINDOWS: true if window was dragged

    auto process_events() const -> bool;

    auto static HeadlessInit(char const* argv0, path logFile = "") -> platform;

    auto static IsRunningOnWine() -> bool;

    static inline char const* service_name {"platform"};

private:
    void init_locales();
    void init_render_system(string const& windowTitle);

    void remove_services() const;

    void on_key_down(input::keyboard::event const& ev);

    void static InitSDL();
    void static InitSignatures();
    void static InitConfigFormats();
    void static InitImageCodecs();
    void static InitAudioCodecs();
    void static InitFontEngines();

    std::vector<locale> _locales {};

    std::unique_ptr<data::config_file> _configFile;
    bool                               _wasPaused {false};
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
