// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/script/LuaFunction.hpp>

#include <lua.hpp>

namespace tcob::lua {
namespace detail {
    static auto writer(lua_State*, const void* p, isize sz, void* ud) -> i32
    {
        auto* stream { reinterpret_cast<OutputFileStream*>(ud) };
        stream->write(reinterpret_cast<const byte*>(p), sz);
        return 0;
    }

    void FunctionBase::dump(OutputFileStream& stream) const
    {
        const auto& ls { state() };
        push_self();
        lua_dump(ls.lua(), &writer, &stream, true);
        ls.pop(1);
    }

    auto FunctionBase::do_call(i32 nargs) const -> ResultState
    {
        return state().do_call(nargs, LUA_MULTRET);
    }
}

////////////////////////////////////////////////////////////

auto Coroutine::close() const -> CoroutineState
{
    return thread().reset_thread() == LUA_OK ? CoroutineState::Ok : CoroutineState::Error;
}

auto Coroutine::current_state() const -> CoroutineState
{
    switch (thread().status()) {
    case LUA_OK:
        return CoroutineState::Ok;
    case LUA_YIELD:
        return CoroutineState::Suspended;
    default:
        return CoroutineState::Error;
    }
}

auto Coroutine::thread() const -> State
{
    const auto& ls { state() };
    push_self();
    const auto thread { ls.to_thread(-1) };
    ls.pop(1);
    return thread;
}
}
