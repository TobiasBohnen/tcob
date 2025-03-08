// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <format>
#include <memory>
#include <source_location>

#include "tcob/core/Interfaces.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API logger : public non_copyable {
public:
    class TCOB_API error_msg {
    public:
        error_msg(char const* msg, std::source_location loc = std::source_location::current());
        error_msg(string msg, std::source_location loc = std::source_location::current());

        string               Message;
        std::source_location SourceLocation;
    };

    enum class level : u8 {
        Debug   = 0,
        Info    = 1,
        Warning = 2,
        Error   = 3,
        Off     = 4
    };

    virtual ~logger();

    void static Debug(string const& message, auto&&... args);
    void static Debug(string const& message);
    void static Info(string const& message, auto&&... args);
    void static Info(string const& message);
    void static Warning(string const& message, auto&&... args);
    void static Warning(string const& message);
    void static Error(error_msg const& msg, auto&&... args);
    void static Error(error_msg const& msg);

    level MinLevel {level::Debug};

    static inline char const* service_name {"logger"};

protected:
    void virtual log(string const& message, level level) const = 0;
    auto format_message(string const& message, level level) const -> string;

private:
    void static FormatDebug(string const& message, std::format_args const& args);
    void static FormatInfo(string const& message, std::format_args const& args);
    void static FormatWarning(string const& message, std::format_args const& args);
    void static FormatError(error_msg const& msg, std::format_args const& args);

    void static Log(string const& message, level level);
};

////////////////////////////////////////////////////////////

class TCOB_API file_logger final : public logger {
public:
    explicit file_logger(path const& logfile);
    ~file_logger() override;

protected:
    void log(string const& message, level level) const override;

private:
    std::unique_ptr<io::ofstream> _logStream;
};

////////////////////////////////////////////////////////////

class TCOB_API null_logger final : public logger {
public:
    null_logger();

protected:
    void log(string const& message, level level) const override;
};

////////////////////////////////////////////////////////////

class TCOB_API stdout_logger final : public logger {
public:
    stdout_logger();

protected:
    void log(string const& message, level level) const override;
};

}

#include "Logger.inl"
