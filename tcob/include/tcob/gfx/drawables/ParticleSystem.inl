// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ParticleSystem.hpp"

#include <algorithm>

#include "tcob/core/Common.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////

template <typename Emitter>
inline particle_system<Emitter>::particle_system(bool multiThreaded)
    : _multiThreaded {multiThreaded}
{
    Material.Changed.connect([&](auto const& value) { _renderer.set_material(value.ptr()); });
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
    for (auto& emitter : _emitters) {
        emitter->reset();
    }

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
    helper::erase(_emitters, [&emitter](auto const& val) { return val.get() == &emitter; });
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
    if (!_isRunning || !Material()) { return; }

    for (auto& emitter : _emitters) {
        emitter->emit(*this, deltaTime);
    }

    std::set<isize, std::greater<>> toBeDeactivated;

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                auto& particle {_particles[i]};
                if (particle.RemainingLife.count() > 0) {
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
    return _isRunning && _aliveParticleCount != 0 && !Material().is_expired();
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

}
