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
#include <optional>
#include <set>
#include <tuple>
#include <utility>

#include "tcob/core/Common.hpp"
#include "tcob/core/Serialization.hpp"
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

auto constexpr point_particle::settings::Members()
{
    return std::tuple {
        member<&point_particle::settings::Speed> {"speed"},
        member<&point_particle::settings::Direction> {"direction"},

        member<&point_particle::settings::LinearAcceleration> {"linear_acceleration"},
        member<&point_particle::settings::LinearDamping> {"linear_dampling"},
        member<&point_particle::settings::RadialAcceleration> {"radial_acceleration"},
        member<&point_particle::settings::TangentialAcceleration> {"tangential_acceleration"},

        member<&point_particle::settings::Gravity> {"gravity"},

        member<&point_particle::settings::TextureRegion> {"texture_region"},
        member<&point_particle::settings::Colors> {"colors"},
        member<&point_particle::settings::Transparency> {"transparency"},

        member<&point_particle::settings::Lifetime> {"lifetime"},
    };
}

auto constexpr quad_particle::settings::Members()
{
    return std::tuple {
        member<&quad_particle::settings::Speed> {"speed"},
        member<&quad_particle::settings::Direction> {"direction"},

        member<&quad_particle::settings::LinearAcceleration> {"linear_acceleration"},
        member<&quad_particle::settings::LinearDamping> {"linear_dampling"},
        member<&quad_particle::settings::RadialAcceleration> {"radial_acceleration"},
        member<&quad_particle::settings::TangentialAcceleration> {"tangential_acceleration"},

        member<&quad_particle::settings::Gravity> {"gravity"},

        member<&quad_particle::settings::TextureRegion> {"texture_region"},
        member<&quad_particle::settings::Colors> {"colors"},
        member<&quad_particle::settings::Transparency> {"transparency"},

        member<&quad_particle::settings::Lifetime> {"lifetime"},

        member<&quad_particle::settings::Scale> {"scale"},
        member<&quad_particle::settings::Size> {"size"},
        member<&quad_particle::settings::Spin> {"spin"},
        member<&quad_particle::settings::Rotation> {"rotation"},
    };
}

auto constexpr point_particle_emitter::settings::Members()
{
    return std::tuple {
        member<&point_particle_emitter::settings::Template> {"template"},
        member<&point_particle_emitter::settings::SpawnArea> {"spawn_area"},
        member<&point_particle_emitter::settings::SpawnRate> {"spawn_rate"},
        member<&point_particle_emitter::settings::IsExplosion> {"is_explosion"},
        member<&point_particle_emitter::settings::Lifetime, std::nullopt> {"lifetime"},
    };
}

auto constexpr quad_particle_emitter::settings::Members()
{
    return std::tuple {
        member<&quad_particle_emitter::settings::Template> {"template"},
        member<&quad_particle_emitter::settings::SpawnArea> {"spawn_area"},
        member<&quad_particle_emitter::settings::SpawnRate> {"spawn_rate"},
        member<&quad_particle_emitter::settings::IsExplosion> {"is_explosion"},
        member<&quad_particle_emitter::settings::Lifetime, std::nullopt> {"lifetime"},
    };
}

}
