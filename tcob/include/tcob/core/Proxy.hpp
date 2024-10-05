// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Concepts.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <typename Object, typename... Keys>
class [[nodiscard]] proxy final {
public:
    proxy(Object& object, std::tuple<Keys...> keys);

    template <typename T>
    explicit operator T() const;

    auto operator=(auto&& other) -> proxy&;
    auto operator=(auto&& other) const -> proxy& = delete;

    template <typename Key>
    auto operator[](Key key) -> proxy<Object, Keys..., Key>;
    template <typename Key>
    auto operator[](Key key) const -> proxy<Object, Keys..., Key> const;

    template <typename T>
    auto as() const -> T;

    template <typename T>
    auto get() const; //-> result

    template <typename T>
    auto is() const -> bool;

    template <typename T>
    auto try_get(T& val) const -> bool;

    template <typename Key, typename T>
    auto try_get(T& val, Key key) const -> bool;

private:
    Object&             _object;
    std::tuple<Keys...> _keys;
};

}

#include "Proxy.inl"
