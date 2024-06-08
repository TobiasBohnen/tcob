// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <cassert>
#include <unordered_map>

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API service_locator final {
public:
    service_locator();
    service_locator(service_locator const& other) noexcept                    = delete;
    auto operator=(service_locator const& other) noexcept -> service_locator& = delete;
    ~service_locator();

    auto static GetInstance() -> service_locator&;

    template <typename T>
    void set(std::shared_ptr<T> service);

    template <typename T>
    auto get() const -> T*;

private:
    std::unordered_map<char const*, std::shared_ptr<void>> _services;
};

template <typename T, typename R = T>
auto register_service(std::shared_ptr<R> service = std::make_shared<R>()) -> std::shared_ptr<R>
{
    service_locator::GetInstance().set<T>(service);
    return service;
}

template <typename T>
auto locate_service() -> T&
{
    auto* retValue {service_locator::GetInstance().get<T>()};
    assert(retValue);
    return *retValue;
}

template <typename T>
void remove_service()
{
    service_locator::GetInstance().set<T>(nullptr);
}

}

#include "ServiceLocator.inl"
