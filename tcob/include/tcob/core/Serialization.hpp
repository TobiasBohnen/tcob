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
    void constexpr get(auto& target, T const& object) const
    {
        target[Name] = object.*Ptr;
    }

    template <typename T>
    auto constexpr set(auto const& source, T& object) const -> bool
    {
        return source.try_get(object.*Ptr, Name);
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
    void constexpr get(auto& target, T const& object) const
    {
        target[Name] = object.*Ptr;
    }

    template <typename T>
    auto constexpr set(auto const& source, T& object) const -> bool
    {
        if (!source.try_get(object.*Ptr, Name)) {
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
    void constexpr get(auto& target, T const& object) const
    {
        target[Name] = Get(object);
    }

    template <typename T>
    auto constexpr set(auto const& source, T& object) const -> bool
    {
        using FieldType = std::invoke_result_t<decltype(Get), T const&>;
        FieldType temp;
        if (!source.try_get(temp, Name)) { return false; }
        Set(object, temp);
        return true;
    }
};

////////////////////////////////////////////////////////////

}
