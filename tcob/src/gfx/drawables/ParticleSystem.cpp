// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/ParticleSystem.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"

#include <algorithm>
#include <utility>

namespace tcob::gfx {
using namespace std::chrono_literals;

particle_system::particle_system(bool multiThreaded)
    : _multiThreaded {multiThreaded}
{
    Material.Changed.connect([&](auto const& value) { _renderer.set_material(value); });
}

void particle_system::start()
{
    if (_isRunning) { return; }

    _isRunning = true;
    for (auto& emitter : _emitters) {
        emitter->reset();
    }

    _particles.reserve(100);
    _particles.clear();
    _aliveParticleCount = 0;
}

void particle_system::restart()
{
    stop();
    start();
}

void particle_system::stop()
{
    _isRunning = false;

    _renderer.reset_geometry();
    _particles.clear();
    _aliveParticleCount = 0;
}

void particle_system::remove_emitter(particle_emitter const& emitter)
{
    _emitters.erase(std::find_if(_emitters.begin(), _emitters.end(), [&emitter](auto const& val) {
        return val.get() == &emitter;
    }));
}

void particle_system::clear_emitters()
{
    _emitters.clear();
    stop();
}

auto particle_system::get_particle_count() const -> i32
{
    return _aliveParticleCount;
}

auto particle_system::activate_particle() -> particle&
{
    if (_aliveParticleCount == std::ssize(_particles)) {
        _aliveParticleCount++;
        return _particles.emplace_back();
    }

    return _particles[_aliveParticleCount++];
}

void particle_system::deactivate_particle(particle& particle)
{
    assert(_aliveParticleCount > 0);
    std::swap(particle, _particles[--_aliveParticleCount]);
}

void particle_system::on_update(milliseconds deltaTime)
{
    if (!_isRunning || !Material()) { return; }

    for (auto& emitter : _emitters) {
        if (emitter->is_alive()) {
            emitter->emit_particles(*this, deltaTime);
        }
    }

    std::set<i32, std::greater<>> toBeDeactivated;

    locate_service<task_manager>().run_parallel(
        [&](task_context const& ctx) {
            for (i32 i {ctx.Start}; i < ctx.End; ++i) {
                auto& particle {_particles[i]};
                if (particle.is_alive()) {
                    ParticleUpdate({particle, deltaTime});
                    particle.update(deltaTime);
                } else {
                    std::scoped_lock lock {_mutex};
                    toBeDeactivated.insert(i);
                }
            }
        },
        _aliveParticleCount, _multiThreaded ? 1 : _aliveParticleCount);

    for (auto const& i : toBeDeactivated) {
        deactivate_particle(_particles[i]);
    }
}

auto particle_system::can_draw() const -> bool
{
    return _isRunning && _aliveParticleCount != 0 && !Material().is_expired();
}

void particle_system::on_draw_to(render_target& target)
{
    std::vector<quad> quads;
    quads.reserve(_aliveParticleCount);
    std::atomic<usize> count {0};

    locate_service<task_manager>().run_parallel(
        [&](task_context const& ctx) {
            for (i32 i {ctx.Start}; i < ctx.End; ++i) {
                auto& particle {_particles[i]};
                if (particle.Visible) {
                    particle.to_quad(&quads[i]);
                    ++count;
                }
            }
        },
        _aliveParticleCount, _multiThreaded ? 1 : _aliveParticleCount);

    _renderer.set_geometry({quads.data(), count});

    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////

particle_emitter::particle_emitter() = default;

particle_emitter::~particle_emitter() = default;

auto particle_emitter::is_alive() const -> bool
{
    return !Lifetime || _remainLife > 0ms;
}

void particle_emitter::reset()
{
    if (Lifetime) {
        _remainLife = *Lifetime;
    }
}

void particle_emitter::emit_particles(particle_system& system, milliseconds time)
{
    _remainLife -= time;

    f64 const particleAmount {SpawnRate * (time.count() / 1000) + _emissionDiff};
    i32 const particleCount {static_cast<i32>(particleAmount)};
    _emissionDiff = particleAmount - particleCount;

    auto const& texRegion {system.Material->Texture->get_region(Template.Texture)};
    auto const& sysPos {system.Position};

    for (i32 i {0}; i < particleCount; ++i) {
        auto& particle {system.activate_particle()};

        particle.set_texture_region(texRegion);

        u8 const alpha {static_cast<u8>(255 - static_cast<u8>(255 * std::clamp(_randomGen(Template.Transparency.first, Template.Transparency.second), 0.0f, 1.0f)))};

        particle.Direction    = _randomGen(Template.Direction.first.Value, Template.Direction.second.Value);
        particle.Speed        = _randomGen(Template.Speed.first, Template.Speed.second);
        particle.Spin         = _randomGen(Template.Spin.first.Value, Template.Spin.second.Value);
        particle.Acceleration = _randomGen(Template.Acceleration.first, Template.Acceleration.second);
        particle.UserData     = 0;
        particle.Color        = color {Template.Color.R, Template.Color.G, Template.Color.B, static_cast<u8>((Template.Color.A + alpha) / 2)};

        // set scale
        f32 const scaleF {_randomGen(Template.Scale.first, Template.Scale.second)};
        particle.Scale = {scaleF, scaleF};

        // set life
        particle.set_lifetime(milliseconds {_randomGen(Template.Lifetime.first.count(), Template.Lifetime.second.count())});

        // calculate random postion
        f32 const x {_randomGen(SpawnArea.left(), SpawnArea.right()) - Template.Size.Width / 2};
        f32 const y {_randomGen(SpawnArea.top(), SpawnArea.bottom()) - Template.Size.Height / 2};

        // set bounds
        particle.Bounds = {{x + sysPos.X, y + sysPos.Y}, Template.Size};

        particle.Visible = true;
    }
}

////////////////////////////////////////////////////////////

auto particle::is_alive() const -> bool
{
    return _remainingLife > 0ms;
}

void particle::update(milliseconds delta)
{
    f64 const seconds {delta.count() / 1000};

    // age
    _remainingLife -= delta;

    // move
    Speed += Acceleration * seconds;
    f64 const      var {Speed * seconds};
    degree_f const angle {Direction.as_normalized() - degree_f {90}};
    point_f const  direction {static_cast<f32>(angle.cos()), static_cast<f32>(angle.sin())};
    point_f const  offset {static_cast<f32>(direction.X * var), static_cast<f32>(direction.Y * var)};
    Bounds = {Bounds.get_position() + offset, Bounds.get_size()};

    // spin
    Rotation += degree_f {Spin.Value * static_cast<f32>(seconds)};

    point_f const origin {Bounds.get_center()};
    _transform.to_identity();

    if (Scale != size_f::One) {
        _transform.scale_at(Scale, origin);
    }
    if (Rotation != degree_f {0}) {
        _transform.rotate_at(Rotation, origin);
    }
}

void particle::set_lifetime(milliseconds life)
{
    _startingLife  = life;
    _remainingLife = life;
}

auto particle::get_lifetime_ratio() const -> f32
{
    return static_cast<f32>(_remainingLife / _startingLife);
}

void particle::set_texture_region(texture_region const& texRegion)
{
    _region = texRegion;
}

void particle::to_quad(quad* quad)
{
    geometry::set_position(*quad, Bounds, _transform);
    geometry::set_color(*quad, Color);
    geometry::set_texcoords(*quad, _region);
}

}
