// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <source_location>

#include <tcob/core/io/FileStream.hpp>

namespace tcob {
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger final {
public:
    static auto Instance() -> Logger&
    {
        static Logger instance;
        return instance;
    }

    void log(const std::string& message, LogLevel level, std::source_location location) const;
    void done();

private:
    Logger();
    ~Logger();

    std::unique_ptr<OutputFileStream> _logStream;
};

#define Log(X, Y) Logger::Instance().log((X), (Y), std::source_location::current());
}