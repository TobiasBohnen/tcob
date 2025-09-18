// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <cassert>
#include <memory>
#include <unordered_map>

#include "tcob/core/Interfaces.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API service_locator final : public non_copyable {
public:
    service_locator()  = default;
    ~service_locator() = default;

    static auto GetInstance() -> service_locator&;

    template <typename T>
    void set(std::shared_ptr<T> service);

    template <typename T>
    auto get() const -> T*;

    template <typename T>
    auto has() const -> bool;

    template <typename T>
    auto name() const -> string;

private:
    std::unordered_map<usize, std::shared_ptr<void>> _services;
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
auto has_service() -> bool
{
    return service_locator::GetInstance().has<T>();
}

template <typename T>
void remove_service()
{
    service_locator::GetInstance().set<T>(nullptr);
}

}

#include "ServiceLocator.inl"
