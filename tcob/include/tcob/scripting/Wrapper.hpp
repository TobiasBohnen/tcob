// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <functional>
    #include <optional>
    #include <tuple>
    #include <unordered_map>
    #include <utility>
    #include <vector>

    #include "tcob/core/Common.hpp"
    #include "tcob/core/Concepts.hpp"
    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/core/Signal.hpp"
    #include "tcob/scripting/Closure.hpp"
    #include "tcob/scripting/Conversions.hpp"
    #include "tcob/scripting/Lua.hpp"
    #include "tcob/scripting/Scripting.hpp"
    #include "tcob/scripting/Types.hpp"

namespace tcob::scripting {

////////////////////////////////////////////////////////////

enum class wrap_target : u8 {
    Getter,
    Setter,
    Method
};

template <typename T>
struct getter {
    T Method;
};

template <typename T>
struct setter {
    T Method;
};

template <typename Get, typename Set>
using property = std::pair<Get, Set>;

template <typename... Ts>
using overload = std::tuple<Ts...>;

////////////////////////////////////////////////////////////

namespace detail {
    class wrapper_base { };
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

template <typename WrappedType>
class wrapper final : public detail::wrapper_base, public non_copyable {
public:
    ////////////////////////////////////////////////////////////

    class unknown_get_event {
    public:
        unknown_get_event(WrappedType* instance, string name, state_view view);

        WrappedType* Instance {nullptr};
        string       Name;
        bool         Handled {false};

        void return_value(auto&& value);

    private:
        state_view _view {nullptr};
    };

    class unknown_set_event {
    public:
        unknown_set_event(WrappedType* instance, string name, state_view view);

        WrappedType* Instance {nullptr};
        string       Name;
        bool         Handled {false};

        template <typename X>
        auto get_value(X& val) -> bool;

    private:
        state_view _view {nullptr};
    };

    class proxy {
    public:
        proxy(wrapper& parent, string name);

        template <typename T>
        auto operator=(T const& method) -> proxy&;

        template <typename T>
        auto operator=(getter<T> const& get) -> proxy&;

        template <typename T>
        auto operator=(setter<T> const& set) -> proxy&;

        template <typename Get, typename Set>
        auto operator=(property<Get, Set> const& prop) -> proxy&;

        template <typename... Ts>
        auto operator=(overload<Ts...> const& ov) -> proxy&;

    private:
        wrapper& _parent;
        string   _name;
    };

    ////////////////////////////////////////////////////////////

    wrapper(state_view view, table* globaltable, string name);
    ~wrapper();

    signal<unknown_get_event> UnknownGet;
    signal<unknown_set_event> UnknownSet;

    auto operator[](string const& name) -> proxy;

    template <auto Func>
    void method(string_view name);
    void method(string_view name, auto&& func);

    template <typename... Funcs>
    void overload(string_view name, Funcs&&... funcs);

    template <auto Getter, auto Setter>
    void property(string_view name);
    template <auto Field>
    void property(string_view name);
    void property(string_view name, auto&& get, auto&& set);

    template <auto Getter>
    void getter(string_view name);
    void getter(string_view name, auto&& get);

    template <auto Setter>
    void setter(string_view name);
    void setter(string_view name, auto&& set);

    void metamethod(metamethod_type method, auto&& func);

    template <typename... Ts>
    void constructors(std::optional<table> targetTable = std::nullopt);

    void hide_metatable(auto&& value) const;

    template <typename S>
    void register_base();

private:
    template <typename Func>
    auto wrap_method_helper(Func&& func);
    template <typename R, typename S, typename... Args>
    auto wrap_method_helper(R (S::*func)(Args...));
    template <typename R, typename S, typename... Args>
    auto wrap_method_helper(R (S::*func)(Args...) const);
    template <typename R, typename... Args>
    auto wrap_method_helper(R (*func)(Args...));

    template <typename Func>
    auto wrap_property_helper(Func&& func);
    template <typename R, typename S>
    auto wrap_property_helper(R const (S::*prop)() const);
    template <typename R, typename S>
    auto wrap_property_helper(R (S::*prop)() const);
    template <typename R, typename S>
    auto wrap_property_helper(R const (S::*prop)());
    template <typename R, typename S>
    auto wrap_property_helper(R (S::*prop)());
    template <typename R, typename S>
    auto wrap_property_helper(void (S::*prop)(R const));
    template <typename R, typename S>
    auto wrap_property_helper_field_getter(R S::* field);
    template <typename R, typename S>
    auto wrap_property_helper_field_setter(R S::* field);
    template <typename R, typename S>
    auto wrap_property_helper_field_getter(prop<R> S::* prop);
    template <typename R, typename S>
    auto wrap_property_helper_field_setter(prop<R> S::* prop);

    template <typename... Args>
    auto wrap_constructor(arg_list<WrappedType(Args...)>) -> std::function<managed_ptr<WrappedType>(Args...)>;

    void wrap_func(string_view name, wrap_target target, native_closure_unique_ptr func);

    void set_metatable_field(string const& name, string const& tableName, auto&& value) const;

    void create_metatable(string const& name, bool gc);
    void remove_metatable(string const& name);

    template <typename R, typename... P>
    void push_metamethod(string const& methodname, std::function<R(P...)>&& func, i32 tableIdx);

    void index(WrappedType* b, i32 arg);
    void index(WrappedType* b, string const& arg);

    void newindex(WrappedType* b, i32 arg);
    void newindex(WrappedType* b, string const& arg);

    static auto gc(lua_State* l) -> i32;

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

    #include "Wrapper.inl"

#endif
