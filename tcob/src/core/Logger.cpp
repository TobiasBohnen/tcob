// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Logger.hpp"

#include <iostream>
#include <version>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

#if defined(_MSC_VER)
    #define WIN32_LEAN_AND_MEAN
    #include "Windows.h"
#endif

namespace tcob {

logger::~logger() = default;

void logger::FormatDebug(string const& message, std::format_args const& args)
{
    Debug(std::vformat(message, args));
}

void logger::Debug([[maybe_unused]] string const& message)
{
#if defined(TCOB_DEBUG)
    Log(message, level::Debug);
#endif
}

void logger::FormatInfo(string const& message, std::format_args const& args)
{
    Info(std::vformat(message, args));
}

void logger::Info(string const& message)
{
    Log(message, level::Info);
}

void logger::FormatWarning(string const& message, std::format_args const& args)
{
    Warning(std::vformat(message, args));
}

void logger::Warning(string const& message)
{
    Log(message, level::Warning);
}

void logger::FormatError(error_msg const& msg, std::format_args const& args)
{
    Error({std::vformat(msg.Message, args), msg.SourceLocation});
}

void logger::Error(error_msg const& msg)
{
    string const source {" @[" + io::get_stem(msg.SourceLocation.file_name()) + "(" + std::to_string(msg.SourceLocation.line()) + ")]"};
    Log(msg.Message + source, level::Error);
}

void logger::Log(string const& message, level level)
{
    locate_service<logger>().log(message, level);
}

auto logger::format_message(string const& message, level level) const -> string
{
    string prefix;

    switch (level) {
    case level::Debug:
        prefix = "[debug] ";
        break;
    case level::Info:
        prefix = "[info]  ";
        break;
    case level::Warning:
        prefix = "[warn]  ";
        break;
    case level::Error:
        prefix = "[error] ";
        break;
    case level::Off:
        break;
    }
#if __cpp_lib_chrono >= 201907L
    auto const time {std::chrono::current_zone()->to_local(std::chrono::system_clock::now())};
    return std::format("{} ({:%F - %T}) {}\n", prefix, time, message);
#else
    return std::format("{} {}\n", prefix, message);
#endif
}

////////////////////////////////////////////////////////////

logger::error_msg::error_msg(char const* msg, std::source_location loc)
    : Message {msg}
    , SourceLocation {loc}
{
}

logger::error_msg::error_msg(string msg, std::source_location loc)
    : Message {std::move(msg)}
    , SourceLocation {loc}
{
}

////////////////////////////////////////////////////////////

file_logger::file_logger(path const& logfile)
    : _logStream {std::make_unique<io::ofstream>(logfile, 0)}
{
}

void file_logger::log(string const& message, level level) const
{
    if (level < MinLevel) {
        return;
    }

    *_logStream << format_message(message, level);
}

////////////////////////////////////////////////////////////

null_logger::null_logger() = default;

void null_logger::log(string const&, level) const
{
}

////////////////////////////////////////////////////////////

stdout_logger::stdout_logger() = default;

void stdout_logger::log(string const& message, level level) const
{
#if defined(_MSC_VER)
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif

    if (level < MinLevel) { return; }

#if !defined(__EMSCRIPTEN__)
    std::cout << "\033[";

    switch (level) {
    case logger::level::Debug:
        std::cout << "36";
        break;
    case logger::level::Info:
        std::cout << "32";
        break;
    case logger::level::Warning:
        std::cout << "33";
        break;
    case logger::level::Error:
        std::cout << "31";
        break;
    case logger::level::Off: break;
    }

    std::cout << "m"
              << format_message(message, level)
              << "\033[0m";
#else
    std::cout << format_message(message, level);
#endif
}

}
