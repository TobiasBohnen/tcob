// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <type_traits>
#include <utility>
#include <variant>

#include "tcob/core/Concepts.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

namespace helper {
    TCOB_API auto byteswap(u16 val) -> u16;
    TCOB_API auto byteswap(u32 val) -> u32;
    TCOB_API auto byteswap(u64 val) -> u64;

    template <POD T>
    auto byteswap(T val) -> T
    {
        usize const size {sizeof(T)};
        static_assert(size % 2 == 0);
        byte* dst {reinterpret_cast<byte*>(&val)};

        for (usize i {0}; i < size / 2; ++i) {
            std::swap(dst[i], dst[size - i - 1]);
        }

        return val;
    }

    TCOB_API auto round_to_multiple(i32 num, i32 step) -> i32;
    TCOB_API auto round_up_to_multiple(i32 num, i32 step) -> i32;
    TCOB_API auto round_down_to_multiple(i32 num, i32 step) -> i32;

    TCOB_API auto get_bits(u32 i, i32 offset, i32 count) -> u32;
}

////////////////////////////////////////////////////////////

namespace enum_ops {

    template <Enum T>
    auto operator|(T lhs, T rhs) -> T
    {
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) | static_cast<std::underlying_type_t<T>>(rhs));
    }

    template <Enum T>
    auto operator&(T lhs, T rhs) -> T
    {
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) & static_cast<std::underlying_type_t<T>>(rhs));
    }

}

////////////////////////////////////////////////////////////

enum class load_status : u8 {
    Ok,
    FileNotFound,
    Error
};

////////////////////////////////////////////////////////////

enum class direction : u8 {
    None,
    Left,
    Right,
    Up,
    Down
};

////////////////////////////////////////////////////////////

enum class playback_mode : u8 {
    Normal,
    Reversed,
    Looped,
    ReversedLooped,
    Alternated,
    AlternatedLooped
};

////////////////////////////////////////////////////////////

enum class playback_status : u8 {
    Running,
    Paused,
    Stopped
};

////////////////////////////////////////////////////////////

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

namespace detail {
    ////////////////////////////////////////////////////////////

    template <typename T, typename... Ts>
    struct first_element {
        using type = T;
    };

    template <typename... Ts>
    using first_element_t = typename first_element<Ts...>::type;

    template <typename T, typename... Ts>
    struct last_element {
        using type = typename last_element<Ts...>::type;
    };

    template <typename T>
    struct last_element<T> {
        using type = T;
    };

    template <typename... Ts>
    using last_element_t = typename last_element<Ts...>::type;

}

////////////////////////////////////////////////////////////
}
