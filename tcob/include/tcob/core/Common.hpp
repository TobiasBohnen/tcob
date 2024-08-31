// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

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

namespace detail {
    template <typename... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };

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

    ////////////////////////////////////////////////////////////

    template <typename T>
    class counting_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = T;
        using pointer           = T const*;
        using reference         = T;

        counting_iterator() = default;

        counting_iterator(T value)
            : _value {value}
        {
        }

        auto operator*() const -> T const&
        {
            return _value;
        }

        void operator++()
        {
            ++_value;
        }

        auto operator!=(counting_iterator const& other) const -> bool
        {
            return _value != other._value;
        }

        auto operator+(counting_iterator const& other) const -> T
        {
            return _value + other._value;
        }

        auto operator-(counting_iterator const& other) const -> T
        {
            return _value - other._value;
        }

        auto operator[](T x) -> T
        {
            return _value + x;
        }

    private:
        T _value {};
    };

    ////////////////////////////////////////////////////////////

    template <typename T>
    inline void hash_combine(std::size_t& seed, T const& v)
    {
        seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // Recursive template code derived from Matthieu M.
    template <typename Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
    struct hash_value_impl {
        void static apply(size_t& seed, Tuple const& tuple)
        {
            hash_value_impl<Tuple, Index - 1>::apply(seed, tuple);
            hash_combine(seed, std::get<Index>(tuple));
        }
    };

    template <typename Tuple>
    struct hash_value_impl<Tuple, 0> {
        void static apply(size_t& seed, Tuple const& tuple)
        {
            hash_combine(seed, std::get<0>(tuple));
        }
    };

    template <typename T>
    struct tuple_hasher {
        auto operator()(T const& tt) const -> size_t
        {
            size_t seed = 0;
            hash_value_impl<T>::apply(seed, tt);
            return seed;
        }
    };
}

////////////////////////////////////////////////////////////
}
