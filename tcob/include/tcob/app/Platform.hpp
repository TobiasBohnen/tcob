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
#include "tcob/data/ConfigFile.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API platform : public non_copyable {
public:
    virtual ~platform();

    signal<path const> DropFile;

    prop<i32> FrameLimit;

    auto config() const -> data::config_file&;

    auto virtual displays() const -> std::map<i32, gfx::display>          = 0;
    auto virtual get_desktop_mode(i32 display) const -> gfx::display_mode = 0;

    auto virtual preferred_locales() const -> std::vector<locale> const& = 0;

    auto virtual window_freezed() const -> bool = 0; // WINDOWS: true if window was dragged

    auto virtual process_events() const -> bool = 0;

    auto static IsRunningOnWine() -> bool;

    static inline char const* ServiceName {"platform"};

    auto static Init(game::init const& ginit) -> std::shared_ptr<platform>;
    auto static HeadlessInit(path const& logFile = "") -> std::shared_ptr<platform>;

protected:
    platform(bool headless, game::init const& ginit);

    void static InitSignatures();
    void static InitConfigFormats();
    void static InitAssetFormats();
    void static InitImageCodecs();
    void static InitAudioCodecs();
    void static InitFontEngines();
    void static InitTaskManager(std::optional<isize> workerThreads);

private:
    void remove_services() const;

    std::unique_ptr<data::config_file> _configFile;
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
