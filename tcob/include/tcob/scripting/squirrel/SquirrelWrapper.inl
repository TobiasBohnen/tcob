// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SquirrelWrapper.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

namespace tcob::scripting::squirrel {

namespace detail {
    ////////////////////////////////////////////////////////////
    [[maybe_unused]] auto static get_metamethod_name(metamethod m) -> std::string
    {
        switch (m) {
        case metamethod::Add: return "_add";
        case metamethod::Subtract: return "_sub";
        case metamethod::Multiply: return "_mul";
        case metamethod::Divide: return "_div";
        case metamethod::Modulo: return "_mod";
        case metamethod::UnaryMinus: return "_unm";
        case metamethod::TypeOf: return "_typeof";
        case metamethod::Compare: return "_cmp";
        case metamethod::Call: return "_call";
        case metamethod::Cloned: return "_cloned";
        case metamethod::ToString: return "_tostring";
        }
        return "";
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline void wrapper<T>::wrap_metamethod(metamethod method, auto&& func)
{
    string const& name {detail::get_metamethod_name(method)};
    push_metamethod(name, std::function {func});
}

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
    return std::make_unique<detail::native_overload<std::remove_cvref_t<Funcs>...>>(false, std::make_tuple(std::forward<Funcs>(fns)...));
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
        _metaTable[name] = func.get();
        _functions[name] = std::move(func);
        break;
    }
}

template <typename T>
template <typename S>
inline void wrapper<T>::impl_register_type()
{
    static_assert(std::is_base_of_v<S, T>);

    std::unordered_set<string> types;
    _metaTable.try_get(types, "__types");
    if (!types.contains(typeid(S).name())) {
        types.insert(typeid(S).name());
        _metaTable["__types"] = types;
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline wrapper<T>::wrapper(vm_view view, table* rootTable, string name)
    : _name {std::move(name)}
    , _rootTable {rootTable}
    , _view {view}
{
    create_metatable(typeid(T).name());
}

template <typename T>
inline wrapper<T>::~wrapper()
{
    remove_metatable(typeid(T).name());
}

////////////////////////////////////////////////////////////

template <typename T>
inline void wrapper<T>::create_metatable(string const& name)
{
    _view.push_registrytable();
    _view.push_string(name);
    _view.new_table();
    _metaTable.acquire(_view, -1);
    _view.new_slot(-3, false);
    _view.pop(1);

    push_metamethod("_get", std::function {[&](T* b, string const& idx) {
                        if (_getters.contains(idx)) {
                            (*_getters[idx])(_view);
                        } else {
                            unknown_get_event ev {b, idx, _view};
                            UnknownGet(ev);
                            if (!ev.Handled) {
                                _view.throw_error("unknown get: " + idx);
                            }
                        }
                    }});
    push_metamethod("_set", std::function {[&](T* b, string const& idx) {
                        if (_setters.contains(idx)) {
                            _view.remove(2); // remove string index
                            (*_setters[idx])(_view);
                        } else {
                            unknown_set_event ev {b, idx, _view};
                            UnknownSet(ev);
                            if (!ev.Handled) {
                                _view.throw_error("unknown set: " + idx);
                            }
                        }
                    }});

    // cmp metamethod
    if constexpr (LessEqualComparable<T> && Equatable<T>) {
        push_metamethod("_cmp",
                        std::function {[](T* instance1, T* instance2) {
                            if (*instance1 == *instance2) {
                                return 0;
                            }
                            if (*instance1 < *instance2) {
                                return -1;
                            }
                            return 1;
                        }});
    }

    // unm metamethod
    if constexpr (Negatable<T>) {
        push_metamethod("_unm",
                        std::function {[](T* instance) { return script_owned_ptr<T> {new T(-*instance)}; }});
    }

    // add metamethod
    if constexpr (Addable<T>) {
        push_metamethod("_add",
                        std::function {[](T* instance1, T* instance2) { return script_owned_ptr<T> {new T(*instance1 + *instance2)}; }});
    }

    // sub metamethod
    if constexpr (Subtractable<T>) {
        push_metamethod("_sub",
                        std::function {[](T* instance1, T* instance2) { return script_owned_ptr<T> {new T(*instance1 - *instance2)}; }});
    }

    // mul metamethod
    if constexpr (Multipliable<T>) {
        push_metamethod("_mul",
                        std::function {[](T* instance1, T* instance2) { return script_owned_ptr<T> {new T(*instance1 * *instance2)}; }});
    }

    // div metamethod
    if constexpr (Dividable<T>) {
        push_metamethod("_div",
                        std::function {[](T* instance1, T* instance2) { return script_owned_ptr<T> {new T(*instance1 / *instance2)}; }});
    }
}

template <typename T>
inline void wrapper<T>::remove_metatable(string const& name)
{
    _view.push_registrytable();
    _view.push_string(name);
    _view.push_convert(nullptr);
    _view.new_slot(-3, false);
    _metaTable.release();
}

template <typename T>
template <typename R, typename... P>
inline void wrapper<T>::push_metamethod(string const& methodname, std::function<R(P...)>&& func)
{
    auto ptr {scripting::wrapper<wrapper<T>>::make_unique_closure(std::forward<std::function<R(P...)>>(std::move(func)))};
    _metaTable[methodname] = ptr.get();
    _metamethods.push_back(std::move(ptr));
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <typename T>
inline wrapper<T>::unknown_get_event::unknown_get_event(T* instance, string name, vm_view view)
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
inline wrapper<T>::unknown_set_event::unknown_set_event(T* instance, string name, vm_view view)
    : Instance {instance}
    , Name {std::move(name)}
    , _view {view}
{
}

template <typename T>
template <typename X>
inline auto wrapper<T>::unknown_set_event::get_value(X& val) const -> bool
{
    if (converter<X>::IsType(_view, -2)) {
        return _view.pull_convert_idx(-2, val);
    }

    return false;
}

}

#endif
