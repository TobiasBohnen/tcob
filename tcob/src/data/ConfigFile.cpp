// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/ConfigFile.hpp"

#include "tcob/core/Logger.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::data {

config_file::config_file(string file)
    : _fileName {std::move(file)}
{
    string configFile {_fileName};
    logger::Info("Config: loading '{}'", configFile);
    if (!io::is_file(configFile)) {
        configFile = io::get_stem(configFile) + ".default" + io::get_extension(configFile);
        logger::Warning("Config: File not found. Trying default file '{}'", configFile);
    }

    if (data::config::object::load(configFile) == load_status::Ok) {
        logger::Info("Config: loading completed");
    } else {
        logger::Warning("Config: loading failed");
    }
}

config_file::~config_file()
{
    save();
}

void config_file::save() const
{
    logger::Info("Config: saving '{}'", _fileName);
    data::config::object::save(_fileName);
    logger::Info("Config: saving completed");
}
}
