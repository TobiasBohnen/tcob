// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Result.hpp"

namespace tcob::scripting {
////////////////////////////////////////////////////////////

enum class error_code : u8 {
    Ok,
    Undefined,
    TypeMismatch,
    NonTableIndex,
    Error
};

template <typename T>
using result = tcob::result<T, error_code>;

////////////////////////////////////////////////////////////

}
