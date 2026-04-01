// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <tuple>
#include <type_traits>

#include "tcob/core/Common.hpp"
#include "tcob/core/Concepts.hpp"
#include "tcob/core/Property.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

struct no_default_t { };
inline constexpr no_default_t no_default {};

template <auto Ptr, auto Default = no_default>
struct member {
    using pointer_type = decltype(Ptr);
    using field_type   = typename detail::member_pointer_traits<pointer_type>::field_type;

    template <typename... Aliases>
    constexpr member(utf8_string_view name, Aliases... aliases)
        : Names {name, aliases...}
        , NameCount {1 + sizeof...(aliases)}
    {
    }

    static constexpr usize                 MaxNames {4};
    std::array<utf8_string_view, MaxNames> Names;
    usize                                  NameCount;

    constexpr auto primary_name() const -> utf8_string_view { return Names[0]; }

    auto get(auto&& object) const -> decltype(auto)
    {
        return object.*Ptr;
    }

    void to_proxy(auto&& proxy, auto&& object) const
    {
        proxy = get(object);
    }

    void set(auto&& object, auto&& value) const
    {
        object.*Ptr = value;
    }

    auto from_proxy(auto const& proxy, auto&& object) const -> bool
    {
        if constexpr (PropertyLike<field_type>) {
            using prop_type = std::remove_cvref_t<typename field_type::return_type>;
            prop_type temp;
            if (proxy.try_get(temp)) {
                set(object, temp);
                return true;
            }
        } else {
            if (proxy.try_get(object.*Ptr)) {
                return true;
            }
        }
        if constexpr (!std::is_same_v<decltype(Default), no_default_t>) {
            set(object, Default);
            return true;
        } else {
            return false;
        }
    }
};

template <auto... Ptrs>
constexpr auto make_members(auto&&... names)
{
    return std::tuple {member<Ptrs> {names}...};
}

////////////////////////////////////////////////////////////

template <auto Get, auto Set>
struct member_fn {
    template <typename... Aliases>
    constexpr member_fn(utf8_string_view name, Aliases... aliases)
        : Names {name, aliases...}
        , NameCount {1 + sizeof...(aliases)}
    {
    }

    static constexpr usize                 MaxNames {4};
    std::array<utf8_string_view, MaxNames> Names;
    usize                                  NameCount;

    constexpr auto primary_name() const -> utf8_string_view { return Names[0]; }

    auto get(auto&& object) const -> decltype(auto)
    {
        return Get(object);
    }

    void to_proxy(auto&& proxy, auto&& object) const
    {
        proxy = get(object);
    }

    void set(auto&& object, auto&& value) const
    {
        Set(object, value);
    }

    auto from_proxy(auto const& proxy, auto&& object) const -> bool
    {
        using FieldType = std::invoke_result_t<decltype(Get), decltype(object) const&>;
        FieldType temp;
        if (!proxy.try_get(temp)) { return false; }
        set(object, temp);
        return true;
    }
};

////////////////////////////////////////////////////////////

}
