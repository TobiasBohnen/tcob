// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Lua.hpp"

#include <type_traits>
#include <utility>

namespace tcob::scripting {

template <ConvertibleTo... T>
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
    return converter<std::remove_cvref_t<T>>::From(*this, idx, value);
}

//////push//////
template <ConvertibleTo T>
inline void state_view::convert_to(T&& value) const
{
    converter<std::remove_cvref_t<T>>::To(*this, std::forward<T>(value));
}

////////////////////////////////////////////////////////////

template <typename WrappedType>
inline unknown_set_event<WrappedType>::unknown_set_event(WrappedType* instance, string name, state_view view)
    : Instance {instance}
    , Name {std::move(name)}
    , _view {view}
{
}

template <typename WrappedType>
template <typename T>
inline auto unknown_set_event<WrappedType>::get_value(T& val) -> bool
{
    if (converter<std::remove_cvref_t<T>>::IsType(_view, 2)) {
        if (_view.pull_convert_idx(2, val)) {
            Handled = true;
            return true;
        }
    }

    return false;
}

template <typename WrappedType>
inline unknown_get_event<WrappedType>::unknown_get_event(WrappedType* instance, string name, state_view view)
    : Instance {instance}
    , Name {std::move(name)}
    , _view {view}
{
}

template <typename WrappedType>
inline void unknown_get_event<WrappedType>::return_value(auto&& value)
{
    _view.push_convert(std::move(value));
    Handled = true;
}

}
