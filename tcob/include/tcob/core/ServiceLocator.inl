// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ServiceLocator.hpp"

namespace tcob {

template <typename T>
inline void service_locator::set(std::shared_ptr<T> service)
{
    _services[T::service_name] = service;
}

template <typename T>
inline auto service_locator::get() const -> T*
{
    return static_cast<T*>(_services.at(T::service_name).get());
}

}
