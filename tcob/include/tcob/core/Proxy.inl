// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Proxy.hpp"

namespace tcob {

template <typename Object, typename... Keys>
inline proxy<Object, Keys...>::proxy(Object& object, std::tuple<Keys...> keys)
    : _object {object}
    , _keys {std::move(keys)}
{
}

template <typename Object, typename... Keys>
template <typename T>
inline proxy<Object, Keys...>::operator T() const
{
    return as<T>();
}

template <typename Object, typename... Keys>
inline auto proxy<Object, Keys...>::operator=(auto&& other) -> proxy&
{
    std::apply([&](auto&&... args) { return _object.set(args..., std::move(other)); }, _keys);
    return *this;
}

template <typename Object, typename... Keys>
template <typename Key>
inline auto proxy<Object, Keys...>::operator[](Key key) -> proxy<Object, Keys..., Key>
{
    return proxy<Object, Keys..., Key>(_object, std::tuple_cat(_keys, std::tuple {key}));
}

template <typename Object, typename... Keys>
template <typename Key>
inline auto proxy<Object, Keys...>::operator[](Key key) const -> proxy<Object, Keys..., Key> const
{
    return proxy<Object, Keys..., Key>(_object, std::tuple_cat(_keys, std::tuple {key}));
}

template <typename Object, typename... Keys>
template <typename T>
inline auto proxy<Object, Keys...>::as() const -> T
{
    return get<T>().value();
}

template <typename Object, typename... Keys>
template <typename T>
inline auto proxy<Object, Keys...>::get() const
{
    return std::apply([&](auto&&... args) { return _object.template get<T>(args...); }, _keys);
}

template <typename Object, typename... Keys>
template <typename T>
inline auto proxy<Object, Keys...>::is() const -> bool
{
    return std::apply([&](auto&&... args) { return _object.template is<T>(args...); }, _keys);
}

template <typename Object, typename... Keys>
template <typename T>
inline auto proxy<Object, Keys...>::try_get(T& val) const -> bool
{
    if (auto const newval {get<T>()}) {
        val = *newval;
        return true;
    }

    return false;
}

template <typename Object, typename... Keys>
template <typename Key, typename T>
inline auto proxy<Object, Keys...>::try_get(T& val, Key key) const -> bool
{
    if (auto const newval {operator[](key).template get<T>()}) {
        val = *newval;
        return true;
    }

    return false;
}

}
