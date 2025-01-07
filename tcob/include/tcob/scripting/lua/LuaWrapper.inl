// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "LuaWrapper.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

namespace tcob::scripting::lua {

namespace detail {

    ////////////////////////////////////////////////////////////
    [[maybe_unused]] auto static get_metamethod_name(metamethod m) -> std::string
    {
        switch (m) {
        case metamethod::Length: return "__len";
        case metamethod::ToString: return "__tostring";
        case metamethod::UnaryMinus: return "__unm";
        case metamethod::Add: return "__add";
        case metamethod::Subtract: return "__sub";
        case metamethod::Divide: return "__div";
        case metamethod::Multiply: return "__mul";
        case metamethod::Concat: return "__concat";
        case metamethod::LessThan: return "__lt";
        case metamethod::LessOrEqualThan: return "__le";
        case metamethod::Call: return "__call";
        case metamethod::FloorDivide: return "__idiv";
        case metamethod::Modulo: return "__mod";
        case metamethod::PowerOf: return "__pow";
        case metamethod::BitwiseAnd: return "__band";
        case metamethod::BitwiseOr: return "__bor";
        case metamethod::BitwiseXor: return "__bxor";
        case metamethod::BitwiseNot: return "__bnot";
        case metamethod::LeftShift: return "__shl";
        case metamethod::RightShift: return "__shr";
        case metamethod::Close: return "__close";
        }

        return "";
    }
}

////////////////////////////////////////////////////////////

template <typename T>
template <typename R, typename... P>
inline auto wrapper<T>::impl_make_unique_closure(std::function<R(P...)>&& fn) -> native_closure_unique_ptr
{
    return make_unique_closure(std::move(fn));
}

template <typename T>
template <typename... Funcs>
inline auto wrapper<T>::impl_make_unique_overload(Funcs&&... fns) -> native_closure_unique_ptr
{
    return make_unique_overload(std::forward<Funcs>(fns)...);
}

template <typename T>
inline void wrapper<T>::impl_wrap_func(string const& name, wrap_target target, native_closure_unique_ptr func)
{
    switch (target) {
    case wrap_target::Getter:
        _getters[name] = std::move(func);
        break;
    case wrap_target::Setter:
        _setters[name] = std::move(func);
        break;
    case wrap_target::Method:
        _functions[name] = std::move(func);
        break;
    }
}

template <typename T>
template <typename S>
inline void wrapper<T>::impl_register_base()
{
    static_assert(std::is_base_of_v<S, T>);
    _view.get_metatable(typeid(T).name());
    table tab {table::Acquire(_view, -1)};

    std::unordered_set<string> types;
    tab.try_get(types, "__types");
    if (!types.contains(typeid(S).name())) {
        types.insert(typeid(S).name());
        tab["__types"] = types;
    }

    _view.pop(1);
}

////////////////////////////////////////////////////////////

template <typename T>
inline void wrapper<T>::wrap_metamethod(metamethod method, auto&& func)
{
    string const& name {detail::get_metamethod_name(method)};
    auto          ptr {scripting::wrapper<wrapper<T>>::make_unique_closure(std::function {func})};
    set_metatable_field(name, typeid(T).name(), ptr.get());
    set_metatable_field(name, (string(typeid(T).name()) + "_gc"), ptr.get());
    _metamethods.push_back(std::move(ptr));
}

template <typename T>
template <typename... Ts>
inline void wrapper<T>::wrap_constructors(std::optional<table> targetTable)
{
    auto& dstTable {targetTable.has_value() ? targetTable.value() : *_globalTable};

    // create constructor table
    if (!dstTable.has(_name)) {
        dstTable[_name] = table {};
    }

    if constexpr (sizeof...(Ts) > 1) {
        _constructor = scripting::wrapper<wrapper<T>>::make_unique_overload(process_constructor(arg_list<Ts> {})...);
    } else {
        _constructor = scripting::wrapper<wrapper<T>>::make_unique_closure(process_constructor(arg_list<Ts> {})...);
    }

    // create 'new' function
    dstTable[_name]["new"] = _constructor.get();
}

template <typename T>
template <typename... Args>
inline auto wrapper<T>::process_constructor(arg_list<T(Args...)>) -> std::function<managed_ptr<T>(Args...)>
{
    return std::function<managed_ptr<T>(Args...)> {[](Args... args) {
        return managed_ptr<T> {new T(std::forward<Args>(args)...)};
    }};
}

////////////////////////////////////////////////////////////

template <typename T>
inline wrapper<T>::wrapper(state_view view, table* globaltable, string name)
    : _name {std::move(name)}
    , _globalTable {globaltable}
    , _view {view}
{
    create_metatable(typeid(T).name(), false);
    create_metatable((string {typeid(T).name()} + "_gc"), true);
}

template <typename T>
inline wrapper<T>::~wrapper()
{
    remove_metatable(typeid(T).name());
    remove_metatable((string {typeid(T).name()} + "_gc"));
}

////////////////////////////////////////////////////////////

template <typename T>
inline void wrapper<T>::hide_metatable(auto&& value) const
{
    set_metatable_field("__metatable", typeid(T).name(), value);
    set_metatable_field("__metatable", (string {typeid(T).name()} + "_gc"), value);
}

template <typename T>
inline void wrapper<T>::set_metatable_field(string const& name, string const& tableName, auto&& value) const
{
    _view.get_metatable(tableName);

    i32 const top {_view.get_top()};
    _view.push_convert(name);
    _view.push_convert(value);
    _view.raw_set(top);

    _view.pop(1);
}

template <typename T>
inline void wrapper<T>::create_metatable(string const& name, bool gc)
{
    _view.new_metatable(name);
    i32 const tableIdx {_view.get_top()};

    // set __name
    _view.push_string("__name");
    _view.push_string(_name);
    _view.raw_set(tableIdx);

    // set __type
    _view.push_string("__type");
    _view.push_string(name);
    _view.raw_set(tableIdx);

    // index metamethod
    push_metamethod("__index",
                    std::function {[&](T* instance, std::variant<i32, string>& arg) {
                        if (auto* arg0 {std::get_if<i32>(&arg)}) {
                            this->index(instance, *arg0);
                        } else if (auto* arg1 {std::get_if<string>(&arg)}) {
                            this->index(instance, *arg1);
                        }
                    }},
                    tableIdx);

    // newindex metamethod
    push_metamethod("__newindex",
                    std::function {[&](T* instance, std::variant<i32, string>& arg) {
                        if (auto* arg0 {std::get_if<i32>(&arg)}) {
                            this->newindex(instance, *arg0);
                        } else if (auto* arg1 {std::get_if<string>(&arg)}) {
                            this->newindex(instance, *arg1);
                        }
                    }},
                    tableIdx);

    // eq metamethod
    if constexpr (Equatable<T>) {
        push_metamethod("__eq",
                        std::function {[](T* instance1, T* instance2) { return *instance1 == *instance2; }},
                        tableIdx);
    }

    // lt metamethod
    if constexpr (LessComparable<T>) {
        push_metamethod("__lt",
                        std::function {[](T* instance1, T* instance2) { return *instance1 < *instance2; }},
                        tableIdx);
    }

    // le metamethod
    if constexpr (LessEqualComparable<T>) {
        push_metamethod("__le",
                        std::function {[](T* instance1, T* instance2) { return *instance1 <= *instance2; }},
                        tableIdx);
    }

    // unm metamethod
    if constexpr (Negatable<T>) {
        push_metamethod("__unm",
                        std::function {[](T* instance) { return managed_ptr<T> {new T(-*instance)}; }},
                        tableIdx);
    }

    // add metamethod
    if constexpr (Addable<T>) {
        push_metamethod("__add",
                        std::function {[](T* instance1, T* instance2) { return managed_ptr<T> {new T(*instance1 + *instance2)}; }},
                        tableIdx);
    }

    // sub metamethod
    if constexpr (Subtractable<T>) {
        push_metamethod("__sub",
                        std::function {[](T* instance1, T* instance2) { return managed_ptr<T> {new T(*instance1 - *instance2)}; }},
                        tableIdx);
    }

    // mul metamethod
    if constexpr (Multipliable<T>) {
        push_metamethod("__mul",
                        std::function {[](T* instance1, T* instance2) { return managed_ptr<T> {new T(*instance1 * *instance2)}; }},
                        tableIdx);
    }

    // div metamethod
    if constexpr (Dividable<T>) {
        push_metamethod("__div",
                        std::function {[](T* instance1, T* instance2) { return managed_ptr<T> {new T(*instance1 / *instance2)}; }},
                        tableIdx);
    }

    // length operator
    if constexpr (HasSize<T>) {
        push_metamethod("__len",
                        std::function {[](T* instance) {
                            return instance->size();
                        }},
                        tableIdx);
    }

    // gc for Lua created instances
    if constexpr (std::is_destructible_v<T>) {
        if (gc) {
            _view.push_convert("__gc");
            _view.push_cfunction(&wrapper::gc);
            _view.set_table(tableIdx);
        }
    }

    _view.pop(1);
}

template <typename T>
inline void wrapper<T>::remove_metatable(string const& name)
{
    _view.push_convert(nullptr);
    _view.set_registry_field(name);
}

template <typename T>
template <typename R, typename... P>
inline void wrapper<T>::push_metamethod(string const& methodname, std::function<R(P...)>&& func, i32 tableIdx)
{
    _view.push_convert(methodname);
    auto ptr {scripting::wrapper<wrapper<T>>::make_unique_closure(std::move(func))};
    _view.push_convert(ptr.get());
    _view.raw_set(tableIdx);
    _metamethods.push_back(std::move(ptr));
}

template <typename T>
inline void wrapper<T>::index(T* b, i32 arg)
{
    if constexpr (detail::IntIndexable<T>) {
        if constexpr (HasSize<T>) {
            if (arg > std::ssize(*b)) {
                _view.push_nil(); // pushing null for ipairs
                return;
            }
        }

        _view.push_convert((*b)[static_cast<typename T::size_type>(arg - 1)]);
    } else {
        _view.push_nil();
    }
}

template <typename T>
inline void wrapper<T>::index(T* b, string const& arg)
{
    if constexpr (detail::StringIndexable<T>) {
        _view.push_convert((*b)[arg]);
    } else {
        if (auto fit {_functions.find(arg)}; fit != _functions.end()) {
            _view.push_convert(fit->second.get());
        } else if (auto git {_getters.find(arg)}; git != _getters.end()) {
            (*git->second)(_view);
        } else {
            unknown_get_event ev {b, arg, _view};
            UnknownGet(ev);
            if (!ev.Handled) { _view.push_nil(); }
        }
    }
}

template <typename T>
inline void wrapper<T>::newindex(T* b, i32 arg)
{
    _view.remove(2); // remove arg
    if constexpr (detail::IntIndexable<T>) {
        typename T::value_type val {};
        _view.pull_convert_idx(-1, val);
        _view.pop(_view.get_top());
        if constexpr (Container<T>) {
            if (arg == std::ssize(*b) + 1) { // add to vector if index is size + 1
                b->push_back(val);
                return;
            }
        }

        (*b)[static_cast<typename T::size_type>(arg - 1)] = val;
    } else {
        _view.error("unknown set: " + std::to_string(arg));
    }
}

template <typename T>
inline void wrapper<T>::newindex(T* b, string const& arg)
{
    _view.remove(2); // remove arg
    if constexpr (detail::StringIndexable<T>) {
        _view.pull_convert_idx(-1, (*b)[arg]);
    } else if (auto sit {_setters.find(arg)}; sit != _setters.end()) {
        (*sit->second)(_view);
    } else {
        unknown_set_event ev {b, arg, _view};
        UnknownSet(ev);
        if (!ev.Handled) { _view.error("unknown set: " + arg); }
    }
    _view.pop(_view.get_top());
}

template <typename T>
inline auto wrapper<T>::gc(lua_State* l) -> i32
{
    T** obj {static_cast<T**>(state_view {l}.to_userdata(-1))};

    if (obj && *obj) {
        delete (*obj); // NOLINT(cppcoreguidelines-owning-memory)
    }

    return 0;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <typename T>
inline wrapper<T>::unknown_get_event::unknown_get_event(T* instance, string name, state_view view)
    : Instance {instance}
    , Name {std::move(name)}
    , _view {view}
{
}

template <typename T>
inline void wrapper<T>::unknown_get_event::return_value(auto&& value) const
{
    _view.push_convert(std::move(value));
}

////////////////////////////////////////////////////////////

template <typename T>
inline wrapper<T>::unknown_set_event::unknown_set_event(T* instance, string name, state_view view)
    : Instance {instance}
    , Name {std::move(name)}
    , _view {view}
{
}

template <typename T>
template <typename X>
inline auto wrapper<T>::unknown_set_event::get_value(X& val) const -> bool
{
    if (converter<X>::IsType(_view, 2)) {
        return _view.pull_convert_idx(2, val);
    }

    return false;
}
}

#endif
