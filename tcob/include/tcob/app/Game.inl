// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Game.hpp"

namespace tcob {

template <std::derived_from<scene> T>
inline void game::push_scene()
{
    push_scene(std::make_shared<T>(*this));
}

}
