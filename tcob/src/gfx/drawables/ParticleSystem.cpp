// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/ParticleSystem.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <span>
#include <utility>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Renderer.hpp"

namespace tcob::gfx {

using namespace std::chrono_literals;

particle_system::particle_system(bool multiThreaded, isize reservedParticleCount)
    : _multiThreaded {multiThreaded}
{
    if (reservedParticleCount > 0) {
        _particles.reserve(reservedParticleCount);
    }
}

auto particle_system::is_running() const -> bool
{
    return _isRunning;
}

void particle_system::start()
{
    if (_isRunning) { return; }

    _isRunning = true;
    for (auto& emitter : _emitters) { emitter->reset(); }

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
    _geometry.clear();
}

auto particle_system::create_emitter() -> particle_emitter&
{
    return *_emitters.emplace_back(std::make_unique<particle_emitter>());
}

auto particle_system::remove_emitter(particle_emitter const& emitter) -> bool
{
    return helper::erase_first(_emitters, [&emitter](auto const& val) { return val.get() == &emitter; });
}

void particle_system::clear()
{
    _emitters.clear();
    stop();
}

auto particle_system::particle_count() const -> isize
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

auto particle_system::can_draw() const -> bool
{
    return _isRunning && _aliveParticleCount != 0 && !(*Material).is_expired();
}

void particle_system::on_draw_to(render_target& target, transform const& xform)
{
    _geometry.resize(_aliveParticleCount);

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                _particles[i].convert_to(&_geometry[i]);
            }
        },
        _aliveParticleCount,
        _multiThreaded ? 64 : _aliveParticleCount);

    for (isize i {0}; i < Material->pass_count(); ++i) {
        auto const& pass {Material->get_pass(i)};
        _renderer.set_geometry({.Vertices = geometry::flatten({_geometry.data(), static_cast<usize>(_aliveParticleCount)}),
                                .Indices  = geometry::get_indices(_aliveParticleCount),
                                .Type     = primitive_type::Triangles},
                               &pass);
        _renderer.render_to_target(target, xform);
    }
}

////////////////////////////////////////////////////////

auto particle_emitter::is_alive() const -> bool
{
    return _alive && (!Settings.Lifetime || _remainingLife > milliseconds::zero());
}

void particle_emitter::reset()
{
    if (Settings.Lifetime) { _remainingLife = *Settings.Lifetime; }
    _alive = true;
}

void particle_emitter::emit(particle_system& system, milliseconds deltaTime)
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

static auto minmax_rng(min_max<f32> const& range, auto&& rng) -> f32
{
    return rng(range.first, range.second);
}

static auto minmax_rng(min_max<point_f> const& range, auto&& rng) -> point_f
{
    return point_f {rng(range.first.X, range.second.X), rng(range.first.Y, range.second.Y)};
}

static auto minmax_rng(min_max<degree_f> const& range, auto&& rng) -> degree_f
{
    return degree_f {rng(range.first.Value, range.second.Value)};
}

static auto minmax_rng(min_max<milliseconds> const& range, auto&& rng) -> milliseconds
{
    return milliseconds {rng(range.first.count(), range.second.count())};
}

////////////////////////////////////////////////////////////

static void CalcVelocity(auto&& particle, point_f pos, f32 seconds)
{
    point_f const radial {pos * particle.RadialAcceleration};
    point_f const tangential {pos.as_perpendicular() * particle.TangentialAcceleration};
    particle.Velocity += (radial + tangential + particle.LinearAcceleration + particle.Gravity) * seconds;
    particle.Velocity *= 1.0f / (1.0f + (particle.LinearDamping * seconds));
}

////////////////////////////////////////////////////////////

void particle::update(milliseconds deltaTime)
{
    f32 const seconds {static_cast<f32>(deltaTime.count() / 1000)};

    // age
    RemainingLife -= std::chrono::abs(deltaTime);

    // move
    point_f const pos {(Bounds.center() - Origin).as_normalized()};
    CalcVelocity(*this, pos, seconds);
    Bounds = {{Bounds.Position + Velocity * seconds}, Bounds.Size};

    // spin
    Rotation += degree_f {Spin.Value * static_cast<f32>(seconds)};
}

void particle::init(settings const& tmpl, texture_region const& texRegion, rect_f const& spawnArea, rng& rng)
{
    Region = texRegion;

    auto const dir {point_f::FromDirection(minmax_rng(tmpl.Direction, rng))};
    Velocity               = dir * minmax_rng(tmpl.Speed, rng);
    LinearAcceleration     = dir * minmax_rng(tmpl.LinearAcceleration, rng);
    LinearDamping          = minmax_rng(tmpl.LinearDamping, rng);
    RadialAcceleration     = minmax_rng(tmpl.RadialAcceleration, rng);
    TangentialAcceleration = minmax_rng(tmpl.TangentialAcceleration, rng);
    Gravity                = minmax_rng(tmpl.Gravity, rng);

    auto const col {tmpl.Colors.empty() ? colors::White : tmpl.Colors[rng(usize {0}, tmpl.Colors.size() - 1)]};
    u8 const   alpha {static_cast<u8>(col.A * (1.0f - std::clamp(minmax_rng(tmpl.Transparency, rng), 0.0f, 1.0f)))};
    Color = color {col.R, col.G, col.B, alpha};

    // get id
    ID = get_random_ID();

    // set life
    auto const life {minmax_rng(tmpl.Lifetime, rng)};
    RemainingLife = life;
    StartingLife  = life;

    // set scale
    f32 const scaleF {minmax_rng(tmpl.Scale, rng)};
    Scale = {scaleF, scaleF};

    // set spin and rotation
    Spin     = minmax_rng(tmpl.Spin, rng);
    Rotation = minmax_rng(tmpl.Rotation, rng);

    // calculate random postion
    f32 const x {rng(spawnArea.left(), spawnArea.right()) - (tmpl.Size.Width / 2)};
    f32 const y {rng(spawnArea.top(), spawnArea.bottom()) - (tmpl.Size.Height / 2)};

    // set bounds
    Bounds = {{x, y}, tmpl.Size};
    Origin = Bounds.center();
}

void particle::convert_to(quad* quad) const
{
    point_f const origin {Bounds.center()};
    transform     xform;
    if (Scale != size_f::One) { xform.scale_at(Scale, origin); }
    if (Rotation != degree_f {0}) { xform.rotate_at(Rotation, origin); }

    geometry::set_position(*quad, Bounds, xform);
    geometry::set_color(*quad, Color);
    geometry::set_texcoords(*quad, Region);
}

auto particle::is_alive() const -> bool
{
    return RemainingLife.count() > 0;
}

////////////////////////////////////////////////////////////

}
