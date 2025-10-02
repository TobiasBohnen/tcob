// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <type_traits>

#include "tcob/core/Concepts.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

namespace helper {

    TCOB_API auto round_to_multiple(i32 num, i32 step) -> i32;
    TCOB_API auto round_to_multiple(f32 num, f32 step) -> f32;
    TCOB_API auto round_up_to_multiple(i32 num, i32 step) -> i32;
    TCOB_API auto round_down_to_multiple(i32 num, i32 step) -> i32;

    TCOB_API auto extract_bits(u32 i, i32 offset, i32 count) -> u32;

    template <typename... Ts>
    auto constexpr hash_combine(usize seed, Ts... values) noexcept -> usize
    {
        constexpr usize magic {static_cast<usize>(0x9e3779b97f4a7c15ull)};
        ((seed ^= std::hash<Ts> {}(values) + magic + (seed << 6) + (seed >> 2)), ...);
        return seed;
    }

    auto erase_first(auto&& container, auto&& pred) -> bool
    {
        if (auto it {std::ranges::find_if(container, pred)}; it != container.end()) {
            container.erase(it);
            return true;
        }

        return false;
    }

    template <typename T>
    auto constexpr lerp(T from, T to, f64 step) -> T
    {
        if constexpr (Arithmetic<T>) {
            return static_cast<T>(std::lerp(static_cast<f64>(from), static_cast<f64>(to), step));
        } else {
            return (step < 0.5) ? from : to;
        }
    }
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

enum class playback_state : u8 {
    Running,
    Paused,
    Stopped
};

////////////////////////////////////////////////////////////

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <typename T, typename... Args>
struct arg_list;
template <typename T, typename... Args>
struct arg_list<T(Args...)> { };

////////////////////////////////////////////////////////////

namespace detail {

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

    ////////////////////////////////////////////////////////////

    template <typename T>
    struct member_pointer_traits;

    template <typename Class, typename Field>
    struct member_pointer_traits<Field Class::*> {
        using field_type = Field;
    };

}

////////////////////////////////////////////////////////////

struct locale {
    string Language;
    string Country;
};

}
