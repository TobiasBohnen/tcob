// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <typename Object, typename... Keys>
class [[nodiscard]] proxy final {
public:
    proxy(Object& object, std::tuple<Keys...> keys);

    auto operator=(auto&& other) -> proxy&;

    template <typename Key>
    auto operator[](Key key) const -> proxy<Object, Keys..., Key>;

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
