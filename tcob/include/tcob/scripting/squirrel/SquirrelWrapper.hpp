// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include <functional>
    #include <unordered_map>
    #include <vector>

    #include "tcob/core/Concepts.hpp"
    #include "tcob/core/Signal.hpp"
    #include "tcob/scripting/Wrapper.hpp"
    #include "tcob/scripting/squirrel/Squirrel.hpp"
    #include "tcob/scripting/squirrel/SquirrelClosure.hpp"
    #include "tcob/scripting/squirrel/SquirrelConversions.hpp"
    #include "tcob/scripting/squirrel/SquirrelTypes.hpp"

namespace tcob::scripting::squirrel {
////////////////////////////////////////////////////////////

enum class metamethod : u8 {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    UnaryMinus,
    TypeOf,
    Compare,
    Call,
    Cloned,
    ToString
};

////////////////////////////////////////////////////////////

template <typename T>
class wrapper final : public scripting::wrapper<wrapper<T>> {
    friend class scripting::wrapper<wrapper<T>>;

public:
    class unknown_get_event {
    public:
        unknown_get_event(T* instance, string name, vm_view view);

        T*     Instance {nullptr};
        string Name;
        bool   Handled {false};

        void return_value(auto&& value);

    private:
        vm_view _view {nullptr};
    };

    class unknown_set_event {
    public:
        unknown_set_event(T* instance, string name, vm_view view);

        T*     Instance {nullptr};
        string Name;
        bool   Handled {false};

        template <typename X>
        auto get_value(X& val) -> bool;

    private:
        vm_view _view {nullptr};
    };

    wrapper(vm_view view, table* rootTable, string name);
    ~wrapper();

    signal<unknown_get_event> UnknownGet;
    signal<unknown_set_event> UnknownSet;

    void wrap_metamethod(metamethod method, auto&& func);

private:
    template <typename R, typename... P>
    auto static impl_make_unique_closure(std::function<R(P...)>&& fn)
        -> native_closure_unique_ptr;
    template <typename... Funcs>
    auto static impl_make_unique_overload(Funcs&&... fns)
        -> native_closure_unique_ptr;

    void impl_wrap_func(string_view name, wrap_target target, native_closure_unique_ptr func);

    template <typename S>
    void impl_register_base();

    void create_metatable(string const& name);
    void remove_metatable(string const& name);

    template <typename R, typename... P>
    void push_metamethod(string const& methodname, std::function<R(P...)>&& func);

    std::unordered_map<string, native_closure_unique_ptr> _functions;
    std::unordered_map<string, native_closure_unique_ptr> _getters;
    std::unordered_map<string, native_closure_unique_ptr> _setters;
    std::vector<native_closure_unique_ptr>                _metamethods;

    string  _name;
    table*  _rootTable;
    table   _metaTable;
    vm_view _view;
};

} // namespace tcob::scripting::squirrel

    #include "SquirrelWrapper.inl"

#endif
