// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <type_traits>
#include <utility>

#include "tcob/core/Concepts.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <auto Ptr>
struct member {
    constexpr member(utf8_string name)
        : Name {std::move(name)}
    {
    }

    utf8_string Name;

    template <typename T>
    void constexpr get(auto proxy, T const& object) const
    {
        proxy = object.*Ptr;
    }

    template <typename T>
    auto constexpr set(auto const& proxy, T& object) const -> bool
    {
        return proxy.try_get(object.*Ptr);
    }
};

template <auto Ptr, auto Default>
struct optional_member {
    constexpr optional_member(utf8_string name)
        : Name {std::move(name)}
    {
    }

    utf8_string Name;

    template <typename T>
    void constexpr get(auto proxy, T const& object) const
    {
        proxy = object.*Ptr;
    }

    template <typename T>
    auto constexpr set(auto const& proxy, T& object) const -> bool
    {
        if (!proxy.try_get(object.*Ptr)) {
            object.*Ptr = Default;
        }
        return true;
    }
};

template <auto Get, auto Set>
struct computed_member {
    constexpr computed_member(utf8_string name)
        : Name {std::move(name)}
    {
    }

    utf8_string Name;

    template <typename T>
    void constexpr get(auto proxy, T const& object) const
    {
        proxy = Get(object);
    }

    template <typename T>
    auto constexpr set(auto const& proxy, T& object) const -> bool
    {
        using FieldType = std::invoke_result_t<decltype(Get), T const&>;
        FieldType temp;
        if (!proxy.try_get(temp)) { return false; }
        Set(object, temp);
        return true;
    }
};

////////////////////////////////////////////////////////////

}
