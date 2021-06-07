// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/Logger.hpp>

#include <tcob/core/io/FileSystem.hpp>

namespace tcob {
const std::string LOGFILE { "tcob.log" };

Logger::Logger()
    : _logStream { std::make_unique<OutputFileStream>(LOGFILE) }
{
}

Logger::~Logger() = default;

void Logger::log(const std::string& message, LogLevel level, std::source_location location) const
{
    switch (level) {
    case LogLevel::Debug:
#ifdef _DEBUG
        _logStream->write("Debug: ");
        break;
#else
        return;
#endif
    case LogLevel::Info:
        _logStream->write("Info: ");
        break;
    case LogLevel::Warning:
        _logStream->write("Warning: ");
        break;
    case LogLevel::Error:
        _logStream->write("Error: ");
        break;
    }

    _logStream->write(message);
    if (level != LogLevel::Info)
        _logStream->write(" @[" + FileSystem::stem(location.file_name()) + "(" + std::to_string(location.line()) + ")]");
    _logStream->write("\n");
    _logStream->flush();
}

void Logger::done()
{
    _logStream = nullptr;
}
}