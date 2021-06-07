// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/script/LuaScript.hpp>
#include <tcob/script/LuaTable.hpp>

namespace tcob {
class Config final : public LuaTable {
public:
    Config() = default;
    ~Config() override;
    auto operator=(const LuaRef& other) -> Config&;

    void save() const;
    auto load() -> bool;

private:
    LuaScript _script;
};
}