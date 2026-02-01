// Copyright (c) 2026 Tobias Bohnen
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

template <typename Particle>
particle_system<Particle>::particle_system(bool multiThreaded, isize reservedParticleCount)
    : _multiThreaded {multiThreaded}
{
    if (reservedParticleCount > 0) {
        _particles.reserve(reservedParticleCount);
    }
}

template <typename Particle>
inline auto particle_system<Particle>::is_running() const -> bool
{
    return _isRunning;
}

template <typename Particle>
inline void particle_system<Particle>::start()
{
    if (_isRunning) { return; }

    _isRunning = true;
    for (auto& emitter : _emitters) { emitter->reset(); }

    _particles.clear();
    _aliveParticleCount = 0;
}

template <typename Particle>
inline void particle_system<Particle>::restart()
{
    stop();
    start();
}

template <typename Particle>
inline void particle_system<Particle>::stop()
{
    _isRunning = false;

    _renderer.reset_geometry();
    _particles.clear();
    _aliveParticleCount = 0;
    _geometry.clear();
}

template <typename Particle>
inline auto particle_system<Particle>::create_emitter() -> particle_emitter<particle_type>&
{
    return *_emitters.emplace_back(std::make_unique<particle_emitter<particle_type>>());
}

template <typename Particle>
inline auto particle_system<Particle>::remove_emitter(particle_emitter<particle_type> const& emitter) -> bool
{
    return helper::erase_first(_emitters, [&emitter](auto const& val) { return val.get() == &emitter; });
}

template <typename Particle>
inline void particle_system<Particle>::clear()
{
    _emitters.clear();
    stop();
}

template <typename Particle>
inline auto particle_system<Particle>::particle_count() const -> isize
{
    return _aliveParticleCount;
}

template <typename Particle>
inline auto particle_system<Particle>::activate_particle() -> particle_type&
{
    if (_aliveParticleCount == std::ssize(_particles)) {
        _aliveParticleCount++;
        return _particles.emplace_back();
    }

    return _particles[_aliveParticleCount++];
}

template <typename Particle>
inline void particle_system<Particle>::deactivate_particle(particle_type& particle)
{
    assert(_aliveParticleCount > 0);
    std::swap(particle, _particles[--_aliveParticleCount]);
}

template <typename Particle>
inline void particle_system<Particle>::on_update(milliseconds deltaTime)
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

template <typename Particle>
inline auto particle_system<Particle>::can_draw() const -> bool
{
    return _isRunning && _aliveParticleCount != 0 && !(*Material).is_expired();
}

template <typename Particle>
inline void particle_system<Particle>::on_draw_to(render_target& target)
{
    _geometry.resize(_aliveParticleCount);

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                _particles[i].convert_to(&_geometry[i]);
            }
        },
        _aliveParticleCount, _multiThreaded ? 64 : _aliveParticleCount);

    for (isize i {0}; i < Material->pass_count(); ++i) {
        auto const& pass {Material->get_pass(i)};

        _renderer.set_geometry({_geometry.data(), static_cast<usize>(_aliveParticleCount)}, &pass);
        _renderer.render_to_target(target);
    }
}

////////////////////////////////////////////////////////

template <typename Particle>
inline auto particle_emitter<Particle>::is_alive() const -> bool
{
    return _alive && (!Settings.Lifetime || _remainingLife > milliseconds {0});
}

template <typename Particle>
inline void particle_emitter<Particle>::reset()
{
    if (Settings.Lifetime) { _remainingLife = *Settings.Lifetime; }
    _alive = true;
}

template <typename Particle>
template <typename ParticleSystem>
inline void particle_emitter<Particle>::emit(ParticleSystem& system, milliseconds deltaTime)
{
    if (!is_alive()) { return; }

    _remainingLife -= deltaTime;

    i32 particleCount {0};
    if (Settings.IsExplosion) {
        particleCount = static_cast<i32>(Settings.SpawnRate);
        _alive        = false;
    } else {
        f64 const particleAmount {(Settings.SpawnRate * (deltaTime.count() / 1000)) + _emissionDiff};
        particleCount = static_cast<i32>(particleAmount);
        _emissionDiff = particleAmount - particleCount;
    }

    auto const& tmpl {Settings.Template};
    auto const& texRegion {system.Material->first_pass().Texture->regions()[tmpl.TextureRegion]}; // TODO texRegion pass

    for (i32 i {0}; i < particleCount; ++i) {
        auto& particle {system.activate_particle()};
        particle.init(tmpl, texRegion, Settings.SpawnArea, _rng);
    }
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

template <typename Particle>
auto constexpr particle_emitter<Particle>::settings::Members()
{
    return std::tuple {
        member<&particle_emitter::settings::Template> {"template"},
        member<&particle_emitter::settings::SpawnArea> {"spawn_area"},
        member<&particle_emitter::settings::SpawnRate> {"spawn_rate"},
        member<&particle_emitter::settings::IsExplosion> {"is_explosion"},
        member<&particle_emitter::settings::Lifetime, std::nullopt> {"lifetime"},
    };
}

}
