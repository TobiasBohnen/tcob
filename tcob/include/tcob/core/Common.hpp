// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <algorithm>
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

    TCOB_API auto get_bits(u32 i, i32 offset, i32 count) -> u32;

    template <typename... Ts>
    auto constexpr hash_combine(usize seed, Ts... values) noexcept -> usize
    {
        constexpr usize magic {sizeof(usize) == 8 ? 0x9e3779b97f4a7c15ULL : 0x9e3779b9U};
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

    template <auto... MemberPointers>
    class member_ptr {
    public:
        void set(auto&& instance, auto const& value) const
        {
            get(instance) = value;
        }

        auto get(auto&& instance) const -> auto&
        {
            return Get(instance, MemberPointers...);
        }

    private:
        template <typename... Rest>
        auto static Get(auto&& parent, auto&& child, Rest... rest) -> auto&
        {
            if constexpr (sizeof...(Rest) == 0) {
                return parent.*child;
            } else {
                return Get(parent.*child, rest...);
            }
        }
    };
}

////////////////////////////////////////////////////////////

struct locale {
    string Language;
    string Country;
};

}
