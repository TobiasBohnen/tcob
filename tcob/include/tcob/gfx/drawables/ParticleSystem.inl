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

inline void particle_template::Serialize(particle_template const& v, auto&& s)
{
    s["acceleration"] = v.Acceleration;
    s["direction"]    = v.Direction;
    s["lifetime"]     = v.Lifetime;
    s["scale"]        = v.Scale;
    s["size"]         = v.Size;
    s["speed"]        = v.Speed;
    s["spin"]         = v.Spin;
    s["texture"]      = v.Texture;
    s["color"]        = v.Color;
    s["transparency"] = v.Transparency;
}

inline auto particle_template::Deserialize(particle_template& v, auto&& s) -> bool
{
    return s.try_get(v.Acceleration, "acceleration")
        && s.try_get(v.Direction, "direction")
        && s.try_get(v.Lifetime, "lifetime")
        && s.try_get(v.Scale, "scale")
        && s.try_get(v.Size, "size")
        && s.try_get(v.Speed, "speed")
        && s.try_get(v.Spin, "spin")
        && s.try_get(v.Texture, "texture")
        && s.try_get(v.Color, "color")
        && s.try_get(v.Transparency, "transparency");
}

inline void particle_emitter::Serialize(particle_emitter const& v, auto&& s)
{
    s["template"]   = v.Template;
    s["spawn_area"] = v.SpawnArea;
    s["spawn_rate"] = v.SpawnRate;
    if (v.Lifetime) {
        s["lifetime"] = *v.Lifetime;
    }
}

inline auto particle_emitter::Deserialize(particle_emitter& v, auto&& s) -> bool
{
    if (s.has("lifetime")) {
        v.Lifetime = s["lifetime"].template as<milliseconds>();
    }
    return s.try_get(v.Template, "template")
        && s.try_get(v.SpawnArea, "spawn_area")
        && s.try_get(v.SpawnRate, "spawn_rate");
}

}
