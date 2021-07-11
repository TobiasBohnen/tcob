// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <any>
#include <cassert>
#include <concepts>
#include <optional>
#include <variant>

#include <tcob/core/Helper.hpp>

struct lua_State;
typedef int (*lua_CFunction)(lua_State* L);

namespace tcob::lua {
class State;

template <typename T>
concept ConvertableToLua = requires(T& t, const State& state)
{
    { tcob::lua::Converter<T>::ToLua(state, t) };
};

template <typename T>
concept ConvertableFromLua = requires(T& t, const State& state)
{
    { tcob::lua::Converter<T>::FromLua(state, 1, t) };
};

enum class ResultState {
    Ok,
    Yielded,
    Undefined,
    TypeMismatch,
    NonTableIndex,
    RuntimeError,
    MemAllocError,
    SyntaxError
};

enum class Type {
    None,
    Nil,
    Boolean,
    LightUserdata,
    Number,
    String,
    Table,
    Function,
    Userdata,
    Thread
};

enum class ThreadState {
    Ok,
    Yielded,
    RuntimeError,
    SyntaxError,
    MemError,
    ErrorError
};

template <typename T>
struct [[nodiscard]] Result {
    T Value;
    ResultState State { ResultState::Ok };

    operator T() const
    {
        if constexpr (!tcob::detail::is_specialization<T, std::optional>())
            assert(State == ResultState::Ok);
        return Value;
    }
};

template <>
struct [[nodiscard]] Result<void> {
    ResultState State;
};

class StackGuard final {
public:
    explicit StackGuard(lua_State* l);
    ~StackGuard();

private:
    lua_State* _luaState;
    mutable i32 _oldTop { 0 };
};

class State final {
public:
    explicit State(lua_State* l);

    auto create_stack_guard() const -> StackGuard;

    template <typename... T>
    void push(T&&... t) const
    {
        check_stack(sizeof...(t));

        (to_lua(std::forward<T>(t)), ...);
    }

    template <typename T>
    auto get(i32&& idx) const -> T
    {
        T t {};
        try_get(std::forward<i32>(idx), t);
        return t;
    }

    template <typename T>
    auto try_get(i32&& idx, T& t) const -> bool
    {
        return from_lua(std::forward<i32>(idx), t);
    }

    auto is_bool(i32 idx) const -> bool;
    auto is_function(i32 idx) const -> bool;
    auto is_integer(i32 idx) const -> bool;
    auto is_number(i32 idx) const -> bool;
    auto is_string(i32 idx) const -> bool;
    auto is_table(i32 idx) const -> bool;
    auto is_thread(i32 idx) const -> bool;
    auto is_nil(i32 idx) const -> bool;
    auto is_userdata(i32 idx) const -> bool;
    auto to_bool(i32 idx) const -> bool;
    auto to_integer(i32 idx) const -> i64;
    auto to_number(i32 idx) const -> f64;
    auto to_string(i32 idx) const -> const char*;
    auto to_thread(i32 idx) const -> State;
    auto to_userdata(i32 idx) const -> void*;
    auto get_type(i32 idx) const -> Type;

    auto get_top() const -> i32;

    auto check_stack(i32 size) const -> bool;

    auto next(i32 idx) const -> bool;
    void push_bool(bool val) const;
    void push_cfunction(i32 (*lua_CFunction)(lua_State*)) const;
    void push_cclosure(i32 (*lua_CFunction)(lua_State*), i32 n) const;
    void push_integer(i64 val) const;
    void push_lightuserdata(void* p) const;
    void push_nil() const;
    void push_number(f64 val) const;
    void push_string(const char* val) const;
    void push_value(i32 idx) const;
    void pop(i32 count) const;
    void remove(i32 count) const;
    auto get_table(i32 idx) const -> Type;
    void get_metatable(const char* tableName) const;
    void set_table(i32 idx) const;
    void set_metatable(i32 idx) const;
    void create_table(i32 narr, i32 nrec) const;
    void new_table() const;
    auto new_metatable(const char* tableName) const -> i32;

    auto new_userdata(i32 size) const -> void*;
    auto new_userdata(i32 size, i32 nuvalue) const -> void*;
    auto set_uservalue(i32 index, i32 n) const -> i32;
    auto get_uservalue(i32 index, i32 n) const -> i32;

    void set_registry_field(const char* name) const;
    void insert(i32 idx) const;

    auto raw_len(i32 idx) const -> u64;
    auto raw_get(i32 idx, i64 n) const -> Type;
    auto raw_get(i32 idx) const -> Type;
    void raw_set(i32 idx, i64 n) const;
    void raw_set(i32 idx) const;

    auto ref(i32 idx) const -> i32;
    void unref(i32 t, i32 ref) const;

    auto status() const -> i32;

    auto resume(i32 argCount, i32* resultCount) const -> ThreadState;
    auto reset_thread() const -> i32;

    static auto UpvalueIndex(i32 n) -> i32;

    auto lua() const -> lua_State*;

    auto do_call(i32 nargs, i32 nret) const -> ResultState;

private:
    //////get//////
    template <ConvertableFromLua T>
    auto from_lua(i32&& idx, T& value) const -> bool
    {
        return Converter<T>::FromLua(*this, std::forward<i32>(idx), value);
    }

    //////push//////
    template <ConvertableToLua T>
    void to_lua(const T& value) const
    {
        Converter<T>::ToLua(*this, value);
    }

    template <ConvertableToLua T>
    void to_lua(T&& value) const
    {
        Converter<T>::ToLua(*this, value);
    }

    template <ConvertableToLua T>
    void to_lua(T& value) const
    {
        Converter<T>::ToLua(*this, value);
    }

    void to_lua(const char* value) const
    {
        push_string(value);
    }

    lua_State* _luaState;
};
}