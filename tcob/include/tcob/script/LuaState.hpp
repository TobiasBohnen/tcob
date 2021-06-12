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

namespace tcob {
template <typename T>
concept ConvertableToLua = requires(T& t, const LuaState& state)
{
    { tcob::LuaConverter<T>::ToLua(state, t) };
};

template <typename T>
concept ConvertableFromLua = requires(T& t, const LuaState& state)
{
    { tcob::LuaConverter<T>::FromLua(state, 1, t) };
};

enum class LuaResultState {
    Ok,
    Yielded,
    Undefined,
    TypeMismatch,
    NonTableIndex,
    RuntimeError,
    MemAllocError,
    SyntaxError
};

enum class LuaType {
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

enum class LuaThreadState {
    Ok,
    Yielded,
    RuntimeError,
    SyntaxError,
    MemError,
    ErrorError
};

template <typename T>
struct [[nodiscard]] LuaResult {
    T Value;
    LuaResultState State { LuaResultState::Ok };

    operator T() const
    {
        if constexpr (!tcob::detail::is_specialization<T, std::optional>())
            assert(State == LuaResultState::Ok);
        return Value;
    }
};

template <>
struct [[nodiscard]] LuaResult<void> {
    LuaResultState State;
};

class LuaStackGuard final {
public:
    explicit LuaStackGuard(lua_State* l);
    ~LuaStackGuard();

private:
    lua_State* _luaState;
    mutable i32 _oldTop { 0 };
};

class LuaState final {
public:
    explicit LuaState(lua_State* l);

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
    auto to_userdata(i32 idx) const -> void*;
    auto get_type(i32 idx) const -> LuaType;

    auto get_top() const -> i32;

    auto create_stack_guard() const -> LuaStackGuard;

    void check_stack(i32 size) const;
    auto resume(i32 argCount, i32* resultCount) const -> LuaThreadState;
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
    void get_table(i32 idx) const;
    void get_metatable(const char* tableName) const;
    void set_table(i32 idx) const;
    void set_metatable(i32 idx) const;
    void create_table(i32 narr, i32 nrec) const;
    void new_table() const;
    auto new_metatable(const char* tableName) const -> i32;
    auto new_userdata(i32 size) const -> void*;
    void set_registry_field(const char* name) const;
    void insert(i32 idx) const;

    auto rawlen(i32 idx) const -> u64;
    void rawgeti(i32 idx, i64 n) const;
    void rawget(i32 idx) const;
    void rawseti(i32 idx, i64 n) const;
    void rawset(i32 idx) const;

    static auto UpvalueIndex(i32 n) -> i32;

    auto lua() const -> lua_State*;

private:
    //////get//////
    template <ConvertableFromLua T>
    auto from_lua(i32&& idx, T& value) const -> bool
    {
        return LuaConverter<T>::FromLua(*this, std::forward<i32>(idx), value);
    }

    //////push//////
    template <ConvertableToLua T>
    void to_lua(const T& value) const
    {
        LuaConverter<T>::ToLua(*this, value);
    }

    template <ConvertableToLua T>
    void to_lua(T&& value) const
    {
        LuaConverter<T>::ToLua(*this, value);
    }

    template <ConvertableToLua T>
    void to_lua(T& value) const
    {
        LuaConverter<T>::ToLua(*this, value);
    }

    void to_lua(const char* value) const
    {
        push_string(value);
    }

    lua_State* _luaState;
};
}