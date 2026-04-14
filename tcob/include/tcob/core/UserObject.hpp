// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <type_traits>
#include <typeindex>
#include <utility>

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API user_object final {
public:
    user_object() = default;

    user_object(user_object const&)                        = default;
    user_object(user_object&&) noexcept                    = default;
    auto operator=(user_object const&) -> user_object&     = default;
    auto operator=(user_object&&) noexcept -> user_object& = default;

    template <typename T>
        requires(!std::is_same_v<std::decay_t<T>, user_object>)
    user_object(T&& value);

    template <typename T>
        requires(!std::is_same_v<std::decay_t<T>, user_object>)
    auto operator=(T&& value) -> user_object&;

    template <typename T>
    auto get() const -> T*;

    auto has_value() const -> bool;
    auto type() const -> std::type_index;
    void reset();

    template <typename T>
    static auto Make(auto&&... args) -> user_object;

private:
    user_object(std::shared_ptr<void> ptr, std::type_index type);

    std::shared_ptr<void> _data;
    std::type_index       _type {typeid(void)};
};

////////////////////////////////////////////////////////////

template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, user_object>)
inline user_object::user_object(T&& value)
{
    *this = std::forward<T>(value);
}

template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, user_object>)
inline auto user_object::operator=(T&& value) -> user_object&
{
    using U = std::decay_t<T>;

    _data = std::make_shared<U>(std::forward<T>(value));
    _type = typeid(U);
    return *this;
}

template <typename T>
inline auto user_object::get() const -> T*
{
    return _type == typeid(T) ? static_cast<T*>(_data.get()) : nullptr;
}

template <typename T>
inline auto user_object::Make(auto&&... args) -> user_object
{
    return {std::make_shared<T>(args...), typeid(T)};
}

}
