// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <compare>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "tcob/app/Game.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"
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

class TCOB_API display_mode {
public:
    size_i Size {size_i::Zero};
    f32    PixelDensity {0.0f};
    f32    RefreshRate {0.0f};

    auto operator==(display_mode const& other) const -> bool = default;
    auto operator<=>(display_mode const& other) const -> std::partial_ordering;
};

struct display {
    std::set<display_mode, std::greater<>> Modes;
    display_mode                           DesktopMode;
};

////////////////////////////////////////////////////////////

class TCOB_API platform final : public non_copyable {
public:
    ~platform();

    signal<path const> DropFile;

    prop<i32> FrameLimit; //!< Property to control the frame rate limit.

    auto config() const -> data::config_file&;

    auto preferred_locales() const -> std::vector<locale> const&;

    auto displays() const -> std::map<i32, display>;
    auto get_desktop_mode(i32 display) const -> display_mode;

    auto was_paused() const -> bool; // WINDOWS: true if window was dragged

    auto process_events() const -> bool;

    auto static IsRunningOnWine() -> bool;

    auto static Init(game::init const& ginit) -> std::shared_ptr<platform>;
    auto static HeadlessInit(path const& logFile = "") -> std::shared_ptr<platform>;

    static inline char const* ServiceName {"platform"};

private:
    platform(bool headless, game::init const& ginit);

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
