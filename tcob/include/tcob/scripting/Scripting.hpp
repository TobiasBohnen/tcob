// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

namespace tcob::scripting {
////////////////////////////////////////////////////////////

enum class error_code : u8 {
    Undefined,
    TypeMismatch,
    NonTableIndex,
    Error
};

////////////////////////////////////////////////////////////

template <typename T>
struct parameter_pack final {
    std::vector<T> Items;
};

////////////////////////////////////////////////////////////

template <typename T>
struct managed_ptr {
    explicit managed_ptr(T* obj)
        : Pointer {obj}
    {
    }

    T* Pointer {nullptr};
};

}
