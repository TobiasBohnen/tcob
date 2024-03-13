// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "LuaTypes.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include "tcob/scripting/lua/LuaConversions.hpp" // IWYU pragma: keep

namespace tcob::scripting::lua {

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

template <ConvertibleFrom T>
inline auto table::get(auto&&... keys) const -> result<T>
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};
    return get<T>(view, keys...);
}

template <ConvertibleFrom T>
inline auto table::try_get(T& value, auto&& key) const -> bool
{
    auto const res {get<T>(key)};
    if (res.has_value()) {
        value = res.value();
        return true;
    }

    return false;
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
inline auto table::get(state_view view, auto&& key, auto&&... keys) const -> result<T>
{
    push_self();
    view.push_convert(key);
    view.get_table(-2);

    if constexpr (sizeof...(keys) > 0) {
        if (!view.is_table(-1)) {
            return result<T> {error_code::NonTableIndex};
        }
        return table {view, -1}.get<T>(view, keys...);
    } else {
        T          retValue {};
        error_code result {};

        if (view.is_nil(-1)) {
            result = error_code::Undefined;
        } else {
            result = view.pull_convert_idx(-1, retValue) ? error_code::Ok : error_code::TypeMismatch;
        }

        return make_result(std::move(retValue), result);
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
        view.set_table(-3);
    }
}

template <typename T>
inline auto table::is(state_view view, auto&& key, auto&&... keys) const -> bool
{
    push_self();
    view.push_convert(key);
    view.get_table(-2);

    if constexpr (sizeof...(keys) > 0) {
        if (!view.is_table(-1)) {
            return false;
        }
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
        static_cast<void>(call(params...));
    } else {
        return call(params...).value();
    }
}

template <typename R>
inline auto function<R>::call(auto&&... params) const -> result<return_type>
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();

    // push parameters to lua
    i32 const oldTop {view.get_top()};
    view.push_convert(params...);
    i32 const paramsCount {view.get_top() - oldTop};

    // call lua function
    auto result {call_protected(paramsCount)};
    if constexpr (std::is_void_v<R>) {
        return make_result(result);
    } else {
        R retValue {};
        if (result == error_code::Ok) {
            auto const top {view.get_top() - get_stacksize<R>() + 1};
            if (!view.pull_convert_idx(top, retValue)) {
                result = error_code::TypeMismatch;
            }
        }

        return make_result(std::move(retValue), result);
    }
}

template <typename R>
inline auto function<R>::call_async(auto&&... params) const -> std::future<result<return_type>>
{
    return std::async(std::launch::async, [&, params...] {
        return call<R>(params...);
    });
}

template <typename R>
inline auto function<R>::Acquire(state_view view, i32 idx) -> function<R>
{
    return function {view, idx};
}

////////////////////////////////////////////////////////////////////////////////

template <typename R>
inline auto coroutine::resume(auto&&... params) -> result<R>
{
    state_view const thread {get_thread()};
    auto const       guard {thread.create_stack_guard()};

    // push parameters to lua
    i32 const oldTop {thread.get_top()};
    thread.push_convert(params...);
    i32 const paramsCount {thread.get_top() - oldTop};

    // call lua function
    i32 resultCount {0};
    _status = thread.resume(paramsCount, &resultCount);
    if (_status == coroutine_status::Suspended || _status == coroutine_status::Dead) {
        if constexpr (std::is_void_v<R>) {
            return {};
        } else {
            R retValue {};
            return thread.pull_convert_idx(1, retValue)
                ? result<R> {std::move(retValue)}
                : result<R> {error_code::TypeMismatch};
        }
    } else {
        return result<R> {error_code::Error};
    }
}

inline void coroutine::push(auto&&... values) const
{
    get_thread().push_convert(values...);
}

}

#endif
