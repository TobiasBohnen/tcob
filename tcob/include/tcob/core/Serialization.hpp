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
};

template <auto Ptr, auto Default>
struct optional_member {
    constexpr optional_member(utf8_string name)
        : Name {std::move(name)}
    {
    }

    utf8_string Name;
};

template <auto Get, auto Set>
struct computed_member {
    constexpr computed_member(utf8_string name)
        : Name {std::move(name)}
    {
    }

    utf8_string Name;
};

////////////////////////////////////////////////////////////

template <typename T, auto Ptr>
void constexpr get_member(member<Ptr> const& m, auto& target, T const& object)
{
    target[m.Name] = object.*Ptr;
}

template <typename T, auto Ptr, auto Default>
void constexpr get_member(optional_member<Ptr, Default> const& m, auto& target, T const& object)
{
    target[m.Name] = object.*Ptr;
}

template <typename T, auto Get, auto Set>
void constexpr get_member(computed_member<Get, Set> const& p, auto& target, T const& object)
{
    target[p.Name] = Get(object);
}

template <typename T, auto Ptr>
auto constexpr set_member(member<Ptr> const& m, auto const& source, T& object) -> bool
{
    return source.try_get(object.*Ptr, m.Name);
}

template <typename T, auto Ptr, auto Default>
auto constexpr set_member(optional_member<Ptr, Default> const& m, auto const& source, T& object) -> bool
{
    if (!source.try_get(object.*Ptr, m.Name)) {
        object.*Ptr = Default;
    }
    return true;
}

template <typename T, auto Get, auto Set>
auto constexpr set_member(computed_member<Get, Set> const& p, auto const& source, T& object) -> bool
{
    using FieldType = std::invoke_result_t<decltype(Get), T const&>;
    FieldType temp;
    if (!source.try_get(temp, p.Name)) { return false; }
    Set(object, temp);
    return true;
}

}
