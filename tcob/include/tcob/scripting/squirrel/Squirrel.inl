// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Squirrel.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

namespace tcob::scripting::squirrel {

template <typename... T>
inline void vm_view::push_convert(T&&... t) const
{
    (convert_to(std::forward<T>(t)), ...);
}

template <ConvertibleFrom T>
inline auto vm_view::pull_convert(SQInteger& idx, T& t) const -> bool
{
    return convert_from(idx, t);
}

template <ConvertibleFrom T>
inline auto vm_view::pull_convert_idx(SQInteger idx, T& t) const -> bool
{
    SQInteger idx0 {idx};
    return convert_from(idx0, t);
}

//////get//////
template <ConvertibleFrom T>
inline auto vm_view::convert_from(SQInteger& idx, T& value) const -> bool
{
    return converter<T>::From(*this, idx, value);
}

//////push//////
template <ConvertibleTo T>
inline void vm_view::convert_to(T const& value) const
{
    converter<T>::To(*this, value);
}

template <ConvertibleTo T>
inline void vm_view::convert_to(T&& value) const
{
    converter<T>::To(*this, std::forward<T>(value));
}

template <ConvertibleTo T>
inline void vm_view::convert_to(T& value) const
{
    converter<T>::To(*this, value);
}

}

#endif
