// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/script/LuaRef.hpp>

#include <cassert>

#include <lua.hpp>

#include <tcob/script/LuaState.hpp>

namespace tcob {
LuaRef::LuaRef()
    : _ref { LUA_NOREF }
{
}

LuaRef::LuaRef(const LuaRef& other)
    : _ref { LUA_NOREF }
{
    if (other._luaState) {
        other.push_self();
        ref(other._luaState, -1);
        if (_ref != LUA_NOREF)
            lua_pop(_luaState, 1); // pop extra copy
    }
}

auto LuaRef::operator=(const LuaRef& other) -> LuaRef&
{
    return *this = LuaRef(other);
}

LuaRef::LuaRef(LuaRef&& other) noexcept
    : _luaState { std::exchange(other._luaState, nullptr) }
    , _ref { std::exchange(other._ref, LUA_NOREF) }
    , _ownsRef { std::exchange(other._ownsRef, false) }
{
}

auto LuaRef::operator=(LuaRef&& other) noexcept -> LuaRef&
{
    std::swap(_luaState, other._luaState);
    std::swap(_ref, other._ref);
    std::swap(_ownsRef, other._ownsRef);
    return *this;
}

LuaRef::~LuaRef()
{
    unref();
}

void LuaRef::ref(lua_State* l, i32 idx)
{
    unref();
    _luaState = l;
    if (l) {
        lua_pushvalue(l, idx); // push copy of ref to top
        _ref = luaL_ref(l, LUA_REGISTRYINDEX);
    }
}

void LuaRef::unref()
{
    if (_luaState && _ownsRef && _ref != LUA_NOREF) {
        luaL_unref(_luaState, LUA_REGISTRYINDEX, _ref);
        _ref = LUA_NOREF;
        _luaState = nullptr;
    }
}

void LuaRef::push_self() const
{
    if (_luaState)
        lua_rawgeti(_luaState, LUA_REGISTRYINDEX, _ref);
}

auto LuaRef::lua_state() const -> LuaState
{
    assert(_luaState);
    return LuaState { _luaState };
}

auto LuaRef::is_valid() const -> bool
{
    return _ref != LUA_NOREF && _luaState != nullptr;
}
}