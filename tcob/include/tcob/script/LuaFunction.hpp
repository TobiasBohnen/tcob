// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <future>

#include <tcob/core/io/Logger.hpp>
#include <tcob/script/LuaRef.hpp>

namespace tcob::lua::detail {
class FunctionBase : public Ref {
public:
    void dump(OutputFileStream& stream) const;

protected:
    auto do_call(i32 nargs) const -> ResultState;
};
}

namespace tcob::lua {
template <typename R>
class Function final : public detail::FunctionBase {
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
    auto call(P&&... params) const -> Result<R>
    {
        const auto& ls { state() };
        const auto guard { ls.create_stack_guard() };

        push_self();

        //push parameters to lua
        const i32 oldTop { ls.get_top() };
        ls.push(params...);
        const i32 paramsCount { ls.get_top() - oldTop };

        //call lua function
        auto result { do_call(paramsCount) };
        if constexpr (std::is_void_v<R>) {
            return { result };
        } else {
            R retValue {};
            if (result == ResultState::Ok) {
                if (!ls.try_get(1, retValue)) {
                    result = ResultState::TypeMismatch;
                }
            }

            return { retValue, result };
        }
    }

    template <typename... P>
    auto call_async(P&&... params) const -> std::future<Result<R>>
    {
        return std::async(std::launch::async, [this, params...] {
            return call<R>(params...);
        });
    }
};

enum class CoroutineState {
    Ok,
    Suspended,
    Error
};

class Coroutine final : public Ref {
public:
    template <typename R, typename... P>
    auto resume(P&&... params) const -> Result<R>
    {
        const State t { thread() };
        const auto guard { t.create_stack_guard() };

        //push parameters to lua
        const i32 oldTop { t.get_top() };
        t.push(params...);
        const i32 paramsCount { t.get_top() - oldTop };

        //call lua function
        i32 nresults { 0 };
        const auto err { t.resume(paramsCount, &nresults) };
        if (err == ThreadState::Ok || err == ThreadState::Yielded) {
            if constexpr (std::is_void_v<R>) {
                return { ResultState::Ok };
            } else {
                R retValue {};
                if (t.try_get(1, retValue)) {
                    return { retValue, err == ThreadState::Ok ? ResultState::Ok : ResultState::Yielded };
                } else {
                    return { R {}, ResultState::TypeMismatch };
                }
            }
        } else {
            ResultState result;

            switch (err) {
            case ThreadState::RuntimeError:
                result = ResultState::RuntimeError;
                break;
            case ThreadState::MemError:
                result = ResultState::MemAllocError;
                break;
            default:
                result = ResultState::RuntimeError;
                break;
            }

            if constexpr (std::is_void_v<R>) {
                return { result };
            } else {
                return { R {}, result };
            }
        }
    }

    template <typename... T>
    void push(T&&... t) const
    {
        thread().push(t...);
    }

    auto close() const -> CoroutineState;

    auto current_state() const -> CoroutineState;

private:
    auto thread() const -> State;
};
}