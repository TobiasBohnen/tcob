// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Wrapper.hpp"

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <variant>

#include "tcob/core/Common.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/scripting/Closure.hpp"
#include "tcob/scripting/Lua.hpp"
#include "tcob/scripting/Scripting.hpp"
#include "tcob/scripting/Types.hpp"

namespace tcob::scripting {

namespace detail {

    ////////////////////////////////////////////////////////////
    [[maybe_unused]] static auto get_metamethod_name(metamethod_type m) -> string
    {
        switch (m) {
        case metamethod_type::Length:          return "__len";
        case metamethod_type::ToString:        return "__tostring";
        case metamethod_type::UnaryMinus:      return "__unm";
        case metamethod_type::Add:             return "__add";
        case metamethod_type::Subtract:        return "__sub";
        case metamethod_type::Divide:          return "__div";
        case metamethod_type::Multiply:        return "__mul";
        case metamethod_type::Concat:          return "__concat";
        case metamethod_type::LessThan:        return "__lt";
        case metamethod_type::LessOrEqualThan: return "__le";
        case metamethod_type::Call:            return "__call";
        case metamethod_type::FloorDivide:     return "__idiv";
        case metamethod_type::Modulo:          return "__mod";
        case metamethod_type::PowerOf:         return "__pow";
        case metamethod_type::BitwiseAnd:      return "__band";
        case metamethod_type::BitwiseOr:       return "__bor";
        case metamethod_type::BitwiseXor:      return "__bxor";
        case metamethod_type::BitwiseNot:      return "__bnot";
        case metamethod_type::LeftShift:       return "__shl";
        case metamethod_type::RightShift:      return "__shr";
        case metamethod_type::Close:           return "__close";
        }

        return "";
    }
}

////////////////////////////////////////////////////////////

template <typename WrappedType>
inline wrapper<WrappedType>::unknown_set_event::unknown_set_event(WrappedType* instance, string name, state_view view)
    : Instance {instance}
    , Name {std::move(name)}
    , _view {view}
{
}

template <typename WrappedType>
template <typename X>
inline auto wrapper<WrappedType>::unknown_set_event::get_value(X& val) -> bool
{
    if (converter<X>::IsType(_view, 2)) {
        if (_view.pull_convert_idx(2, val)) {
            Handled = true;
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////

template <typename WrappedType>
template <auto IsMeta>
inline wrapper<WrappedType>::proxy<IsMeta>::proxy(wrapper& parent, string name)
    : _parent {parent}
    , _name {std::move(name)}
{
}

template <typename WrappedType>
template <auto IsMeta>
template <typename T>
inline auto wrapper<WrappedType>::proxy<IsMeta>::operator=(T const& method) -> proxy&
{
    if constexpr (IsMeta) {
        auto ptr {make_unique_closure(std::function {method})};
        _parent.set_metatable_field(_name, typeid(WrappedType).name(), ptr.get());
        _parent.set_metatable_field(_name, (string(typeid(WrappedType).name()) + "_gc"), ptr.get());
        _parent._metamethods.push_back(std::move(ptr));
    } else {
        if constexpr (std::is_member_object_pointer_v<T>) {
            auto field {_parent.wrap_field_helper(method)};
            _parent.wrap(_name, wrap_target::Getter, std::move(field.first));
            _parent.wrap(_name, wrap_target::Setter, std::move(field.second));
        } else {
            auto ptr {_parent.wrap_method_helper(method)};
            _parent.wrap(_name, wrap_target::Method, std::move(ptr));
        }
    }
    return *this;
}

template <typename WrappedType>
template <auto IsMeta>
template <typename T>
inline auto wrapper<WrappedType>::proxy<IsMeta>::operator=(scripting::getter<T> const& get) -> proxy&
{
    static_assert(!IsMeta, "metamethods don't support properties");

    auto getter {_parent.wrap_method_helper(get.Method)};
    _parent.wrap(_name, wrap_target::Getter, std::move(getter));
    return *this;
}

template <typename WrappedType>
template <auto IsMeta>
template <typename T>
inline auto wrapper<WrappedType>::proxy<IsMeta>::operator=(scripting::setter<T> const& set) -> proxy&
{
    static_assert(!IsMeta, "metamethods don't support properties");

    auto setter {_parent.wrap_method_helper(set.Method)};
    _parent.wrap(_name, wrap_target::Setter, std::move(setter));
    return *this;
}

template <typename WrappedType>
template <auto IsMeta>
template <typename Get, typename Set>
inline auto wrapper<WrappedType>::proxy<IsMeta>::operator=(scripting::property<Get, Set> const& prop) -> proxy&
{
    static_assert(!IsMeta, "metamethods don't support properties");

    auto getter {_parent.wrap_method_helper(prop.first)};
    _parent.wrap(_name, wrap_target::Getter, std::move(getter));
    auto setter {_parent.wrap_method_helper(prop.second)};
    _parent.wrap(_name, wrap_target::Setter, std::move(setter));
    return *this;
}

template <typename WrappedType>
template <auto IsMeta>
template <typename... Ts>
inline auto wrapper<WrappedType>::proxy<IsMeta>::operator=(scripting::overload<Ts...> const& ov) -> proxy&
{
    static_assert(!IsMeta, "metamethods don't support overloads");

    std::apply([&](auto&&... item) {
        auto ptr {make_unique_overload(std::move(item)...)};
        _parent.wrap(_name, wrap_target::Method, std::move(ptr));
    },
               ov);
    return *this;
}

template <typename WrappedType>
inline auto wrapper<WrappedType>::operator[](string const& name) -> proxy<false>
{
    return proxy<false> {*this, name};
}

template <typename WrappedType>
inline auto wrapper<WrappedType>::operator[](metamethod_type type) -> proxy<true>
{
    return proxy<true> {*this, detail::get_metamethod_name(type)};
}

////////////////////////////////////////////////////////////

template <typename WrappedType>
template <typename S>
inline void wrapper<WrappedType>::register_base()
{
    static_assert(std::is_base_of_v<S, WrappedType>);
    _view.get_metatable(typeid(WrappedType).name());
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

template <typename WrappedType>
template <typename Func>
inline auto wrapper<WrappedType>::wrap_method_helper(Func&& func)
{
    return make_unique_closure(std::function {std::forward<Func>(func)});
}

template <typename WrappedType>
template <typename R, typename S, typename... Args>
inline auto wrapper<WrappedType>::wrap_method_helper(R (S::*func)(Args...))
{
    register_base<S>();
    return make_unique_closure(std::function<R(S*, Args...)> {func});
}

template <typename WrappedType>
template <typename R, typename S, typename... Args>
inline auto wrapper<WrappedType>::wrap_method_helper(R (S::*func)(Args...) const)
{
    register_base<S>();
    return make_unique_closure(std::function<R(S*, Args...)> {func});
}

template <typename WrappedType>
template <typename R, typename... Args>
inline auto wrapper<WrappedType>::wrap_method_helper(R (*func)(Args...))
{
    return make_unique_closure(std::function<R(Args...)> {func});
}

template <typename WrappedType>
template <typename R, typename S>
inline auto wrapper<WrappedType>::wrap_field_helper(R S::* field)
{
    register_base<S>();
    if constexpr (PropertyLike<R>) {
        using prop_type = R::const_return_type;
        auto lambdaG {[field](S* instance) -> prop_type { return *(instance->*field); }};
        auto lambdaS {[field](S* instance, prop_type value) -> void { (instance->*field) = value; }};
        return std::pair {make_unique_closure(std::function<prop_type(S*)> {lambdaG}), make_unique_closure(std::function<void(S*, prop_type)> {lambdaS})};
    } else {
        auto lambdaG {[field](S* instance) -> R { return (instance->*field); }};
        auto lambdaS {[field](S* instance, R value) -> void { (instance->*field) = value; }};
        return std::pair {make_unique_closure(std::function<R(S*)> {lambdaG}), make_unique_closure(std::function<void(S*, R)> {lambdaS})};
    }
}

template <typename WrappedType>
inline void wrapper<WrappedType>::wrap(string_view name, wrap_target target, native_closure_unique_ptr func)
{
    switch (target) {
    case wrap_target::Getter: _getters[string {name}] = std::move(func); break;
    case wrap_target::Setter: _setters[string {name}] = std::move(func); break;
    case wrap_target::Method: _functions[string {name}] = std::move(func); break;
    }
}

////////////////////////////////////////////////////////////

template <typename WrappedType>
template <typename... Ts>
inline void wrapper<WrappedType>::constructors(std::optional<table> targetTable)
{
    auto& dstTable {targetTable.has_value() ? targetTable.value() : *_globalTable};

    // create constructor table
    if (!dstTable.has(_name)) { dstTable[_name] = table {}; }

    if constexpr (sizeof...(Ts) > 1) {
        _constructor = make_unique_overload(wrap_constructor(arg_list<Ts> {})...);
    } else {
        _constructor = make_unique_closure(wrap_constructor(arg_list<Ts> {})...);
    }

    // create 'new' function
    dstTable[_name]["new"] = _constructor.get();
}

template <typename WrappedType>
template <typename... Args>
inline auto wrapper<WrappedType>::wrap_constructor(arg_list<WrappedType(Args...)>) -> std::function<managed_ptr<WrappedType>(Args...)>
{
    return std::function<managed_ptr<WrappedType>(Args...)> {[](Args... args) {
        return managed_ptr<WrappedType> {new WrappedType(std::forward<Args>(args)...)};
    }};
}

////////////////////////////////////////////////////////////

template <typename WrappedType>
inline wrapper<WrappedType>::wrapper(state_view view, table* globaltable, string name)
    : _name {std::move(name)}
    , _globalTable {globaltable}
    , _view {view}
{
    create_metatable(typeid(WrappedType).name(), false);
    create_metatable((string {typeid(WrappedType).name()} + "_gc"), true);
}

template <typename WrappedType>
inline wrapper<WrappedType>::~wrapper()
{
    remove_metatable(typeid(WrappedType).name());
    remove_metatable((string {typeid(WrappedType).name()} + "_gc"));
}

////////////////////////////////////////////////////////////

template <typename WrappedType>
inline void wrapper<WrappedType>::hide_metatable(auto&& value) const
{
    set_metatable_field("__metatable", typeid(WrappedType).name(), value);
    set_metatable_field("__metatable", (string {typeid(WrappedType).name()} + "_gc"), value);
}

template <typename WrappedType>
inline void wrapper<WrappedType>::set_metatable_field(string const& name, string const& tableName, auto&& value) const
{
    _view.get_metatable(tableName);

    i32 const top {_view.get_top()};
    _view.push_convert(name);
    _view.push_convert(value);
    _view.raw_set(top);

    _view.pop(1);
}

template <typename WrappedType>
inline void wrapper<WrappedType>::create_metatable(string const& name, bool gc)
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
                    std::function {[this](WrappedType* instance, std::variant<i32, string>& arg) {
                        if (auto* arg0 {std::get_if<i32>(&arg)}) {
                            this->index(instance, *arg0);
                        } else if (auto* arg1 {std::get_if<string>(&arg)}) {
                            this->index(instance, *arg1);
                        }
                    }},
                    tableIdx);

    // newindex metamethod
    push_metamethod("__newindex",
                    std::function {[this](WrappedType* instance, std::variant<i32, string>& arg) {
                        if (auto* arg0 {std::get_if<i32>(&arg)}) {
                            this->newindex(instance, *arg0);
                        } else if (auto* arg1 {std::get_if<string>(&arg)}) {
                            this->newindex(instance, *arg1);
                        }
                    }},
                    tableIdx);

    // eq metamethod
    if constexpr (Equatable<WrappedType>) {
        push_metamethod("__eq",
                        std::function {[](WrappedType* instance1, WrappedType* instance2) { return *instance1 == *instance2; }},
                        tableIdx);
    }

    // lt metamethod
    if constexpr (LessComparable<WrappedType>) {
        push_metamethod("__lt",
                        std::function {[](WrappedType* instance1, WrappedType* instance2) { return *instance1 < *instance2; }},
                        tableIdx);
    }

    // le metamethod
    if constexpr (LessEqualComparable<WrappedType>) {
        push_metamethod("__le",
                        std::function {[](WrappedType* instance1, WrappedType* instance2) { return *instance1 <= *instance2; }},
                        tableIdx);
    }

    // unm metamethod
    if constexpr (Negatable<WrappedType>) {
        push_metamethod("__unm",
                        std::function {[](WrappedType* instance) { return managed_ptr<WrappedType> {new WrappedType(-*instance)}; }},
                        tableIdx);
    }

    // add metamethod
    if constexpr (Addable<WrappedType>) {
        push_metamethod("__add",
                        std::function {[](WrappedType* instance1, WrappedType* instance2) { return managed_ptr<WrappedType> {new WrappedType(*instance1 + *instance2)}; }},
                        tableIdx);
    }

    // sub metamethod
    if constexpr (Subtractable<WrappedType>) {
        push_metamethod("__sub",
                        std::function {[](WrappedType* instance1, WrappedType* instance2) { return managed_ptr<WrappedType> {new WrappedType(*instance1 - *instance2)}; }},
                        tableIdx);
    }

    // mul metamethod
    if constexpr (Multipliable<WrappedType>) {
        push_metamethod("__mul",
                        std::function {[](WrappedType* instance1, WrappedType* instance2) { return managed_ptr<WrappedType> {new WrappedType(*instance1 * *instance2)}; }},
                        tableIdx);
    }

    // div metamethod
    if constexpr (Dividable<WrappedType>) {
        push_metamethod("__div",
                        std::function {[](WrappedType* instance1, WrappedType* instance2) { return managed_ptr<WrappedType> {new WrappedType(*instance1 / *instance2)}; }},
                        tableIdx);
    }

    // length operator
    if constexpr (HasSize<WrappedType>) {
        push_metamethod("__len",
                        std::function {[](WrappedType* instance) {
                            return instance->size();
                        }},
                        tableIdx);
    }

    // gc for Lua created instances
    if constexpr (std::is_destructible_v<WrappedType>) {
        if (gc) {
            _view.push_convert("__gc");
            _view.push_cfunction(&wrapper::gc);
            _view.set_table(tableIdx);
        }
    }

    _view.pop(1);
}

template <typename WrappedType>
inline void wrapper<WrappedType>::remove_metatable(string const& name)
{
    _view.push_convert(nullptr);
    _view.set_registry_field(name);
}

template <typename WrappedType>
template <typename R, typename... P>
inline void wrapper<WrappedType>::push_metamethod(string const& methodname, std::function<R(P...)>&& func, i32 tableIdx)
{
    _view.push_convert(methodname);
    auto ptr {make_unique_closure(std::move(func))};
    _view.push_convert(ptr.get());
    _view.raw_set(tableIdx);
    _metamethods.push_back(std::move(ptr));
}

template <typename WrappedType>
inline void wrapper<WrappedType>::index(WrappedType* b, i32 arg)
{
    if constexpr (detail::IntIndexable<WrappedType>) {
        if constexpr (HasSize<WrappedType>) {
            if (arg > std::ssize(*b)) {
                _view.push_nil(); // pushing null for ipairs
                return;
            }
        }

        _view.push_convert((*b)[static_cast<typename WrappedType::size_type>(arg - 1)]);
    } else {
        _view.push_nil();
    }
}

template <typename WrappedType>
inline void wrapper<WrappedType>::index(WrappedType* b, string const& arg)
{
    if constexpr (detail::StringIndexable<WrappedType>) {
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

template <typename WrappedType>
inline void wrapper<WrappedType>::newindex(WrappedType* b, i32 arg)
{
    _view.remove(2); // remove arg
    if constexpr (detail::IntIndexable<WrappedType>) {
        typename WrappedType::value_type val {};
        _view.pull_convert_idx(-1, val);
        _view.pop(_view.get_top());
        if constexpr (Container<WrappedType>) {
            if (arg == std::ssize(*b) + 1) { // add to vector if index is size + 1
                b->push_back(val);
                return;
            }
        }

        (*b)[static_cast<typename WrappedType::size_type>(arg - 1)] = val;
    } else {
        _view.error("unknown set: " + std::to_string(arg));
    }
}

template <typename WrappedType>
inline void wrapper<WrappedType>::newindex(WrappedType* b, string const& arg)
{
    _view.remove(2); // remove arg
    if constexpr (detail::StringIndexable<WrappedType>) {
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

template <typename WrappedType>
inline auto wrapper<WrappedType>::gc(lua_State* l) -> i32
{
    WrappedType** obj {static_cast<WrappedType**>(state_view {l}.to_userdata(-1))};
    if (obj && *obj) { delete (*obj); } // NOLINT(cppcoreguidelines-owning-memory)
    return 0;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <typename WrappedType>
inline wrapper<WrappedType>::unknown_get_event::unknown_get_event(WrappedType* instance, string name, state_view view)
    : Instance {instance}
    , Name {std::move(name)}
    , _view {view}
{
}

template <typename WrappedType>
inline void wrapper<WrappedType>::unknown_get_event::return_value(auto&& value)
{
    _view.push_convert(std::move(value));
    Handled = true;
}
}
