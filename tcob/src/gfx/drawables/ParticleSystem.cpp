// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/ParticleSystem.hpp"

#include <algorithm>
#include <utility>

namespace tcob::gfx {
using namespace std::chrono_literals;

particle_system::particle_system()
{
    Material.Changed.connect([&](auto const& value) { _renderer.set_material(value); });
}

void particle_system::start()
{
    if (_isRunning) {
        return;
    }

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

auto particle_system::get_particle_count() const -> isize
{
    return _aliveParticleCount;
}

auto particle_system::activate_particle() -> particle&
{
    if (_aliveParticleCount == std::ssize(_particles)) {
        _aliveParticleCount++;
        auto& particle {_particles.emplace_back()};
        ParticleSpawn(particle);
        return particle;
    }

    auto& particle {_particles[_aliveParticleCount++]};
    ParticleSpawn(particle);
    return particle;
}

void particle_system::deactivate_particle(particle& particle)
{
    ParticleDeath(particle);
    std::swap(particle, _particles[--_aliveParticleCount]);
}

void particle_system::remove_all_emitters()
{
    _emitters.clear();
    stop();
}

void particle_system::on_update(milliseconds deltaTime)
{
    if (!_isRunning || !Material()) {
        return;
    }

    /*     Material->update(deltaTime); */

    for (auto& emitter : _emitters) {
        if (emitter->is_alive()) {
            emitter->emit_particles(*this, deltaTime);
        }
    }

    for (i32 i {0}; i < _aliveParticleCount; ++i) {
        auto& particle {_particles[i]};
        if (particle.is_alive()) {
            // call affectors on particle
            ParticleUpdate(particle);

            // update
            particle.update(deltaTime);

        } else {
            deactivate_particle(particle);
            i--;
        }
    }
}

auto particle_system::can_draw() const -> bool
{
    return _isRunning && _aliveParticleCount != 0 && !Material().is_expired();
}

void particle_system::on_draw_to(render_target& target)
{
    usize count {0};

    std::vector<quad> quads;
    quads.reserve(_aliveParticleCount);
    quad* quad {quads.data()};

    for (i32 i {0}; i < _aliveParticleCount; ++i) {
        auto& particle {_particles[i]};
        if (particle.Visible) {
            particle.to_quad(quad);
            ++quad;
            ++count;
        }
    }
    _renderer.set_geometry({quads.data(), count});

    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////

particle_emitter::particle_emitter() = default;

particle_emitter::~particle_emitter() = default;

auto particle_emitter::is_alive() const -> bool
{
    return _remainLife > 0ms || IsLooping;
}

void particle_emitter::reset()
{
    _remainLife = Lifetime;
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
        particle.Color        = color {255, 255, 255, alpha};

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
