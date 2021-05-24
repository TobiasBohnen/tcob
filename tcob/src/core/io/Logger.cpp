// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/Logger.hpp>

namespace tcob {
const std::string LOGFILE { "tcob.log" };

Logger::Logger()
    : _logStream { std::make_unique<OutputFileStream>(LOGFILE) }
{
    // std::cout.rdbuf(_logStream->rdbuf());
}

Logger::~Logger()
    = default;

void Logger::log(const std::string& message) const
{
    _logStream->write(message);
    _logStream->flush();
}

void Logger::done()
{
    _logStream.reset();
}
}