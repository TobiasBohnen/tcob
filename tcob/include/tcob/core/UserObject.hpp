// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <variant>

#include "tcob/core/Concepts.hpp"

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
    auto is() const -> bool;

    template <typename T>
    auto get() -> T*;

    template <typename T>
    auto get() const -> T const*;

    auto has_value() const -> bool;
    auto type() const -> std::type_index;
    void reset();

private:
    static constexpr usize SBO_SIZE {2 * sizeof(void*)};
    using sbo_buffer = std::array<std::byte, SBO_SIZE>;

    std::variant<std::monostate, std::shared_ptr<void>, sbo_buffer> _data;
    std::type_index                                                 _type {typeid(void)};
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
    if constexpr (SharedPtr<U>) {
        _data = value;
        _type = typeid(typename U::element_type);
    } else if constexpr (sizeof(U) <= SBO_SIZE && POD<U>) {
        sbo_buffer data {};
        memcpy(data.data(), &value, sizeof(U));
        _data = data;
        _type = typeid(U);
    } else {
        _data = std::make_shared<U>(std::forward<T>(value));
        _type = typeid(U);
    }
    return *this;
}

template <typename T>
inline auto user_object::is() const -> bool
{
    using U = std::remove_const_t<T>;
    return has_value() && _type == typeid(U);
}

template <typename T>
inline auto user_object::get() -> T*
{
    return is<T>() ? overloaded_visit(
                         _data,
                         [](std::monostate) -> T* { return nullptr; },
                         [](std::shared_ptr<void>& val) -> T* { return static_cast<T*>(val.get()); },
                         [](sbo_buffer& val) -> T* { return reinterpret_cast<T*>(val.data()); })
                   : nullptr;
}

template <typename T>
inline auto user_object::get() const -> T const*
{
    return is<T>() ? overloaded_visit(
                         _data,
                         [](std::monostate) -> T const* { return nullptr; },
                         [](std::shared_ptr<void> const& val) -> T const* { return static_cast<T const*>(val.get()); },
                         [](sbo_buffer const& val) -> T const* { return reinterpret_cast<T const*>(val.data()); })
                   : nullptr;
}

}
