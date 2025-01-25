// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Logger.hpp"

namespace tcob {

inline void logger::Debug([[maybe_unused]] string const& message, [[maybe_unused]] auto&&... args)
{
#if defined(TCOB_DEBUG)
    FormatDebug(message, std::make_format_args(args...));
#endif
}

inline void logger::Info(string const& message, auto&&... args)
{
    FormatInfo(message, std::make_format_args(args...));
}

inline void logger::Warning(string const& message, auto&&... args)
{
    FormatWarning(message, std::make_format_args(args...));
}

inline void logger::Error(error_msg const& msg, auto&&... args)
{
    FormatError(msg, std::make_format_args(args...));
}

}
