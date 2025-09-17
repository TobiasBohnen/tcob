// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Types.hpp"

#include <expected>
#include <tuple>
#include <vector>

#include "tcob/core/Proxy.hpp"
#include "tcob/scripting/Lua.hpp"
#include "tcob/scripting/Scripting.hpp"

namespace tcob::scripting {

template <typename Key>
inline auto table::operator[](Key key) -> proxy<table, Key>
{
    return proxy<table, Key> {*this, std::tuple {key}};
}

template <typename Key>
inline auto table::operator[](Key key) const -> proxy<table const, Key>
{
    return proxy<table const, Key> {*this, std::tuple {key}};
}

template <typename... Args, typename T>
inline auto table::try_make(T& value, auto&&... keys) const -> bool
{
    constexpr usize argsCount {sizeof...(Args)};
    constexpr usize keyCount {sizeof...(keys)};
    static_assert(argsCount == keyCount);

    std::tuple<Args...> tup {};
    if (std::apply(
            [&](auto&&... item) {
                return (try_get(item, keys) && ...);
            },
            tup)) {
        value = std::make_from_tuple<T>(tup);
        return true;
    }
    return false;
}

template <ConvertibleFrom T>
inline auto table::get(auto&&... keys) const -> std::expected<T, error_code>
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};
    return get<T>(view, keys...);
}

template <ConvertibleFrom T>
inline auto table::try_get(T& value, auto&& key) const -> bool
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    view.push_convert(key);
    view.get_table(-2);

    return !view.is_nil(-1) && view.pull_convert_idx(-1, value);
}

inline void table::set(auto&&... keysOrValue) const
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};
    set(view, keysOrValue...);
}

template <typename T>
inline auto table::is(auto&&... keys) const -> bool
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};
    return is<T>(view, keys...);
}

inline auto table::has(auto&&... keys) const -> bool
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};
    return has(view, keys...);
}

template <typename T>
inline auto table::get_keys() const -> std::vector<T>
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    std::vector<T> retValue {};
    push_self();
    view.push_nil();
    while (view.next(-2)) {
        view.push_value(-2);

        T var {};
        if (converter<T>::IsType(view, -1) && view.pull_convert_idx(-1, var)) {
            retValue.push_back(var);
        }

        view.pop(2);
    }

    return retValue;
}

template <typename T>
inline auto table::get(state_view view, auto&& key, auto&&... keys) const -> std::expected<T, error_code>
{
    push_self();
    view.push_convert(key);
    view.get_table(-2);

    if constexpr (sizeof...(keys) > 0) {
        if (!view.is_table(-1)) {
            return std::unexpected {error_code::NonTableIndex};
        }
        return table {view, -1}.get<T>(view, keys...);
    } else {
        if (view.is_nil(-1)) { return std::unexpected {error_code::Undefined}; }

        T retValue {};
        return view.pull_convert_idx(-1, retValue)
            ? std::expected<T, error_code> {std::move(retValue)}
            : std::unexpected {error_code::TypeMismatch};
    }
}

inline void table::set(state_view view, auto&& key, auto&&... keysOrValue) const
{
    push_self();
    view.push_convert(key);

    if constexpr (sizeof...(keysOrValue) > 1) {
        view.get_table(-2);
        table lt {};
        if (!view.is_table(-1)) { // set new nested table
            view.new_table();
            lt.acquire(view, -1);
            set(view, key, lt);
        } else {
            lt.acquire(view, -1);
        }

        lt.set(view, keysOrValue...);
    } else {
        view.push_convert(keysOrValue...);

        if (view.get_top() >= 3 && view.is_table(-3)) {
            view.set_table(-3);
        } // TODO: else error
    }
}

template <typename T>
inline auto table::is(state_view view, auto&& key, auto&&... keys) const -> bool
{
    push_self();
    view.push_convert(key);
    view.get_table(-2);

    if constexpr (sizeof...(keys) > 0) {
        if (!view.is_table(-1)) { return false; }
        table lt {view, -1};
        return lt.is<T>(view, keys...);
    } else {
        return !view.is_nil(-1) && converter<T>::IsType(view, view.get_top());
    }
}

inline auto table::has(state_view view, auto&& key, auto&&... keys) const -> bool
{
    push_self();
    view.push_convert(key);
    view.get_table(-2);

    if constexpr (sizeof...(keys) > 0) {
        if (!view.is_table(-1)) {
            return false;
        }
        table const lt {view, -1};
        return lt.has(view, keys...);
    } else {
        return !view.is_nil(-1);
    }
}

////////////////////////////////////////////////////////////////////////////////

template <typename R>
inline auto function<R>::operator()(auto&&... params) const -> return_type
{
    if constexpr (std::is_void_v<R>) {
        static_cast<void>(protected_call(params...));
    } else {
        return protected_call(params...).value();
    }
}

template <typename R>
inline auto function<R>::protected_call(auto&&... params) const -> std::expected<R, error_code>
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();

    // push parameters to lua
    i32 const oldTop {view.get_top()};
    view.push_convert(params...);
    i32 const paramsCount {view.get_top() - oldTop};

    // call lua function
    auto const result {pcall(paramsCount)};
    if constexpr (std::is_void_v<R>) {
        return !result ? std::expected<void, error_code> {} : std::unexpected {*result};
    } else {
        if (result) { return std::unexpected {*result}; }

        R retValue {};
        if (!view.pull_convert_idx(oldTop, retValue)) {
            return std::unexpected {error_code::TypeMismatch};
        }

        return std::expected<R, error_code> {std::move(retValue)};
    }
}

template <typename R>
inline auto function<R>::unprotected_call(auto&&... params) const -> std::expected<R, error_code>
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();

    // push parameters to lua
    i32 const oldTop {view.get_top()};
    view.push_convert(params...);
    i32 const paramsCount {view.get_top() - oldTop};

    // call lua function
    call(paramsCount);
    if constexpr (std::is_void_v<R>) {
        return {};
    } else {
        R retValue {};
        if (!view.pull_convert_idx(oldTop, retValue)) {
            return std::unexpected {error_code::TypeMismatch};
        }

        return retValue;
    }
}

template <typename R>
inline auto function<R>::Acquire(state_view view, i32 idx) -> function<R>
{
    return function {view, idx};
}

////////////////////////////////////////////////////////////////////////////////

template <typename R>
inline auto coroutine::resume(auto&&... params) -> std::expected<R, error_code>
{
    if (_status == coroutine_status::Dead) {
        return std::unexpected {error_code::Error};
    }

    state_view const thread {get_thread()};
    auto const       guard {thread.create_stack_guard()};

    // push parameters to lua
    i32 const oldTop {thread.get_top()};
    thread.push_convert(params...);
    i32 const paramsCount {thread.get_top() - oldTop};

    // call lua function
    _status = thread.resume(paramsCount);
    if (_status == coroutine_status::Suspended || _status == coroutine_status::Dead) {
        if constexpr (std::is_void_v<R>) {
            return {};
        } else {
            R retValue {};
            return thread.pull_convert_idx(1, retValue)
                ? std::expected<R, error_code> {std::move(retValue)}
                : std::unexpected {error_code::TypeMismatch};
        }
    } else {
        return std::unexpected {error_code::Error};
    }
}

inline void coroutine::push(auto&&... values) const
{
    get_thread().push_convert(values...);
}

}
