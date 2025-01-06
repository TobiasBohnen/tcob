// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <concepts>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/scripting/Scripting.hpp"

////////////////////////////////////////////////////////////

struct lua_State;
struct lua_Debug;
using lua_CFunction    = int (*)(lua_State*);
using lua_WarnFunction = void (*)(void*, char const*, int);
using lua_Hook         = void (*)(lua_State*, lua_Debug*);
using lua_Writer       = int (*)(lua_State*, void const* p, size_t sz, void* ud);
using lua_Alloc        = void* (*)(void* ud, void* ptr, size_t osize, size_t nsize);

////////////////////////////////////////////////////////////

namespace tcob::scripting::lua {

////////////////////////////////////////////////////////////

constexpr i32 NOREF = -2;             //  LUA_NOREF
    #if defined(TCOB_USE_LUAJIT)
constexpr i32 REGISTRYINDEX = -10000; //  LUA_REGISTRYINDEX
    #else
constexpr i32 REGISTRYINDEX = -1001000; //  LUA_REGISTRYINDEX
    #endif

////////////////////////////////////////////////////////////

template <typename T>
struct converter;

class state_view;

template <typename T>
concept ConvertibleTo =
    requires(T& t, state_view view) {
        { converter<T>::To(view, t) };
    } || requires(T& t, state_view view) {
        { converter<std::remove_cvref_t<T>>::To(view, t) };
    };

template <typename T>
concept ConvertibleFrom =
    requires(T& t, i32& i, state_view view) {
        {
            converter<T>::From(view, i, t)
        };
    } || requires(T& t, i32& i, state_view view) {
        {
            converter<std::remove_cvref_t<T>>::From(view, i, t)
        };
    };

////////////////////////////////////////////////////////////

enum class library : u8 {
    Base,
    Table,
    String,
    Math,
    IO,
    OS,
    Debug,
    Package,
    #if defined(TCOB_USE_LUAJIT)
    JIT,
    #else
    Coroutine,
    Utf8,
    #endif

};

////////////////////////////////////////////////////////////

enum class type : u8 {
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

////////////////////////////////////////////////////////////

enum class coroutine_status : u8 {
    Ok,
    Suspended,
    Dead,
    RuntimeError,
    SyntaxError,
    MemError,
    Error
};

////////////////////////////////////////////////////////////

enum class debug_event : u32 { // NOLINT(performance-enum-size)
    Call     = 0,
    Return   = 1,
    Line     = 2,
    Count    = 3,
    TailCall = 4
};

struct debug_mask {
    bool Call {true};
    bool Return {true};
    bool Line {true};
    bool Count {true};
};

////////////////////////////////////////////////////////////

class TCOB_API debug {
public:
    debug(state_view* view, lua_Debug* ar);

    debug_event Event {};
    string      Name;                      /* (n) */
    string      What;                      /* (S) */
    string      Source;                    /* (S) */
    i32         CurrentLine {0};           /* (l) */
    i32         LineDefined {0};           /* (S) */
    i32         LastLineDefined {0};       /* (S) */
    string      NameWhat;                  /* (n) */
    ubyte       UpvalueCount {0};          /* (u) number of upvalues */
    ubyte       ParameterCount {0};        /* (u) number of parameters */
    bool        IsVarArg {false};          /* (u) */
    bool        IsTailCall {false};        /* (t) */
    u16         FirstTransfer {0};         /* (r) index of first value transferred */
    u16         TransferredValueCount {0}; /* (r) number of transferred values */
    string      ShortSource;               /* (S) */

    auto get_local(i32 n) const -> string;
    auto set_local(i32 n) const -> string;

    auto static GetMask(debug_mask mask) -> i32;

private:
    state_view* _view;
    lua_Debug*  _ar;
};

////////////////////////////////////////////////////////////

class TCOB_API stack_guard final : public non_copyable {
public:
    explicit stack_guard(lua_State* l);
    ~stack_guard();

    auto get_top() const -> i32;

private:
    lua_State* _luaState;
    i32        _oldTop;
};

////////////////////////////////////////////////////////////

class TCOB_API state_view final {
public:
    explicit state_view(lua_State* l);

    auto create_stack_guard [[nodiscard]] () const -> stack_guard;

    template <typename... T>
    void push_convert(T&&... t) const;

    template <ConvertibleFrom T>
    auto pull_convert(i32& idx, T& t) const -> bool;

    template <ConvertibleFrom T>
    auto pull_convert_idx(i32 idx, T& t) const -> bool;

    auto is_bool(i32 idx) const -> bool;
    auto is_function(i32 idx) const -> bool;
    auto is_integer(i32 idx) const -> bool;
    auto is_number(i32 idx) const -> bool;
    auto is_string(i32 idx) const -> bool;
    auto is_table(i32 idx) const -> bool;
    auto is_thread(i32 idx) const -> bool;
    auto is_nil(i32 idx) const -> bool;
    auto is_none(i32 idx) const -> bool;
    auto is_none_or_nil(i32 idx) const -> bool;
    auto is_userdata(i32 idx) const -> bool;

    auto to_bool(i32 idx) const -> bool;
    auto to_integer(i32 idx) const -> i64;
    auto to_number(i32 idx) const -> f64;
    auto to_string(i32 idx) const -> char const*;
    auto to_thread(i32 idx) const -> state_view;
    auto to_userdata(i32 idx) const -> void*;

    auto get_type(i32 idx) const -> type;

    auto get_top() const -> i32;

    auto info(string const& what, lua_Debug* ar) const -> bool;
    auto get_local(lua_Debug* ar, i32 n) const -> string;
    auto set_local(lua_Debug* ar, i32 n) const -> string;

    auto check_stack(i32 size) const -> bool;

    auto next(i32 idx) const -> bool;

    void push_bool(bool val) const;
    void push_cfunction(i32 (*lua_CFunction)(lua_State*)) const;
    void push_cclosure(i32 (*lua_CFunction)(lua_State*), i32 n) const;
    void push_integer(i64 val) const;
    void push_lightuserdata(void* p) const;
    void push_nil() const;
    void push_number(f64 val) const;
    void push_string(string const& val) const;
    void push_string(char const* val) const;
    void push_lstring(string_view val) const;
    void push_value(i32 idx) const;
    void push_globaltable() const;

    void pop(i32 count) const;
    void remove(i32 idx) const;

    auto get_table(i32 idx) const -> type;
    void set_table(i32 idx) const;
    void create_table(i32 narr, i32 nrec) const;
    void new_table() const;

    auto get_metatable(i32 objindex) const -> bool;
    void get_metatable(string const& tableName) const;
    void get_metatable(char const* tableName) const;
    void set_metatable(i32 idx) const;
    auto new_metatable(string const& tableName) const -> i32;
    auto new_metatable(char const* tableName) const -> i32;

    auto new_userdata(usize size) const -> void*;

    auto set_uservalue(i32 idx) const -> i32;
    auto get_uservalue(i32 idx) const -> type;

    void get_field(i32 idx, string const& name) const;
    void set_field(i32 idx, string const& name) const;
    void set_registry_field(string const& name) const;
    void insert(i32 idx) const;

    auto raw_len(i32 idx) const -> u64;
    auto raw_get(i32 idx, i64 n) const -> type;
    auto raw_get(i32 idx) const -> type;
    void raw_set(i32 idx, i64 n) const;
    void raw_set(i32 idx) const;
    auto raw_equal(i32 idx1, i32 idx2) const -> bool;

    auto ref(i32 idx) const -> i32;
    void unref(i32 t, i32 ref) const;

    auto status() const -> i32;

    auto is_yieldable() const -> bool;
    auto resume(i32 argCount) const -> coroutine_status;
    auto close_thread() const -> bool;

    void error(string const& message) const;

    auto call(i32 nargs) const -> error_code;
    auto pcall(i32 nargs) const -> error_code;

    auto traceback(i32 level) const -> string;

    void requiref(string const& modname, lua_CFunction openf, bool glb) const;
    void require_library(library lib) const;

    auto load_buffer(string_view script, string const& name) const -> bool;
    auto load_buffer(string_view script, string const& name, string const& mode) const -> bool;

    void set_warnf(lua_WarnFunction f, void* ud) const;

    void set_hook(lua_Hook func, i32 mask, i32 count) const;
    void get_info(lua_Debug* ar) const;

    auto gc(i32 what, i32 a, i32 b, i32 c) const -> i32;

    auto dump(lua_Writer writer, void* data, i32 strip) const -> i32;

    auto get_upvalue(i32 funcindex, i32 n) const -> char const*;
    auto set_upvalue(i32 funcindex, i32 n) const -> char const*;

    auto static GetUpvalueIndex(i32 n) -> i32;

    auto static NewState() -> lua_State*;

    void close();
    auto is_valid() const -> bool;

private:
    template <ConvertibleFrom T>
    auto convert_from(i32& idx, T& value) const -> bool;

    template <ConvertibleTo T>
    void convert_to(T const& value) const;

    template <ConvertibleTo T>
    void convert_to(T&& value) const;

    template <ConvertibleTo T>
    void convert_to(T& value) const;

    lua_State* _state;
};

////////////////////////////////////////////////////////////

class TCOB_API garbage_collector final {
public:
    explicit garbage_collector(state_view l);

    auto count() const -> i32;
    auto is_running() const -> bool;

    void start_incremental_mode(i32 pause, i32 stepmul, i32 stepsize) const;
    void start_generational_mode(i32 minormul, i32 majormul) const;

    void collect() const;

    void stop() const;
    void restart() const;

private:
    state_view _luaState;
};

////////////////////////////////////////////////////////////
}

    #include "Lua.inl"

#endif
