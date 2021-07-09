// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <functional>
#include <vector>

#include <tcob/core/Helper.hpp>
#include <tcob/script/LuaClosure.hpp>
#include <tcob/script/LuaState.hpp>
#include <tcob/script/LuaTable.hpp>

namespace tcob::lua {
template <typename T>
struct LuaOwnedPtr {
    explicit LuaOwnedPtr(T* obj)
        : Obj { obj }
    {
    }
    T* Obj { nullptr };
};

enum class Metamethod : char {
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
    Involution,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    BitwiseNot,
    LeftShift,
    RightShift
};

namespace detail {
    class WrapperBase {
    };

    template <class... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    template <typename T>
    concept Equatable = requires(T& t)
    {
        { t == t };
    };

    template <typename T>
    concept LessComparable = requires(T& t)
    {
        { t < t };
    };

    template <typename T>
    concept LessEqualComparable = requires(T& t)
    {
        { t <= t };
    };

    template <typename T>
    concept Negatable = requires(T& t)
    {
        { -t };
    };

    template <typename T>
    concept Addable = requires(T& t)
    {
        { t + t };
    };

    template <typename T>
    concept Subtractable = requires(T& t)
    {
        { t - t };
    };

    template <typename T>
    concept Multipliable = requires(T& t)
    {
        { t * t };
    };

    template <typename T>
    concept Dividable = requires(T& t)
    {
        { t / t };
    };

    template <typename T>
    concept IntIndexable = requires(T& t, i32&& u)
    {
        typename T::value_type;
        { t[u] };
    };

    template <typename T>
    concept StringIndexable = requires(T& t, std::string&& u)
    {
        typename T::key_type;
        typename T::mapped_type;
        typename T::value_type;
        { t[u] };
    };
}

template <typename T>
class Wrapper final : public detail::WrapperBase {
public:
    Wrapper(const State& state, Table* globaltable, const std::string& name)
        : _name { name }
        , _globalTable { globaltable }
        , _state { state }
    {
        create_metatable(typeid(T).name(), false);
        create_metatable((std::string(typeid(T).name()) + "_gc").c_str(), true);
    }

    ~Wrapper()
    {
        remove_metatable(typeid(T).name());
        remove_metatable((std::string(typeid(T).name()) + "_gc").c_str());
    }

    // define member function
    template <typename R, typename... P>
    void function(const std::string& name, R (T::*func)(P...))
    {
        const auto lambda { [func](T* instance, P&&... args) {
            return (instance->*func)(std::forward<P>(args)...);
        } };
        _funcs[name] = make_unique_closure(std::function<R(T*, P...)>(lambda));
    }

    template <typename R, typename... P>
    void function(const std::string& name, R (T::*func)(P...) const)
    {
        const auto lambda { [func](T* instance, P&&... args) {
            return (instance->*func)(std::forward<P>(args)...);
        } };
        _funcs[name] = make_unique_closure(std::function<R(T*, P...)>(lambda));
    }

    // define non-member function
    template <typename Func>
    void function(const std::string& name, Func func)
    {
        _funcs[name] = make_unique_closure(std::function(func));
    }

    // define overloaded function
    template <typename... Func>
    void function(const std::string& name, Func... funcs)
    {
        overload(_funcs, name, funcs...);
    }

    // define property
    template <typename Get, typename Set>
    void property(const std::string& name, Get get, Set set)
    {
        getter(name, get);
        setter(name, set);
    }

    template <typename R>
    void property(const std::string& name, R T::*property)
    {
        getter(name, property);
        setter(name, property);
    }

    template <typename R>
    void getter(const std::string& name, const R (T::*getter)() const)
    {
        const auto getterlambda { [getter](T* instance) -> R { return (instance->*getter)(); } };
        this->getter(name, getterlambda);
    }

    template <typename R>
    void getter(const std::string& name, R (T::*getter)() const)
    {
        const auto getterlambda { [getter](T* instance) -> R { return (instance->*getter)(); } };
        this->getter(name, getterlambda);
    }

    template <typename R>
    void getter(const std::string& name, R* (T::*getter)())
    {
        auto getterlambda { [getter](T* instance) -> R* { return (instance->*getter)(); } };
        this->getter(name, getterlambda);
    }

    template <typename R>
    void getter(const std::string& name, R T::*field)
    {
        const auto getterlambda { [field](T* instance) -> R { return (instance->*field); } };
        this->getter(name, getterlambda);
    }

    template <typename Func>
    void getter(const std::string& name, Func func)
    {
        _getters[name] = make_unique_closure(std::function(func));
    }

    template <typename R>
    void setter(const std::string& name, void (T::*setter)(const R))
    {
        const auto setterlambda { [setter](T* instance, R arg) { (instance->*setter)(arg); } };
        this->setter(name, setterlambda);
    }

    template <typename R>
    void setter(const std::string& name, void (T::*setter)(const R) const)
    {
        const auto setterlambda { [setter](T* instance, R arg) { (instance->*setter)(arg); } };
        this->setter(name, setterlambda);
    }

    template <typename R>
    void setter(const std::string& name, R T::*field)
    {
        const auto setterlambda { [field](T* instance, R arg) { (instance->*field) = (arg); } };
        this->setter(name, setterlambda);
    }

    template <typename Func>
    void setter(const std::string& name, Func func)
    {
        _setters[name] = make_unique_closure(std::function(func));
    }

    template <typename... Func>
    void setters(const std::string& name, Func... funcs)
    {
        overload(_setters, name, funcs...);
    }

    // define constructor
    template <typename... P>
    void constructor()
    {
        // create constructor table
        if (!_globalTable->has(_name)) {
            _globalTable->create_table(_name);
        }
        // create 'new' function
        if (!_globalTable->has(_name, "new")) {
            auto ptr { make_unique_closure(
                std::function<void(void)>(
                    [this]() {
                        this->overload_resolution("new");
                    })) };
            (*_globalTable)[_name]["new"] = ptr.get();
            _metamethods.push_back(std::move(ptr));
        }

        // create constructor lambda
        auto lambda { [](P... args) {
            return LuaOwnedPtr<T>(new T(std::forward<P>(args)...));
        } };

        auto constr { make_unique_closure(std::function<LuaOwnedPtr<T>(P...)>(lambda)) };
        _constructors.push_back(std::move(constr));
    }

    // define metamethod
    template <typename Func>
    void metamethod(Metamethod method, Func func)
    {
        std::string name = metamethodsMap[method];
        if (!_overloads.contains(name)) {
            auto lambda { [this, name]() { this->overload_resolution(name); } };
            auto ptr { make_unique_closure(std::function(lambda)) };
            set_metamethod(name, typeid(T).name(), ptr);
            set_metamethod(name, (std::string(typeid(T).name()) + "_gc").c_str(), ptr);
            _metamethods.push_back(std::move(ptr));
        }

        auto ptr { make_unique_closure(std::function(func)) };
        _overloads[name].push_back(std::move(ptr));
    }

private:
    void set_metamethod(const std::string& name, const char* tableName, ClosureUniquePtr& ptr)
    {
        _state.get_metatable(tableName);

        i32 top { _state.get_top() };
        _state.push(name);
        _state.push(ptr.get());
        _state.set_table(top);

        _state.pop(1);
    }

    template <typename R, typename... P, typename... Func>
    void overload(std::unordered_map<std::string, ClosureUniquePtr>& target, const std::string& name, R (T::*func)(P...), Func... funcs)
    {
        // wrap member functions
        const auto lambda { [func](T* instance, auto&&... args) {
            return (instance->*func)(std::forward<decltype(args)>(args)...);
        } };

        _overloads[name].push_back(make_unique_closure(std::function<R(T*, P...)>(lambda)));
        overload(target, name, funcs...);
    }

    template <typename R, typename... P, typename... Func>
    void overload(std::unordered_map<std::string, ClosureUniquePtr>& target, const std::string& name, R (T::*func)(P...) const, Func... funcs)
    {
        // wrap member functions
        const auto lambda { [func](T* instance, auto&&... args) {
            return (instance->*func)(std::forward<decltype(args)>(args)...);
        } };

        _overloads[name].push_back(make_unique_closure(std::function<R(T*, P...)>(lambda)));
        overload(target, name, funcs...);
    }

    template <typename Func, typename... Funcs>
    void overload(std::unordered_map<std::string, ClosureUniquePtr>& target, const std::string& name, Func func, Funcs... funcs)
    {
        _overloads[name].push_back(make_unique_closure(std::function(func)));
        overload(target, name, funcs...);
    }

    void overload(std::unordered_map<std::string, ClosureUniquePtr>& target, const std::string& name)
    {
        auto lambda { [this, name]() { this->overload_resolution(name); } };
        target[name] = make_unique_closure(std::function<void(void)>(lambda));
    }

    void overload_resolution(const std::string& name)
    {
        std::vector<ClosureUniquePtr>* funcs;
        if (name == "new") {
            funcs = &_constructors;
        } else {
            funcs = &_overloads[name];
        }
        const i32 top { _state.get_top() };

        if (funcs->size() == 1) { //if we only have one: call it
            (*(*funcs)[0])(_state.lua());
        } else {
            for (auto& func : *funcs) {
                if (func->compare_args_to_stack(_state.lua(), top)) {
                    (*func)(_state.lua());
                    break;
                }
            }
        }
    }

    void create_metatable(const char* name, bool gc)
    {
        _state.new_metatable(name);
        const i32 tableIdx { _state.get_top() };

        // index metamethod
        push_metamethod(
            "__index",
            std::function([this](T* instance, std::variant<i32, std::string>& var) { this->index(instance, var); }),
            tableIdx);

        // newindex metamethod
        push_metamethod(
            "__newindex",
            std::function([this](T* instance, std::variant<i32, std::string>& var) { this->newindex(instance, var); }),
            tableIdx);

        // eq metamethod
        if constexpr (has_equal_to) {
            push_metamethod(
                "__eq",
                std::function([](T* instance1, T* instance2) { return *instance1 == *instance2; }),
                tableIdx);
        }

        // lt metamethod
        if constexpr (has_less) {
            push_metamethod(
                "__lt",
                std::function([](T* instance1, T* instance2) { return *instance1 < *instance2; }),
                tableIdx);
        }

        // le metamethod
        if constexpr (has_less_equal) {
            push_metamethod(
                "__le",
                std::function([](T* instance1, T* instance2) { return *instance1 <= *instance2; }),
                tableIdx);
        }

        // unm metamethod
        if constexpr (has_unary_minus) {
            push_metamethod(
                "__unm",
                std::function([](T* instance) { return LuaOwnedPtr<T> { new T(-*instance) }; }),
                tableIdx);
        }

        // add metamethod
        if constexpr (has_plus) {
            push_metamethod(
                "__add",
                std::function([](T* instance1, T* instance2) { return LuaOwnedPtr<T> { new T(*instance1 + *instance2) }; }),
                tableIdx);
        }

        // sub metamethod
        if constexpr (has_minus) {
            push_metamethod(
                "__sub",
                std::function([](T* instance1, T* instance2) { return LuaOwnedPtr<T> { new T(*instance1 - *instance2) }; }),
                tableIdx);
        }

        // mul metamethod
        if constexpr (has_multiplies) {
            push_metamethod(
                "__mul",
                std::function([](T* instance1, T* instance2) { return LuaOwnedPtr<T> { new T(*instance1 * *instance2) }; }),
                tableIdx);
        }

        // div metamethod
        if constexpr (has_divides) {
            push_metamethod(
                "__div",
                std::function([](T* instance1, T* instance2) { return LuaOwnedPtr<T> { new T(*instance1 / *instance2) }; }),
                tableIdx);
        }

        // length operator when T is vector
        if constexpr (is_vector) {
            push_metamethod(
                "__len",
                std::function([](T* instance) { return instance->size(); }),
                tableIdx);
        }

        // gc for Lua created instances
        if (gc) {
            if constexpr (std::is_destructible_v<T>) {
                _state.push("__gc");
                _state.push_cfunction(&Wrapper::gc);
                _state.set_table(tableIdx);
            }
        }

        _state.pop(1);
    }

    void remove_metatable(const char* name)
    {
        _state.push(nullptr);
        _state.set_registry_field(name);
    }

    template <typename R, typename... P>
    void push_metamethod(const std::string& methodname, std::function<R(P...)>&& func, i32 idx)
    {
        _state.push(methodname);
        auto ptr { make_unique_closure(std::forward<std::function<R(P...)>>(func)) };
        _state.push(ptr.get());
        _state.set_table(idx);
        _metamethods.push_back(std::move(ptr));
    }

    void index(T* b, std::variant<i32, std::string>& var)
    {
        std::visit(
            detail::overloaded {
                // int index
                [&]([[maybe_unused]] i32 arg) {
                    if constexpr (is_int_indexable) {
                        if constexpr (is_vector) {
                            if (arg > b->size()) {
                                _state.push(nullptr);
                                return;
                            }
                        }

                        _state.push((*b)[arg - 1]);
                    } else {
                        // TODO: log
                        _state.push(&Wrapper::noop);
                    }
                },
                // string index
                [&](const std::string& arg) {
                    if constexpr (is_string_indexable) {
                        _state.push((*b)[arg]);
                    } else {
                        if (_funcs.contains(arg)) {
                            _state.push(_funcs[arg].get());
                        } else if (_getters.contains(arg)) {
                            (*_getters[arg])(_state.lua());
                        } else {
                            // TODO: log
                            _state.push(&Wrapper::noop);
                        }
                    }
                },
            },
            var);
    }

    void newindex(T* b, std::variant<i32, std::string>& var)
    {
        _state.remove(2);
        std::visit(
            detail::overloaded {
                // int index
                [&]([[maybe_unused]] i32 arg) {
                    if constexpr (is_int_indexable) {
                        auto val { _state.get<typename T::value_type>(2) };
                        if constexpr (is_vector) {
                            if (arg == b->size() + 1) {
                                b->push_back(val);
                                return;
                            }
                        }

                        (*b)[arg - 1] = val;
                    }
                },
                // string index
                [&](const std::string& arg) {
                    if constexpr (is_string_indexable) {
                        (*b)[arg] = _state.get<typename T::mapped_type>(2);
                    } else {
                        (*_setters[arg])(_state.lua());
                    }
                },
            },
            var);
    }

    static auto gc(lua_State* l) -> i32
    {
        T** obj { static_cast<T**>(State { l }.to_userdata(-1)) };

        if (obj && *obj)
            delete (*obj);

        return 0;
    }

    static void noop()
    {
        // TODO: log
    }

    std::unordered_map<std::string, ClosureUniquePtr> _funcs;
    std::unordered_map<std::string, ClosureUniquePtr> _getters;
    std::unordered_map<std::string, ClosureUniquePtr> _setters;
    std::unordered_map<std::string, std::vector<ClosureUniquePtr>> _overloads;
    std::vector<ClosureUniquePtr> _constructors;
    std::vector<ClosureUniquePtr> _metamethods;

    std::string _name;
    Table* _globalTable;
    State _state;

    static constexpr bool is_vector { tcob::detail::is_specialization<T, std::vector>() };
    static constexpr bool is_string_indexable { detail::StringIndexable<T> };
    static constexpr bool is_int_indexable { detail::IntIndexable<T> };

    static constexpr bool has_equal_to { detail::Equatable<T> };
    static constexpr bool has_less { detail::LessComparable<T> };
    static constexpr bool has_less_equal { detail::LessEqualComparable<T> };
    static constexpr bool has_unary_minus { detail::Negatable<T> };
    static constexpr bool has_plus { detail::Addable<T> };
    static constexpr bool has_minus { detail::Subtractable<T> };
    static constexpr bool has_multiplies { detail::Multipliable<T> };
    static constexpr bool has_divides { detail::Dividable<T> };

    inline static std::unordered_map<Metamethod, std::string> metamethodsMap = {
        { Metamethod::Length, "__len" },
        { Metamethod::ToString, "__tostring" },
        { Metamethod::UnaryMinus, "__unm" },
        { Metamethod::Concat, "__concat" },
        { Metamethod::Add, "__add" },
        { Metamethod::Subtract, "__sub" },
        { Metamethod::Multiply, "__mul" },
        { Metamethod::Divide, "__div" },
        { Metamethod::LessThan, "__lt" },
        { Metamethod::LessOrEqualThan, "__le" },
        { Metamethod::Call, "__call" },
        { Metamethod::FloorDivide, "__idiv" },
        { Metamethod::Modulo, "__mod" },
        { Metamethod::Involution, "__pow" },
        { Metamethod::BitwiseAnd, "__band" },
        { Metamethod::BitwiseOr, "__bor" },
        { Metamethod::BitwiseXor, "__bxor" },
        { Metamethod::BitwiseNot, "__bnot" },
        { Metamethod::LeftShift, "__shl" },
        { Metamethod::RightShift, "__shr" }
    };
};

}