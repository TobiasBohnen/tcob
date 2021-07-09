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

auto Config::operator=(const lua::Ref& other) -> Config&
{
    lua::Ref::operator=(other);
    return *this;
}

auto Config::load() -> bool
{
    Log("loading config", LogLevel::Info);
    std::string file;
    if (FileSystem::exists(CONFIG_FILE)) {
        file = CONFIG_FILE;
    } else {
        // load default
        Log("Config file not found! Loading defaults.", LogLevel::Warning);
        file = DEFAULT_CONFIG_FILE;
    }

    const auto result { _script.run_file<lua::Table>(file) };
    if (result.State == lua::ResultState::Ok) {
        *this = result.Value;
        return true;
    } else {
        *this = _script.global_table().create_table("Config");
        return false;
    }
}

void Config::save() const
{
    Log("saving config", LogLevel::Info);

    OutputFileStream s { CONFIG_FILE };
    s.write("Config = ");
    dump(s);
    s.write("\nreturn Config ");
}
}