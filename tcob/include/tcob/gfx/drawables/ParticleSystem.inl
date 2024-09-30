// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ParticleSystem.hpp"

namespace tcob::gfx {

template <typename T>
inline auto particle_system::create_emitter(auto&&... args) -> std::shared_ptr<T>
{
    auto retValue {std::make_shared<T>(args...)};
    _emitters.push_back(retValue);
    return retValue;
}

template <typename T>
inline void particle_system::add_emitter(std::shared_ptr<T> const& emitter)
{
    _emitters.push_back(emitter);
}

}
