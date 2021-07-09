// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <functional>

#include <tcob/script/LuaState.hpp>

namespace tcob::lua {
class Ref {
public:
    Ref();
    virtual ~Ref();

    Ref(const Ref& other);
    auto operator=(const Ref& other) -> Ref&;

    Ref(Ref&& other) noexcept;
    auto operator=(Ref&& other) noexcept -> Ref&;

    void ref(const State& state, i32 idx);
    void unref();

    void push_self() const;

    auto is_valid() const -> bool;

protected:
    auto state() const -> const State&;

private:
    State _luaState { nullptr };
    i32 _ref;
    bool _ownsRef { true };
};
}