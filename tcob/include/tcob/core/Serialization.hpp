// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <type_traits>
#include <utility>

#include "tcob/core/Common.hpp"
#include "tcob/core/Concepts.hpp"
#include "tcob/core/Property.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

struct no_default_t { };
inline constexpr no_default_t no_default {};

template <auto Ptr, auto Default = no_default>
struct member {
    using field_type = typename detail::member_pointer_traits<decltype(Ptr)>::field_type;

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
        if constexpr (PropertyLike<field_type>) {
            using prop_type = std::remove_cvref_t<typename field_type::return_type>;
            prop_type temp;
            if (proxy.try_get(temp)) {
                object.*Ptr = temp;
                return true;
            }
        } else {
            if (proxy.try_get(object.*Ptr)) {
                return true;
            }
        }

        if constexpr (!std::is_same_v<decltype(Default), no_default_t>) {
            object.*Ptr = Default;
            return true;
        } else {
            return false;
        }
    }
};

template <auto Get, auto Set>
struct member_fn {
    constexpr member_fn(utf8_string name)
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
