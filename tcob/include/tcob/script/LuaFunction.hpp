// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <future>

#include <tcob/core/io/Logger.hpp>
#include <tcob/script/LuaRef.hpp>
#include <tcob/script/LuaState.hpp>

namespace tcob::detail {
class LuaFunctionBase : public LuaRef {
public:
    void dump(OutputFileStream& stream) const;

protected:
    auto do_call(i32 nargs) const -> LuaResultState;
};
}

namespace tcob {
template <typename R>
class LuaFunction final : public detail::LuaFunctionBase {
public:
    template <typename... P>
    auto operator()(P&&... params) -> R
    {
        if constexpr (std::is_void_v<R>) {
            static_cast<void>(call(params...));
        } else {
            return call(params...).Value;
        }
    }

    template <typename... P>
    auto call(P&&... params) const -> LuaResult<R>
    {
        const LuaState ls { lua_state() };

        push_self();

        //push parameters to lua
        const i32 oldTop { ls.get_top() };
        ls.push(params...);
        const i32 paramsCount { ls.get_top() - oldTop };

        //call lua function
        const LuaResultState result { do_call(paramsCount) };
        if (result == LuaResultState::Ok) {
            if constexpr (std::is_void_v<R>) {
                ls.restore_top();
                return { LuaResultState::Ok };
            } else {
                R retValue {};
                bool ok { ls.try_get(1, retValue) };
                ls.restore_top();
                if (ok) {
                    return { retValue, LuaResultState::Ok };
                } else {
                    return { R(), LuaResultState::TypeMismatch };
                }
            }
        } else {
            ls.restore_top();
            if constexpr (std::is_void_v<R>) {
                return { result };
            } else {
                return { R(), result };
            }
        }
    }

    template <typename... P>
    auto call_async(P&&... params) const -> std::future<LuaResult<R>>
    {
        return std::async(std::launch::async, [this, params...] {
            return call<R>(params...);
        });
    }
};

enum class LuaCoroutineState {
    Ok,
    Suspended,
    Error
};

class LuaCoroutine final : public LuaRef {
public:
    template <typename R, typename... P>
    auto resume(P&&... params) const -> LuaResult<R>
    {
        lua_State* t { thread() };
        const LuaState ls { t };

        //push parameters to lua
        const i32 oldTop { ls.get_top() };
        ls.push(params...);
        const i32 paramsCount { ls.get_top() - oldTop };

        //call lua function
        i32 nresults { 0 };
        LuaThreadState err { ls.resume(paramsCount, &nresults) };

        if (err == LuaThreadState::Ok || err == LuaThreadState::Yielded) {
            if constexpr (std::is_void_v<R>) {
                ls.restore_top();
                return { LuaResultState::Ok };
            } else {
                R retValue {};
                bool ok { ls.try_get(1, retValue) };
                ls.restore_top();

                if (ok) {
                    return { retValue, err == LuaThreadState::Ok ? LuaResultState::Ok : LuaResultState::Yielded };
                } else {
                    return { R(), LuaResultState::TypeMismatch };
                }
            }
        } else {
            LuaResultState result;

            switch (err) {
            case LuaThreadState::RuntimeError:
                result = LuaResultState::RuntimeError;
                break;
            case LuaThreadState::MemError:
                result = LuaResultState::MemAllocError;
                break;
            default:
                result = LuaResultState::RuntimeError;
                break;
            }

            ls.restore_top();
            if constexpr (std::is_void_v<R>) {
                return { result };
            } else {
                return { R(), result };
            }
        }
    }

    template <typename... T>
    void push(T&&... t) const
    {
        LuaState { thread() }.push(t...);
    }

    auto close() const -> LuaCoroutineState;

    auto state() const -> LuaCoroutineState;

private:
    auto thread() const -> lua_State*;
};
}