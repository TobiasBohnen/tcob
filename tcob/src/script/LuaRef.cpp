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
    if (other._luaState.lua()) {
        other.push_self();
        ref(other._luaState, -1);
        if (_ref != LUA_NOREF)
            _luaState.pop(1); // pop extra copy
    }
}

auto LuaRef::operator=(const LuaRef& other) -> LuaRef&
{
    return *this = LuaRef(other);
}

LuaRef::LuaRef(LuaRef&& other) noexcept
    : _luaState { std::exchange(other._luaState, LuaState { nullptr }) }
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

void LuaRef::ref(const LuaState& ls, i32 idx)
{
    unref();
    _luaState = ls;

    if (_luaState.lua()) {
        _luaState.push_value(idx); // push copy of ref to top
        _ref = _luaState.ref(LUA_REGISTRYINDEX);
    }
}

void LuaRef::unref()
{
    if (is_valid() && _ownsRef) {
        _luaState.unref(LUA_REGISTRYINDEX, _ref);
        _ref = LUA_NOREF;
        _luaState = LuaState { nullptr };
    }
}

void LuaRef::push_self() const
{
    if (is_valid())
        _luaState.raw_get(LUA_REGISTRYINDEX, _ref);
}

auto LuaRef::lua_state() const -> const LuaState&
{
    return _luaState;
}

auto LuaRef::is_valid() const -> bool
{
    return _ref != LUA_NOREF && _luaState.lua() != nullptr;
}
}