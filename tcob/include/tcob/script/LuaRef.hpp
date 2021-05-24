// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <functional>

struct lua_State;
namespace tcob {
class LuaRef {
public:
    LuaRef();
    virtual ~LuaRef();

    LuaRef(const LuaRef& other);
    auto operator=(const LuaRef& other) -> LuaRef&;

    LuaRef(LuaRef&& other) noexcept;
    auto operator=(LuaRef&& other) noexcept -> LuaRef&;

    void ref(lua_State* l, i32 idx);
    void unref();

    void push_self() const;

    auto is_valid() const -> bool;

protected:
    auto lua_state() const -> LuaState;

private:
    lua_State* _luaState { nullptr };
    i32 _ref;
    bool _ownsRef { true };
};
}