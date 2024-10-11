// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "LightingSystem.hpp"

namespace tcob::gfx {

template <typename T>
inline auto lighting_system::create_light_source(auto&&... args) -> std::shared_ptr<T>
{
    auto retValue {std::shared_ptr<T>(new T {this, args...})};
    _lightSources.push_back(retValue);
    _isDirty = true;
    return retValue;
}

template <typename T>
inline auto lighting_system::create_shadow_caster(auto&&... args) -> std::shared_ptr<T>
{
    auto retValue {std::shared_ptr<T>(new T {this, args...})};
    _shadowCasters.push_back(retValue);
    _isDirty = true;
    return retValue;
}

}
