// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>

#include "tcob/app/Game.hpp"
#include "tcob/core/Interfaces.hpp"

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

    void process_events(gfx::window* window) const;

    auto get_preferred_locales() const -> std::vector<locale> const&;
    auto get_display_size(i32 display) const -> size_i;

    void remove_services() const;

    auto static HeadlessInit(char const* argv0, path logFile = "") -> platform;

    auto static IsRunningOnWine() -> bool;

    static inline char const* service_name {"platform"};

private:
    void init_locales();

    void static InitSDL();
    void static InitSignatures();
    void static InitConfigFormats();
    void static InitImageCodecs();
    void static InitAudioCodecs();
    void static InitFontEngines();
    void static InitRenderSystem(string const& renderer);

    game*               _game {nullptr};
    std::vector<locale> _locales {};
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
