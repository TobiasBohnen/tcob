// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SquirrelTypes.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

namespace tcob::scripting::squirrel {

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
inline auto table::get(auto&&... keys) const -> result<T>
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

    return view.get(-2) && view.pull_convert_idx(view.get_top(), value);
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
    view.push_null();
    while (view.next(-2)) {
        T var {};
        if (converter<T>::IsType(view, -2) && view.pull_convert_idx(-2, var)) {
            retValue.push_back(var);
        }

        view.pop(2);
    }

    return retValue;
}

template <typename T>
inline auto table::get(vm_view view, auto&& key, auto&&... keys) const -> result<T>
{
    push_self();
    view.push_convert(key);
    if (view.get(-2)) {
        if constexpr (sizeof...(keys) > 0) {
            if (!view.is_table(-1)) {
                return result<T> {error_code::NonTableIndex};
            }
            return table {view, -1}.get<T>(view, keys...);
        } else {
            T          retValue {};
            error_code result {view.pull_convert_idx(view.get_top(), retValue) ? error_code::Ok : error_code::TypeMismatch};
            return make_result(std::move(retValue), result);
        }
    }

    return result<T> {error_code::Undefined};
}

inline void table::set(vm_view view, auto&& key, auto&&... keysOrValue) const
{
    push_self();
    view.push_convert(key);

    if constexpr (sizeof...(keysOrValue) > 1) {
        table lt {};
        if (!view.get(-2) || !view.is_table(-1)) { // set new nested table
            view.new_table();
            lt.acquire(view, -1);
            set(view, key, lt);
        } else {
            lt.acquire(view, -1);
        }

        lt.set(view, keysOrValue...);
    } else {
        view.push_convert(keysOrValue...);
        if (view.is_table(-3)) {
            view.new_slot(-3, false);
        } // TODO: else error
    }
}

template <typename T>
inline auto table::is(vm_view view, auto&& key, auto&&... keys) const -> bool
{
    push_self();
    view.push_convert(key);
    if (view.get(-2)) {
        if constexpr (sizeof...(keys) > 0) {
            if (!view.is_table(-1)) { return false; }
            table lt {view, -1};
            return lt.is<T>(view, keys...);
        }

        return !view.is_null(-1) && converter<T>::IsType(view, view.get_top());
    }

    return false;
}

inline auto table::has(vm_view view, auto&& key, auto&&... keys) const -> bool
{
    push_self();
    view.push_convert(key);
    bool retValue {false};

    if (view.get(-2)) {
        if constexpr (sizeof...(keys) > 0) {
            if (!view.is_table(-1)) { return false; }
            table lt {};
            lt.acquire(view, -1);
            retValue = lt.has(view, keys...);
        } else {
            retValue = !view.is_null(-1);
        }
    }

    return retValue;
}

////////////////////////////////////////////////////////////

template <ConvertibleFrom T>
inline auto array::get(SQInteger index) const -> result<T>
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    view.push_convert(index);
    if (view.get(-2)) {
        T          retValue {};
        error_code result {view.pull_convert_idx(-1, retValue) ? error_code::Ok : error_code::TypeMismatch};
        return make_result(std::move(retValue), result);
    }

    return result<T> {error_code::Undefined};
}

inline void array::set(SQInteger index, auto&& value)
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    view.push_convert(index);
    view.push_convert(value);
    view.set(-3);
}

template <ConvertibleFrom T>
inline auto array::is(SQInteger index) const -> bool
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    view.push_convert(index);
    if (view.get(-2)) {
        return !view.is_null(-1) && converter<T>::IsType(view, -1);
    }

    return false;
}

template <ConvertibleTo T>
inline void array::add(T const& addValue)
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    view.push_convert(addValue);
    view.array_append(-2);
}

////////////////////////////////////////////////////////////////////////////////
namespace detail {

    template <ConvertibleFrom T>
    inline auto type_ref::get(auto&& key) const -> result<T>
    {
        auto const view {get_view()};
        auto const guard {view.create_stack_guard()};

        push_self();
        view.push_convert(key);
        if (view.get(-2)) {
            T          retValue {};
            error_code result {view.pull_convert_idx(view.get_top(), retValue) ? error_code::Ok : error_code::TypeMismatch};
            return make_result(std::move(retValue), result);
        }

        return result<T> {error_code::Undefined};
    }

    template <ConvertibleFrom T>
    inline auto type_ref::try_get(T& value, auto&& key) const -> bool
    {
        auto const view {get_view()};
        auto const guard {view.create_stack_guard()};

        push_self();
        view.push_convert(key);

        return view.get(-2) && view.pull_convert_idx(view.get_top(), value);
    }

    template <typename T>
    inline auto type_ref::is(auto&& key) const -> bool
    {
        auto const view {get_view()};
        auto const guard {view.create_stack_guard()};

        push_self();
        view.push_convert(key);
        return view.get(-2)
            && !view.is_null(-1)
            && converter<T>::IsType(view, view.get_top());
    }

    inline auto type_ref::has(auto&& key) const -> bool
    {
        auto const view {get_view()};
        auto const guard {view.create_stack_guard()};

        push_self();
        view.push_convert(key);
        return view.get(-2) && !view.is_null(-1);
    }
}

////////////////////////////////////////////////////////////////////////////////

template <typename Key>
inline auto instance::operator[](Key key) -> proxy<instance, Key>
{
    return proxy<instance, Key> {*this, std::tuple {key}};
}

template <typename Key>
inline auto instance::operator[](Key key) const -> proxy<instance const, Key>
{
    return proxy<instance const, Key> {*this, std::tuple {key}};
}

inline void instance::set(auto&& key, auto&& value) const
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    view.push_convert(key);
    view.push_convert(value);
    view.set(-3);
}

////////////////////////////////////////////////////////////////////////////////

template <typename Key>
inline auto clazz::operator[](Key key) -> proxy<clazz, Key>
{
    return proxy<clazz, Key> {*this, std::tuple {key}};
}

template <typename Key>
inline auto clazz::operator[](Key key) const -> proxy<clazz const, Key>
{
    return proxy<clazz const, Key> {*this, std::tuple {key}};
}

inline void clazz::set(auto&& key, auto&& value) const
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    view.push_convert(key);
    view.push_convert(value);
    view.new_slot(-3, false);
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

    // push parameters to squirrel
    SQInteger const oldTop {view.get_top()};
    view.push_roottable(); // sq_getclosureroot?
    view.push_convert(params...);
    SQInteger const paramsCount {view.get_top() - oldTop};

    // call squirrel function
    auto result {call_protected(paramsCount, !std::is_void_v<R>)};
    if constexpr (std::is_void_v<R>) {
        return make_result(result);
    } else {
        R retValue {};
        if (result == error_code::Ok) {
            if (!view.pull_convert_idx(view.get_top(), retValue)) {
                result = error_code::TypeMismatch;
            }
        }

        return make_result(std::move(retValue), result);
    }
}

template <typename R>
inline auto function<R>::Acquire(vm_view view, SQInteger idx) -> function<return_type>
{
    return function {view, idx};
}

template <typename R>
inline auto function<R>::IsType(vm_view view, SQInteger idx) -> bool
{
    return view.is_closure(idx) || view.is_nativeclosure(idx);
}

////////////////////////////////////////////////////////////////////////////////

template <typename R>
inline auto generator::resume() const -> result<R>
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();

    if (view.resume(!std::is_void_v<R>)) {
        if constexpr (std::is_void_v<R>) {
            return {};
        } else {
            R retValue {};
            return view.pull_convert_idx(view.get_top(), retValue)
                ? result<R> {std::move(retValue)}
                : result<R> {error_code::TypeMismatch};
        }
    }

    return result<R> {error_code::Error};
}

////////////////////////////////////////////////////////////////////////////////

template <typename R>
inline auto thread::call(auto&&... params) const -> result<R>
{
    auto const thread {get_thread()};
    // auto const  guard{thread.create_stack_guard()}; don't clean up stack

    // push parameters to squirrel
    SQInteger const oldTop {thread.get_top()};
    thread.push_roottable(); // sq_getclosureroot?
    thread.push_convert(params...);
    SQInteger const paramsCount {thread.get_top() - oldTop};

    // call squirrel function
    auto result {thread.call(paramsCount, !std::is_void_v<R>, true)};
    if constexpr (std::is_void_v<R>) {
        return make_result(result);
    } else {
        R retValue {};
        if (result == error_code::Ok) {
            if (!thread.pull_convert_idx(thread.get_top(), retValue)) {
                result = error_code::TypeMismatch;
            }
        }

        return make_result(std::move(retValue), result);
    }
}

template <typename R>
inline auto thread::wake_up() const -> result<R>
{
    return wake_up<R>(nullptr);
}

template <typename R>
inline auto thread::wake_up(auto&& arg) const -> result<R>
{
    auto const thread {get_thread()};
    auto const guard {thread.create_stack_guard()};

    thread.push_convert(arg);
    if (thread.wakeup_vm(true, !std::is_void_v<R>)) {
        if constexpr (std::is_void_v<R>) {
            return {};
        } else {
            R retValue {};
            return thread.pull_convert_idx(thread.get_top(), retValue)
                ? result<R> {std::move(retValue)}
                : result<R> {error_code::TypeMismatch};
        }
    }

    return result<R> {error_code::Error};
}

}

#endif
