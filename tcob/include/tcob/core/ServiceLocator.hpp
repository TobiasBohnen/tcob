// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <cassert>
#include <memory>
#include <typeindex>
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
    std::unordered_map<std::type_index, std::shared_ptr<void>> _services;
};

template <typename T, typename R = T>
auto register_service(std::shared_ptr<R> service) -> R&
{
    service_locator::GetInstance().set<T>(service);
    return *service;
}

template <typename T>
auto make_service() -> T&
{
    return register_service<T>(std::make_shared<T>());
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

template <typename Factory, typename... Args>
auto create_from_factory(string const& name, Args&&... args)
{
    return locate_service<typename Factory::factory>().create(name, std::forward<Args>(args)...);
}

template <typename Factory, typename... Args>
auto create_from_factory(io::istream& in, string const& fallback, Args&&... args)
{
    return locate_service<typename Factory::factory>().create_from_magic(in, fallback, std::forward<Args>(args)...);
}

}

#include "ServiceLocator.inl"
