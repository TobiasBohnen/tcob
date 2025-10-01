// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "LightingSystem.hpp"

#include <memory>

namespace tcob::gfx {

template <typename T>
inline auto lighting_system::create_light_source(auto&&... args) -> T&
{
    _isDirty = true;
    return *_lightSources.emplace_back(std::unique_ptr<T>(new T {this, args...}));
}

template <typename T>
inline auto lighting_system::create_shadow_caster(auto&&... args) -> T&
{
    _isDirty = true;
    return *_shadowCasters.emplace_back(std::unique_ptr<T>(new T {this, args...}));
}

}
