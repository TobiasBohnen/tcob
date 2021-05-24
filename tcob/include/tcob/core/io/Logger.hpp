// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <cstring>

#include <tcob/core/io/FileStream.hpp>

#ifdef _WIN32
constexpr auto PATH_SEP { '\\' };
#else
constexpr auto PATH_SEP { '/' };
#endif

#define __FILENAME__ (std::strrchr(__FILE__, PATH_SEP) ? std::strrchr(__FILE__, PATH_SEP) + 1 : __FILE__)

namespace tcob {
class Logger final {
public:
    static auto Instance() -> Logger&
    {
        static Logger instance;
        return instance;
    }

    void log(const std::string& message) const;
    void done();

private:
    Logger();
    ~Logger();

    std::unique_ptr<OutputFileStream> _logStream;
};

#define Log(X) Logger::Instance().log("[" + std::string(__FILENAME__) + "(" + std::to_string(__LINE__) + ")]: " + (X) + "\n");
}