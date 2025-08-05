// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "tcob/app/Game.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/data/ConfigFile.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API platform final : public non_copyable {
public:
    ~platform();

    signal<path const> DropFile;

    prop<i32> FrameLimit;

    auto config() const -> data::config_file&;

    auto preferred_locales() const -> std::vector<locale> const&;

    auto displays() const -> std::map<i32, gfx::display>;
    auto get_desktop_mode(i32 display) const -> gfx::display_mode;

    auto was_paused() const -> bool; // WINDOWS: true if window was dragged

    auto process_events() const -> bool;

    auto static IsRunningOnWine() -> bool;

    auto static Init(game::init const& ginit) -> std::shared_ptr<platform>;
    auto static HeadlessInit(path const& logFile = "") -> std::shared_ptr<platform>;

    static inline char const* ServiceName {"platform"};

private:
    platform(bool headless, game::init const& ginit);

    void init_locales();

    void init_audio_system();
    void init_render_system(string const& windowTitle);
    void init_input_system();

    void remove_services() const;

    void on_key_down(input::keyboard::event const& ev);

    void static InitSDL();
    void static InitSignatures();
    void static InitConfigFormats();
    void static InitAssetFormats();
    void static InitImageCodecs();
    void static InitAudioCodecs();
    void static InitFontEngines();
    void static InitTaskManager(std::optional<isize> workerThreads);

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
