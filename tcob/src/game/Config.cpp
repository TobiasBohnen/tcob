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

auto Config::load() -> bool
{
    Log("loading config");
    std::string file;
    if (FileSystem::exists(CONFIG_FILE)) {
        file = CONFIG_FILE;
    } else {
        // load default
        Log("Warning: Config file not found! Loading defaults.");
        file = DEFAULT_CONFIG_FILE;
    }

    const auto result { _script.run_file<LuaTable>(file) };
    if (result.State == LuaResultState::Ok) {
        *this = result.Value;
        return true;
    } else {
        *this = _script.global_table().create_table("Config");
        return false;
    }
}

void Config::save() const
{
    Log("saving config");

    OutputFileStream s { CONFIG_FILE };
    s.write("Config = ");
    dump(s);
    s.write("\nreturn Config ");
}
}