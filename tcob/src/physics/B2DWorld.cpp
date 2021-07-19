// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/physics/B2DWorld.hpp>

#include <box2d/box2d.h>

namespace tcob::box2d {

World::World()
    : _b2World { std::make_unique<b2World>(b2Vec2_zero) }
{
}

World::~World() = default;

}
