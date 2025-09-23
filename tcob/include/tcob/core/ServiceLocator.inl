// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ServiceLocator.hpp"

#include <memory>

namespace tcob {

template <typename T>
inline void service_locator::set(std::shared_ptr<T> service)
{
    if (service) {
        _services[typeid(T).hash_code()] = service;
    } else {
        _services.erase(typeid(T).hash_code());
    }
}

template <typename T>
inline auto service_locator::get() const -> T*
{
    return static_cast<T*>(_services.at(typeid(T).hash_code()).get());
}

template <typename T>
inline auto service_locator::has() const -> bool
{
    return _services.contains(typeid(T).hash_code());
}

template <typename T>
inline auto service_locator::name() const -> string
{
    return T::ServiceName;
}

}
