// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <variant>

namespace tcob {
////////////////////////////////////////////////////////////

template <typename T, typename ErrorCode>
class [[nodiscard]] result {
public:
    using type = T;

    result() = default;
    explicit result(T&& value);
    explicit result(T const& value);
    explicit result(ErrorCode value);
    result(result const& other) noexcept;
    auto operator=(result const& other) noexcept -> result&;
    result(result&& other) noexcept;
    auto operator=(result&& other) noexcept -> result&;

    explicit operator bool() const;

    auto operator*() & -> T&;
    auto operator*() const& -> T const&;
    auto operator*() && -> T&&;
    auto operator*() const&& -> T const&&;
    auto operator->() const -> T const*;
    auto operator->() -> T*;

    auto value() & -> T&;
    auto value() const& -> T const&;
    auto value() && -> T&&;
    auto value() const&& -> T const&&;
    auto error() const -> ErrorCode;

    template <typename U>
    auto value_or(U&& defValue) const& -> T;
    template <typename U>
    auto value_or(U&& defValue) && -> T;

    auto has_value() const -> bool;
    auto has_error() const -> bool;

    template <typename F>
    auto and_then(F&& f) & -> result;
    template <typename F>
    auto and_then(F&& f) const& -> result;
    template <typename F>
    auto and_then(F&& f) && -> result;
    template <typename F>
    auto and_then(F&& f) const&& -> result;

    template <typename F>
    auto transform(F&& f) &;
    template <typename F>
    auto transform(F&& f) const&;
    template <typename F>
    auto transform(F&& f) &&;
    template <typename F>
    auto transform(F&& f) const&&;

    template <typename F>
    auto or_else(F&& f) const& -> result;
    template <typename F>
    auto or_else(F&& f) && -> result;

private:
    std::variant<T, ErrorCode> _value;
};

template <typename ErrorCode>
class [[nodiscard]] result<void, ErrorCode> {
public:
    result() = default;
    result(ErrorCode value);

    explicit operator bool() const;

    auto error() const -> ErrorCode;

    auto has_error() const -> bool;

private:
    std::optional<ErrorCode> _value;
};

}

#include "Result.inl"
