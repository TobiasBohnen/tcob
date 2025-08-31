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

    virtual auto displays() const -> std::map<i32, gfx::display>          = 0;
    virtual auto get_desktop_mode(i32 display) const -> gfx::display_mode = 0;

    virtual auto preferred_locales() const -> std::vector<locale> const& = 0;

    virtual auto window_freezed() const -> bool = 0; // WINDOWS: true if window was dragged

    virtual auto process_events() const -> bool = 0;

    static auto IsRunningOnWine() -> bool;

    static inline char const* ServiceName {"platform"};

    static auto Init(game::init const& ginit) -> std::shared_ptr<platform>;
    static auto HeadlessInit(path const& logFile = "") -> std::shared_ptr<platform>;

protected:
    platform(bool headless, game::init const& ginit);

    static void InitSignatures();
    static void InitConfigFormats();
    static void InitAssetFormats();
    static void InitImageCodecs();
    static void InitAudioCodecs();
    static void InitFontEngines();
    static void InitTaskManager(std::optional<isize> workerThreads);

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
