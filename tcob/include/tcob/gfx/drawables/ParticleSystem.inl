// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ParticleSystem.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <tuple>

#include "tcob/core/Common.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////

template <typename Emitter>
particle_system<Emitter>::particle_system(bool multiThreaded, isize reservedParticleCount)
    : _multiThreaded {multiThreaded}
{
    Material.Changed.connect([this](auto const& value) { _renderer.set_material(value.ptr()); });
    if (reservedParticleCount > 0) {
        _particles.reserve(reservedParticleCount);
    }
}

template <typename Emitter>
inline auto particle_system<Emitter>::is_running() const -> bool
{
    return _isRunning;
}

template <typename Emitter>
inline void particle_system<Emitter>::start()
{
    if (_isRunning) { return; }

    _isRunning = true;
    for (auto& emitter : _emitters) { emitter->reset(); }

    _particles.clear();
    _aliveParticleCount = 0;
}

template <typename Emitter>
inline void particle_system<Emitter>::restart()
{
    stop();
    start();
}

template <typename Emitter>
inline void particle_system<Emitter>::stop()
{
    _isRunning = false;

    _renderer.reset_geometry();
    _particles.clear();
    _aliveParticleCount = 0;
    _geometry.clear();
}

template <typename Emitter>
inline void particle_system<Emitter>::remove_emitter(emitter_type const& emitter)
{
    helper::erase_first(_emitters, [&emitter](auto const& val) { return val.get() == &emitter; });
}

template <typename Emitter>
inline void particle_system<Emitter>::clear_emitters()
{
    _emitters.clear();
    stop();
}

template <typename Emitter>
inline auto particle_system<Emitter>::particle_count() const -> isize
{
    return _aliveParticleCount;
}

template <typename Emitter>
inline auto particle_system<Emitter>::activate_particle() -> particle_type&
{
    if (_aliveParticleCount == std::ssize(_particles)) {
        _aliveParticleCount++;
        return _particles.emplace_back();
    }

    return _particles[_aliveParticleCount++];
}

template <typename Emitter>
inline void particle_system<Emitter>::deactivate_particle(particle_type& particle)
{
    assert(_aliveParticleCount > 0);
    std::swap(particle, _particles[--_aliveParticleCount]);
}

template <typename Emitter>
inline auto particle_system<Emitter>::create_emitter(auto&&... args) -> std::shared_ptr<Emitter>
{
    auto retValue {std::make_shared<Emitter>(args...)};
    _emitters.push_back(retValue);
    return retValue;
}

template <typename Emitter>
inline void particle_system<Emitter>::add_emitter(std::shared_ptr<Emitter> const& emitter)
{
    _emitters.push_back(emitter);
}

template <typename Emitter>
inline void particle_system<Emitter>::on_update(milliseconds deltaTime)
{
    if (!_isRunning || !Material) { return; }

    for (auto& emitter : _emitters) {
        emitter->emit(*this, deltaTime);
    }

    std::set<isize, std::greater<>> toBeDeactivated;

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                auto& particle {_particles[i]};
                if (particle.is_alive()) {
                    ParticleUpdate({.Particle = particle, .DeltaTime = deltaTime});
                    particle.update(deltaTime);
                } else {
                    std::scoped_lock lock {_mutex};
                    toBeDeactivated.insert(i);
                }
            }
        },
        _aliveParticleCount, _multiThreaded ? 64 : _aliveParticleCount);

    for (auto const& i : toBeDeactivated) {
        deactivate_particle(_particles[i]);
    }
}

template <typename Emitter>
inline auto particle_system<Emitter>::can_draw() const -> bool
{
    return _isRunning && _aliveParticleCount != 0 && !(*Material).is_expired();
}

template <typename Emitter>
inline void particle_system<Emitter>::on_draw_to(render_target& target)
{
    _geometry.resize(_aliveParticleCount);

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                _particles[i].convert_to(&_geometry[i]);
            }
        },
        _aliveParticleCount, _multiThreaded ? 64 : _aliveParticleCount);

    _renderer.set_geometry({_geometry.data(), static_cast<usize>(_aliveParticleCount)});

    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

auto constexpr point_particle::settings::members(this auto&& self)
{
    return std::tuple {
        nm("speed", self.Speed),
        nm("direction", self.Direction),

        nm("linear_acceleration", self.LinearAcceleration),
        nm("linear_dampling", self.LinearDamping),
        nm("radial_acceleration", self.RadialAcceleration),
        nm("tangential_acceleration", self.TangentialAcceleration),

        nm("gravity", self.Gravity),

        nm("texture_region", self.TextureRegion),
        nm("colors", self.Colors),
        nm("transparency", self.Transparency),

        nm("lifetime", self.Lifetime),
    };
}

auto constexpr quad_particle::settings::members(this auto&& self)
{
    return std::tuple {
        nm("speed", self.Speed),
        nm("direction", self.Direction),

        nm("linear_acceleration", self.LinearAcceleration),
        nm("linear_dampling", self.LinearDamping),
        nm("radial_acceleration", self.RadialAcceleration),
        nm("tangential_acceleration", self.TangentialAcceleration),

        nm("gravity", self.Gravity),

        nm("texture_region", self.TextureRegion),
        nm("colors", self.Colors),
        nm("transparency", self.Transparency),

        nm("lifetime", self.Lifetime),

        nm("scale", self.Scale),
        nm("size", self.Size),
        nm("spin", self.Spin),
        nm("rotation", self.Rotation),
    };
}

inline void point_particle_emitter::settings::Serialize(point_particle_emitter::settings const& v, auto&& s)
{
    s["template"]     = v.Template;
    s["spawn_area"]   = v.SpawnArea;
    s["spawn_rate"]   = v.SpawnRate;
    s["is_explosion"] = v.IsExplosion;
    if (v.Lifetime) { s["lifetime"] = *v.Lifetime; }
}

inline void quad_particle_emitter::settings::Serialize(quad_particle_emitter::settings const& v, auto&& s)
{
    s["template"]     = v.Template;
    s["spawn_area"]   = v.SpawnArea;
    s["spawn_rate"]   = v.SpawnRate;
    s["is_explosion"] = v.IsExplosion;
    if (v.Lifetime) { s["lifetime"] = *v.Lifetime; }
}

inline auto point_particle_emitter::settings::Deserialize(point_particle_emitter::settings& v, auto&& s) -> bool
{
    if (s.has("lifetime")) { v.Lifetime = s["lifetime"].template as<milliseconds>(); }
    return s.try_get(v.Template, "template")
        && s.try_get(v.SpawnArea, "spawn_area")
        && s.try_get(v.SpawnRate, "spawn_rate");
}

inline auto quad_particle_emitter::settings::Deserialize(quad_particle_emitter::settings& v, auto&& s) -> bool
{
    if (s.has("lifetime")) { v.Lifetime = s["lifetime"].template as<milliseconds>(); }
    return s.try_get(v.Template, "template")
        && s.try_get(v.SpawnArea, "spawn_area")
        && s.try_get(v.SpawnRate, "spawn_rate");
}

}
