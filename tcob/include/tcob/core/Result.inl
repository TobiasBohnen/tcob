// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Result.hpp"

#include <cassert>

namespace tcob {

template <typename T, typename ErrorCode>
inline result<T, ErrorCode>::result(T&& value)
    : _value {std::in_place_type<T>, std::move(value)}
{
}

template <typename T, typename ErrorCode>
inline result<T, ErrorCode>::result(T const& value)
    : _value {std::in_place_type<T>, value}
{
}

template <typename T, typename ErrorCode>
inline result<T, ErrorCode>::result(ErrorCode value)
    : _value {value}
{
    assert(value != ErrorCode::Ok);
}

template <typename T, typename ErrorCode>
inline result<T, ErrorCode>::result(result const& other) noexcept
{
    *this = other;
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::operator=(result const& other) noexcept -> result&
{
    _value = std::move(other._value);
    return *this;
}

template <typename T, typename ErrorCode>
inline result<T, ErrorCode>::result(result&& other) noexcept
    : _value {std::exchange(other._value, {})}
{
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::operator=(result&& other) noexcept -> result&
{
    std::swap(_value, other._value);
    return *this;
}

template <typename T, typename ErrorCode>
inline result<T, ErrorCode>::operator bool() const
{
    return has_value();
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::has_value() const -> bool
{
    return std::holds_alternative<T>(_value);
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::value() & -> T&
{
    return std::get<T>(_value);
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::value() const& -> T const&
{
    return std::get<T>(_value);
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::value() && -> T&&
{
    return std::move(std::get<T>(_value));
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::value() const&& -> T const&&
{
    return std::move(std::get<T>(_value));
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::operator*() & -> T&
{
    return value();
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::operator*() const& -> T const&
{
    return value();
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::operator*() && -> T&&
{
    return std::move(value());
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::operator*() const&& -> T const&&
{
    return std::move(value());
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::operator->() -> T*
{
    return std::get_if<T>(&_value);
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::operator->() const -> T const*
{
    return std::get_if<T>(&_value);
}

template <typename T, typename ErrorCode>
template <typename U>
inline auto result<T, ErrorCode>::value_or(U&& defValue) const& -> T
{
    return has_value() ? value() : static_cast<T>(std::forward<U>(defValue));
}

template <typename T, typename ErrorCode>
template <typename U>
inline auto result<T, ErrorCode>::value_or(U&& defValue) && -> T
{
    return has_value() ? value() : static_cast<T>(std::forward<U>(defValue));
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::has_error() const -> bool
{
    return !has_value();
}

template <typename T, typename ErrorCode>
inline auto result<T, ErrorCode>::error() const -> ErrorCode
{
    return std::get<ErrorCode>(_value);
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::and_then(F&& f) & -> result
{
    return has_value() ? std::invoke(std::forward<F>(f), value())
                       : std::remove_cvref_t<std::invoke_result_t<F, T&>> {};
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::and_then(F&& f) const& -> result
{
    return has_value() ? std::invoke(std::forward<F>(f), value())
                       : std::remove_cvref_t<std::invoke_result_t<F, T const&>> {};
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::and_then(F&& f) && -> result
{
    return has_value() ? std::invoke(std::forward<F>(f), std::move(value()))
                       : std::remove_cvref_t<std::invoke_result_t<F, T>> {};
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::and_then(F&& f) const&& -> result
{
    return has_value() ? std::invoke(std::forward<F>(f), std::move(value()))
                       : std::remove_cvref_t<std::invoke_result_t<F, T const>> {};
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::transform(F&& f) &
{
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    return has_value() ? result<U, ErrorCode> {std::invoke(std::forward<F>(f), std::move(value()))}
                       : result<U, ErrorCode> {};
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::transform(F&& f) const&
{
    using U = std::remove_cv_t<std::invoke_result_t<F, T const&>>;
    return has_value() ? result<U, ErrorCode> {std::invoke(std::forward<F>(f), std::move(value()))}
                       : result<U, ErrorCode> {};
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::transform(F&& f) &&
{
    using U = std::remove_cv_t<std::invoke_result_t<F, T>>;
    return has_value() ? result<U, ErrorCode> {std::invoke(std::forward<F>(f), std::move(value()))}
                       : result<U, ErrorCode> {};
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::transform(F&& f) const&&
{
    using U = std::remove_cv_t<std::invoke_result_t<F, T const>>;
    return has_value() ? result<U, ErrorCode> {std::invoke(std::forward<F>(f), std::move(value()))}
                       : result<U, ErrorCode> {};
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::or_else(F&& f) const& -> result
{
    return has_value() ? *this
                       : std::forward<F>(f)(error());
}

template <typename T, typename ErrorCode>
template <typename F>
inline auto result<T, ErrorCode>::or_else(F&& f) && -> result
{
    return has_value() ? std::move(*this)
                       : std::forward<F>(f)(error());
}

////////////////////////////////////////////////////////////

template <typename ErrorCode>
inline result<void, ErrorCode>::result(ErrorCode value)
{
    assert(value != ErrorCode::Ok);
    _value = value;
}

template <typename ErrorCode>
inline result<void, ErrorCode>::operator bool() const
{
    return !has_error();
}

template <typename ErrorCode>
inline auto result<void, ErrorCode>::has_error() const -> bool
{
    return _value.has_value();
}

template <typename ErrorCode>
inline auto result<void, ErrorCode>::error() const -> ErrorCode
{
    return *_value;
}

////////////////////////////////////////////////////////////

template <typename T, typename ErrorCode>
auto make_result(T&& value, ErrorCode err) -> result<T, ErrorCode>
{
    if (err == ErrorCode::Ok) { return result<T, ErrorCode> {std::forward<T>(value)}; }

    return result<T, ErrorCode> {err};
}

template <typename ErrorCode>
auto make_result(ErrorCode err) -> result<void, ErrorCode>
{
    if (err == ErrorCode::Ok) { return result<void, ErrorCode> {}; }

    return result<void, ErrorCode> {err};
}

}
