// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/ConfigFile.hpp"

#include <utility>

#include "tcob/core/Logger.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/data/ConfigTypes.hpp"

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

    if (data::object::load(configFile)) {
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
    if (data::object::save(_fileName)) {
        logger::Info("Config: saving completed");
    }
}
}
