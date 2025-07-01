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

inline void point_particle::settings::Serialize(point_particle::settings const& v, auto&& s)
{
    s["speed"]     = v.Speed;
    s["direction"] = v.Direction;

    s["linear_acceleration"]     = v.LinearAcceleration;
    s["linear_dampling"]         = v.LinearDamping;
    s["radial_acceleration"]     = v.RadialAcceleration;
    s["tangential_acceleration"] = v.TangentialAcceleration;

    s["gravity"] = v.Gravity;

    s["texture_region"] = v.TextureRegion;
    s["colors"]         = v.Colors;
    s["transparency"]   = v.Transparency;

    s["lifetime"] = v.Lifetime;
}

inline void quad_particle::settings::Serialize(quad_particle::settings const& v, auto&& s)
{
    s["speed"]     = v.Speed;
    s["direction"] = v.Direction;

    s["linear_acceleration"]     = v.LinearAcceleration;
    s["linear_dampling"]         = v.LinearDamping;
    s["radial_acceleration"]     = v.RadialAcceleration;
    s["tangential_acceleration"] = v.TangentialAcceleration;

    s["gravity"] = v.Gravity;

    s["texture_region"] = v.TextureRegion;
    s["colors"]         = v.Colors;
    s["transparency"]   = v.Transparency;

    s["lifetime"] = v.Lifetime;

    s["scale"]    = v.Scale;
    s["size"]     = v.Size;
    s["spin"]     = v.Spin;
    s["rotation"] = v.Rotation;
}

inline auto point_particle::settings::Deserialize(point_particle::settings& v, auto&& s) -> bool
{
    return s.try_get(v.Speed, "speed")
        && s.try_get(v.Direction, "direction")

        && s.try_get(v.LinearAcceleration, "linear_acceleration")
        && s.try_get(v.LinearDamping, "linear_dampling")
        && s.try_get(v.RadialAcceleration, "radial_acceleration")
        && s.try_get(v.TangentialAcceleration, "tangential_acceleration")

        && s.try_get(v.Gravity, "gravity")

        && s.try_get(v.TextureRegion, "texture_region")
        && s.try_get(v.Colors, "colors")
        && s.try_get(v.Transparency, "transparency")

        && s.try_get(v.Lifetime, "lifetime");
}

inline auto quad_particle::settings::Deserialize(quad_particle::settings& v, auto&& s) -> bool
{
    return s.try_get(v.Speed, "speed")
        && s.try_get(v.Direction, "direction")

        && s.try_get(v.LinearAcceleration, "linear_acceleration")
        && s.try_get(v.LinearDamping, "linear_dampling")
        && s.try_get(v.RadialAcceleration, "radial_acceleration")
        && s.try_get(v.TangentialAcceleration, "tangential_acceleration")

        && s.try_get(v.Gravity, "gravity")

        && s.try_get(v.TextureRegion, "texture_region")
        && s.try_get(v.Colors, "colors")
        && s.try_get(v.Transparency, "transparency")

        && s.try_get(v.Lifetime, "lifetime")

        && s.try_get(v.Scale, "scale")
        && s.try_get(v.Size, "size")
        && s.try_get(v.Spin, "spin")
        && s.try_get(v.Rotation, "rotation");
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
