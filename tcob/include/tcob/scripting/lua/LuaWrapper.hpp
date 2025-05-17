// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <functional>
    #include <optional>
    #include <unordered_map>
    #include <vector>

    #include "tcob/core/Common.hpp"
    #include "tcob/core/Concepts.hpp"
    #include "tcob/core/Signal.hpp"
    #include "tcob/scripting/Scripting.hpp"
    #include "tcob/scripting/Wrapper.hpp"
    #include "tcob/scripting/lua/Lua.hpp"
    #include "tcob/scripting/lua/LuaClosure.hpp"
    #include "tcob/scripting/lua/LuaConversions.hpp"
    #include "tcob/scripting/lua/LuaTypes.hpp"

namespace tcob::scripting::lua {
////////////////////////////////////////////////////////////

enum class metamethod : u8 {
    Length,
    ToString,
    UnaryMinus,
    Add,
    Subtract,
    Divide,
    Multiply,
    Concat,
    LessThan,
    LessOrEqualThan,
    Call,
    FloorDivide,
    Modulo,
    PowerOf,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    BitwiseNot,
    LeftShift,
    RightShift,
    Close
};

////////////////////////////////////////////////////////////

namespace detail {
    ////////////////////////////////////////////////////////////

    template <typename T>
    concept IntIndexable =
        requires(T& t, i32&& u) {
            typename T::value_type;
            typename T::size_type;

            { t[u] } -> ConvertibleTo;
        };

    template <typename T>
    concept StringIndexable =
        requires(T& t, string&& u) {
            { t[u] } -> ConvertibleTo;
        };
}

////////////////////////////////////////////////////////////

template <typename T>
class wrapper final : public scripting::wrapper<wrapper<T>> {
    friend class scripting::wrapper<wrapper<T>>;

public:
    class unknown_get_event {
    public:
        unknown_get_event(T* instance, string name, state_view view);

        T*     Instance {nullptr};
        string Name;
        bool   Handled {false};

        void return_value(auto&& value) const;

    private:
        state_view _view {nullptr};
    };

    class unknown_set_event {
    public:
        unknown_set_event(T* instance, string name, state_view view);

        T*     Instance {nullptr};
        string Name;
        bool   Handled {false};

        template <typename X>
        auto get_value(X& val) const -> bool;

    private:
        state_view _view {nullptr};
    };

    wrapper(state_view view, table* globaltable, string name);
    ~wrapper();

    signal<unknown_get_event> UnknownGet;
    signal<unknown_set_event> UnknownSet;

    void wrap_metamethod(metamethod method, auto&& func);

    template <typename... Ts>
    void wrap_constructors(std::optional<table> targetTable = std::nullopt);

    void hide_metatable(auto&& value) const;

private:
    template <typename... Args>
    auto process_constructor(arg_list<T(Args...)>) -> std::function<managed_ptr<T>(Args...)>;

    template <typename R, typename... P>
    auto static impl_make_unique_closure(std::function<R(P...)>&& fn) -> native_closure_unique_ptr;
    template <typename... Funcs>
    auto static impl_make_unique_overload(Funcs&&... fns) -> native_closure_unique_ptr;

    void impl_wrap_func(string_view name, wrap_target target, native_closure_unique_ptr func);

    template <typename S>
    void impl_register_base();

    void set_metatable_field(string const& name, string const& tableName, auto&& value) const;

    void create_metatable(string const& name, bool gc);
    void remove_metatable(string const& name);

    template <typename R, typename... P>
    void push_metamethod(string const& methodname, std::function<R(P...)>&& func, i32 tableIdx);

    void index(T* b, i32 arg);
    void index(T* b, string const& arg);

    void newindex(T* b, i32 arg);
    void newindex(T* b, string const& arg);

    auto static gc(lua_State* l) -> i32;

    std::unordered_map<string, native_closure_unique_ptr> _functions;
    std::unordered_map<string, native_closure_unique_ptr> _getters;
    std::unordered_map<string, native_closure_unique_ptr> _setters;
    native_closure_unique_ptr                             _constructor;
    std::vector<native_closure_unique_ptr>                _metamethods;

    string     _name;
    table*     _globalTable;
    state_view _view;
};

}

    #include "LuaWrapper.inl"

#endif
