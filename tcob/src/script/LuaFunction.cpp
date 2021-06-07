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
        auto ls { lua_state() };
        push_self();
        lua_dump(ls.lua(), &writer, &stream, true);
        ls.pop(1);
    }

    auto LuaFunctionBase::do_call(i32 nargs) const -> LuaResultState
    {
        auto ls { lua_state() };
        const i32 hpos { ls.get_top() - nargs };
        i32 err { 0 };

        ls.push_cfunction([](lua_State* l) -> i32 {
                const i32 n { lua_gettop(l)};
                Log("Lua says: " + std::string(lua_tostring(l, n)), LogLevel::Error);
                return 0; });
        ls.insert(hpos);
        err = lua_pcall(ls.lua(), nargs, LUA_MULTRET, hpos);
        ls.remove(hpos);

        switch (err) {
        case LUA_ERRRUN:
            return LuaResultState::RuntimeError;
        case LUA_ERRMEM:
            return LuaResultState::MemAllocError;
        case LUA_OK:
            return LuaResultState::Ok;
        default:
            return LuaResultState::RuntimeError;
        }
    }
}

////////////////////////////////////////////////////////////

auto LuaCoroutine::close() const -> LuaCoroutineState
{
    lua_State* t = thread();
    i32 err = lua_resetthread(t);
    if (err == LUA_OK) {
        return LuaCoroutineState::Ok;
    }

    return LuaCoroutineState::Error;
}

auto LuaCoroutine::state() const -> LuaCoroutineState
{
    lua_State* t = thread();

    const i32 err = lua_status(t);

    if (err == LUA_OK)
        return LuaCoroutineState::Ok;
    else if (err == LUA_YIELD)
        return LuaCoroutineState::Suspended;
    return LuaCoroutineState::Error;
}

auto LuaCoroutine::thread() const -> lua_State*
{
    auto ls { lua_state() };
    push_self();
    lua_State* thread = lua_tothread(ls.lua(), -1);
    ls.pop(1);
    return thread;
}
}