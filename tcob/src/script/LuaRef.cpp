// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/script/LuaRef.hpp>

#include <cassert>
#include <lua.hpp>

#include <tcob/script/LuaState.hpp>

namespace tcob::lua {
Ref::Ref()
    : _ref { LUA_NOREF }
{
}

Ref::Ref(const Ref& other)
    : _ref { LUA_NOREF }
{
    if (other._luaState.lua()) {
        other.push_self();
        ref(other._luaState, -1);
        if (_ref != LUA_NOREF)
            _luaState.pop(1); // pop extra copy
    }
}

auto Ref::operator=(const Ref& other) -> Ref&
{
    return *this = Ref(other);
}

Ref::Ref(Ref&& other) noexcept
    : _luaState { std::exchange(other._luaState, State { nullptr }) }
    , _ref { std::exchange(other._ref, LUA_NOREF) }
    , _ownsRef { std::exchange(other._ownsRef, false) }
{
}

auto Ref::operator=(Ref&& other) noexcept -> Ref&
{
    std::swap(_luaState, other._luaState);
    std::swap(_ref, other._ref);
    std::swap(_ownsRef, other._ownsRef);
    return *this;
}

Ref::~Ref()
{
    unref();
}

void Ref::ref(const State& ls, i32 idx)
{
    unref();
    _luaState = ls;

    if (_luaState.lua()) {
        _luaState.push_value(idx); // push copy of ref to top
        _ref = _luaState.ref(LUA_REGISTRYINDEX);
    }
}

void Ref::unref()
{
    if (is_valid() && _ownsRef) {
        _luaState.unref(LUA_REGISTRYINDEX, _ref);
        _ref = LUA_NOREF;
        _luaState = State { nullptr };
    }
}

void Ref::push_self() const
{
    if (is_valid())
        _luaState.raw_get(LUA_REGISTRYINDEX, _ref);
}

auto Ref::state() const -> const State&
{
    return _luaState;
}

auto Ref::is_valid() const -> bool
{
    return _ref != LUA_NOREF && _luaState.lua() != nullptr;
}
}