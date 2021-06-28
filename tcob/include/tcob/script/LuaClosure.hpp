// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>

#include <tcob/script/LuaState.hpp>

namespace tcob::detail {
class LuaClosureBase {
public:
    LuaClosureBase() = default;
    virtual ~LuaClosureBase() = default;

    virtual auto operator()(lua_State* l) -> i32 = 0;
    virtual auto compare_args_to_stack(lua_State* l, i32 args) -> bool = 0;
};

template <typename R, typename... P>
class LuaClosure;
template <typename R, typename... P>
class LuaClosure<R(P...)> : public LuaClosureBase {
    using func = std::function<R(P...)>;

public:
    explicit LuaClosure(func& fn)
        : _fn { fn }
    {
        if constexpr (sizeof...(P) > 0)
            _numArgs = count_args<P...>();
    }

    virtual ~LuaClosure()
    {
        _fn = nullptr;
    }

    auto operator()(lua_State* l) -> i32 override
    {
        return invoke(l);
    }

    auto compare_args_to_stack(lua_State* l, i32 args) -> bool override
    {
        if constexpr (sizeof...(P) == 0) {
            return args == 0 || (args == 1 && LuaState { l }.is_userdata(args));
        } else if (args == _numArgs) {
            return compare_types<P...>(l, 1);
        } else {
            return false;
        }
    }

    auto invoke(lua_State* l) -> i32
    {
        std::tuple<std::remove_cvref_t<P>...> params;
        LuaState ls { l };

        std::apply([&ls](auto&... item) {
            //FIXME: check parameters
            i32 idx { 1 };
            (ls.try_get(std::forward<i32>(idx), item), ...);
            static_cast<void>(idx);
        },
            params);

        const i32 oldTop { ls.get_top() };
        if constexpr (std::is_void_v<R>) {
            std::apply(_fn, params);
        } else {
            const R result { std::apply(_fn, params) };
            ls.push(result);
        }
        return ls.get_top() - oldTop;
    }

private:
    template <typename T, typename... Args>
    auto compare_types(lua_State* l, i32 startIndex) const -> bool
    {
        bool result { LuaConverter<std::remove_cvref_t<T>>::IsType(LuaState { l }, startIndex) };

        if constexpr (sizeof...(Args) > 0) {
            using Type = std::remove_cvref_t<T>;
            startIndex += LuaConverter<Type>::StackSlots;
            result = result && compare_types<Args...>(l, startIndex);
        }
        return result;
    }

    template <typename T, typename... Args>
    constexpr auto count_args() -> i32
    {
        using Type = std::remove_cvref_t<T>;

        if constexpr (sizeof...(Args) > 0) {
            return LuaConverter<Type>::StackSlots + count_args<Args...>();
        } else {
            return LuaConverter<Type>::StackSlots;
        }
    }

    i32 _numArgs { 0 };
    func _fn;
};

}
namespace tcob {
using LuaClosureUniquePtr = std::unique_ptr<detail::LuaClosureBase>;
using LuaClosureSharedPtr = std::shared_ptr<detail::LuaClosureBase>;

template <typename R, typename... P>
auto make_unique_luaclosure(std::function<R(P...)>&& fn) -> LuaClosureUniquePtr
{
    return std::make_unique<detail::LuaClosure<R(P...)>>(fn);
}

template <typename R, typename... P>
auto make_shared_luaclosure(std::function<R(P...)>&& fn) -> LuaClosureSharedPtr
{
    return std::make_shared<detail::LuaClosure<R(P...)>>(fn);
}
}