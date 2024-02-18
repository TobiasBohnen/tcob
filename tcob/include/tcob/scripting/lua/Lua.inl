// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Lua.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

namespace tcob::scripting::lua {

template <typename... T>
inline void state_view::push_convert(T&&... t) const
{
    check_stack(sizeof...(t));

    (convert_to(std::forward<T>(t)), ...);
}

template <ConvertibleFrom T>
inline auto state_view::pull_convert(i32& idx, T& t) const -> bool
{
    return convert_from(idx, t);
}

template <ConvertibleFrom T>
inline auto state_view::pull_convert_idx(i32 idx, T& t) const -> bool
{
    i32 idx0 {idx};
    return convert_from(idx0, t);
}

//////get//////
template <ConvertibleFrom T>
inline auto state_view::convert_from(i32& idx, T& value) const -> bool
{
    return converter<T>::From(*this, idx, value);
}

//////push//////
template <ConvertibleTo T>
inline void state_view::convert_to(T const& value) const
{
    converter<T>::To(*this, value);
}

template <ConvertibleTo T>
inline void state_view::convert_to(T&& value) const
{
    converter<T>::To(*this, std::forward<T>(value));
}

template <ConvertibleTo T>
inline void state_view::convert_to(T& value) const
{
    converter<T>::To(*this, value);
}

}

#endif
