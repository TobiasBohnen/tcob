// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/game/Config.hpp>

namespace tcob {
const std::string& CONFIG_FILE { "config.lua" };
const std::string& DEFAULT_CONFIG_FILE { "config.default.lua" };

Config::~Config()
{
    unref();
}

auto Config::operator=(const LuaRef& other) -> Config&
{
    LuaRef::operator=(other);
    return *this;
}

void Config::load()
{
    Log("loading config");
    std::string file;
    if (FileSystem::exists(CONFIG_FILE)) {
        file = CONFIG_FILE;
    } else {
        // load default
        Log("Config file not found! Loading default.");
        file = DEFAULT_CONFIG_FILE;
    }

    const auto result { _state.run_file<LuaTable>(file) };
    if (result.State == LuaResultState::Ok) {
        *this = result.Value;
    } else {
        Log("Error loading config file. Aborting.");
        std::terminate();
    }
}

void Config::save() const
{
    Log("saving config");

    OutputFileStream s { CONFIG_FILE };
    s.write("local config = ");
    dump(s);
    s.write("\nreturn config ");
}
}