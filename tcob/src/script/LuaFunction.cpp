// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/script/LuaFunction.hpp>

#include <lua.hpp>

namespace tcob {
namespace detail {
    static auto writer(lua_State*, const void* p, isize sz, void* ud) -> i32
    {
        auto* stream { reinterpret_cast<OutputFileStream*>(ud) };
        stream->write(reinterpret_cast<const byte*>(p), sz);
        return 0;
    }

    void LuaFunctionBase::dump(OutputFileStream& stream) const
    {
        const auto& ls { state() };
        push_self();
        lua_dump(ls.lua(), &writer, &stream, true);
        ls.pop(1);
    }

    auto LuaFunctionBase::do_call(i32 nargs) const -> LuaResultState
    {
        return state().do_call(nargs, LUA_MULTRET);
    }
}

////////////////////////////////////////////////////////////

auto LuaCoroutine::close() const -> LuaCoroutineState
{
    if (thread().reset_thread() == LUA_OK) {
        return LuaCoroutineState::Ok;
    }

    return LuaCoroutineState::Error;
}

auto LuaCoroutine::current_state() const -> LuaCoroutineState
{
    const i32 err { thread().status() };
    if (err == LUA_OK)
        return LuaCoroutineState::Ok;
    else if (err == LUA_YIELD)
        return LuaCoroutineState::Suspended;
    return LuaCoroutineState::Error;
}

auto LuaCoroutine::thread() const -> LuaState
{
    const auto& ls { state() };
    push_self();
    const auto thread { ls.to_thread(-1) };
    ls.pop(1);
    return thread;
}
}