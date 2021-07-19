// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

class b2World;

namespace tcob::box2d {

class World {
public:
    World();
    ~World();

private:
    std::unique_ptr<b2World> _b2World;
};
}