// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/ParticleSystem.hpp"

#include <algorithm>
#include <array>
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

////////////////////////////////////////////////////////////

void particles::reserve(isize count)
{
    RemainingLife.reserve(count);
    Velocity.reserve(count);
    LinearAcceleration.reserve(count);
    LinearDamping.reserve(count);
    RadialAcceleration.reserve(count);
    TangentialAcceleration.reserve(count);
    Gravity.reserve(count);
    Bounds.reserve(count);
    Rotation.reserve(count);
    Spin.reserve(count);
    Scale.reserve(count);
    Origin.reserve(count);
    StartingLife.reserve(count);
    Color.reserve(count);
    UVRegion.reserve(count);
    ID.reserve(count);
    EmitterID.reserve(count);
}

void particles::push_back()
{
    RemainingLife.emplace_back(0);
    Velocity.push_back(point_f::Zero);
    LinearAcceleration.push_back(point_f::Zero);
    LinearDamping.push_back(0.0f);
    RadialAcceleration.push_back(0.0f);
    TangentialAcceleration.push_back(0.0f);
    Gravity.push_back(point_f::Zero);
    Bounds.push_back(rect_f::Zero);
    Rotation.push_back(0.0f);
    Spin.push_back(0.0f);
    Scale.push_back(size_f::One);
    Origin.push_back(point_f::Zero);
    StartingLife.emplace_back(0);
    Color.push_back(colors::White);
    UVRegion.push_back({});
    ID.push_back(0);
    EmitterID.push_back(0);
}

void particles::swap(isize a, isize b)
{
    std::swap(RemainingLife[a], RemainingLife[b]);
    std::swap(Velocity[a], Velocity[b]);
    std::swap(LinearAcceleration[a], LinearAcceleration[b]);
    std::swap(LinearDamping[a], LinearDamping[b]);
    std::swap(RadialAcceleration[a], RadialAcceleration[b]);
    std::swap(TangentialAcceleration[a], TangentialAcceleration[b]);
    std::swap(Gravity[a], Gravity[b]);
    std::swap(Bounds[a], Bounds[b]);
    std::swap(Rotation[a], Rotation[b]);
    std::swap(Spin[a], Spin[b]);
    std::swap(Scale[a], Scale[b]);
    std::swap(Origin[a], Origin[b]);
    std::swap(StartingLife[a], StartingLife[b]);
    std::swap(Color[a], Color[b]);
    std::swap(UVRegion[a], UVRegion[b]);
    std::swap(ID[a], ID[b]);
    std::swap(EmitterID[a], EmitterID[b]);
}

////////////////////////////////////////////////////////////

particle_system::particle_system(bool multiThreaded, isize reservedParticleCount)
    : _multiThreaded {multiThreaded}
{
    if (reservedParticleCount > 0) {
        reserve(reservedParticleCount);
    }
}

void particle_system::reserve(isize count)
{
    Particles.reserve(count);
    _geometry.reserve(count);
    _indices = geometry::get_indices(count);
}

auto particle_system::is_running() const -> bool
{
    return _isRunning;
}

void particle_system::start()
{
    if (_isRunning) { return; }

    std::erase_if(_emitters, [](auto const& e) { return e->Settings.Transient; });

    _isRunning = true;
    for (auto& emitter : _emitters) { emitter->reset(); }

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
    _aliveParticleCount = 0;
    _geometry.clear();
}

auto particle_system::create_emitter() -> particle_emitter&
{
    auto& retValue {*_emitters.emplace_back(std::make_unique<particle_emitter>(get_random_ID()))};
    return retValue;
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

auto particle_system::activate_particle() -> isize
{
    if (_aliveParticleCount == std::ssize(Particles.RemainingLife)) {
        Particles.push_back();
    }

    return _aliveParticleCount++;
}

void particle_system::deactivate_particle(isize index)
{
    assert(_aliveParticleCount > 0);
    Particles.swap(index, --_aliveParticleCount);
}

void particle_system::on_update(milliseconds deltaTime)
{
    if (!_isRunning || !Material) { return; }

    for (auto& emitter : _emitters) {
        emitter->emit(*this, deltaTime);
    }

    std::set<isize, std::greater<>> toBeDeactivated;

    f32 const seconds {static_cast<f32>(deltaTime.count() / 1000)};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                if (Particles.RemainingLife[i].count() <= 0) {
                    std::scoped_lock lock {_mutex};
                    toBeDeactivated.insert(i);
                    continue;
                }

                // age
                Particles.RemainingLife[i] -= deltaTime;

                // update
                ParticleUpdate({.Particles = Particles, .Index = i, .DeltaTime = deltaTime});

                // move
                auto& bounds {Particles.Bounds[i]};
                auto& vel {Particles.Velocity[i]};
                auto& origin {Particles.Origin[i]};

                point_f const c {bounds.center()};
                point_f       p {c - origin};
                f64 const     len {p.length()};
                if (len > 0.0) { p /= len; }

                auto const& la {Particles.LinearAcceleration[i]};
                auto const& grav {Particles.Gravity[i]};
                f32 const   ra {Particles.RadialAcceleration[i]};
                f32 const   ta {Particles.TangentialAcceleration[i]};
                f32 const   ld {Particles.LinearDamping[i]};

                // radial + tangential + linear + gravity
                vel += ((p * ra) + (p.as_perpendicular() * ta) + la + grav) * seconds;
                f32 const damp {1.0f / (1.0f + (ld * seconds))};
                vel *= damp;

                bounds = {bounds.left() + (vel.X * seconds), bounds.top() + (vel.Y * seconds), bounds.width(), bounds.height()};

                // spin
                Particles.Rotation[i] += Particles.Spin[i] * seconds;
            }
        },
        _aliveParticleCount, _multiThreaded ? 64 : _aliveParticleCount);

    for (auto const& i : toBeDeactivated) {
        ParticleDeath({.Particles = Particles, .Index = i, .DeltaTime = deltaTime});
        deactivate_particle(i);
    }
}

auto particle_system::can_draw() const -> bool
{
    return _isRunning && _aliveParticleCount != 0 && !(*Material).is_expired();
}

void particle_system::on_draw_to(render_target& target, transform const& xform)
{
    _geometry.resize(_aliveParticleCount);

    if (std::ssize(_indices) < _aliveParticleCount * 6) {
        _indices = geometry::get_indices(_aliveParticleCount);
    }

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                auto&       q {_geometry[i]};
                auto const& bounds {Particles.Bounds[i]};
                f32 const   rot {Particles.Rotation[i]};

                point_f const origin {bounds.left() + (bounds.width() * 0.5f), bounds.top() + (bounds.height() * 0.5f)};
                transform     tf;
                if (Particles.Scale[i] != size_f::One) { tf.scale_at(Particles.Scale[i], origin); }
                if (rot != 0.0f) { tf.rotate_at(degree_f {rot}, origin); }

                geometry::set_position(q, bounds, tf);
                geometry::set_color(q, Particles.Color[i]);
                geometry::set_texcoords(q, Particles.UVRegion[i]);
            }
        },
        _aliveParticleCount,
        _multiThreaded ? 64 : _aliveParticleCount);

    for (isize i {0}; i < Material->pass_count(); ++i) {
        auto const& pass {Material->get_pass(i)};
        _renderer.set_geometry({.Vertices = geometry::flatten({_geometry.data(), static_cast<usize>(_aliveParticleCount)}),
                                .Indices  = {_indices.data(), static_cast<usize>(_aliveParticleCount * 6)},
                                .Type     = primitive_type::Triangles},
                               &pass);
        _renderer.render_to_target(target, xform);
    }
}

////////////////////////////////////////////////////////

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

particle_emitter::particle_emitter(uid id)
    : _id {id}
{
}

auto particle_emitter::is_alive() const -> bool
{
    return _alive && (!Settings.Lifetime || _remainingLife > milliseconds::zero());
}

auto particle_emitter::id() const -> uid
{
    return _id;
}

void particle_emitter::reset()
{
    if (Settings.Lifetime) { _remainingLife = *Settings.Lifetime; }
    _alive = true;

    overloaded_visit(
        Settings.Pattern,
        [&](emit_linear const&) { },
        [&](emit_burst const& pattern) {
            _burstTimer       = milliseconds::zero();
            _burstRepeatsLeft = pattern.Repeats;
        });
}

void particle_emitter::emit(particle_system& system, milliseconds deltaTime)
{
    if (!is_alive()) { return; }

    _remainingLife -= deltaTime;

    i32 particleCount {0};

    overloaded_visit(
        Settings.Pattern,
        [&](emit_linear const& pattern) {
            f64 const particleAmount {(pattern.Rate * (deltaTime.count() / 1000)) + _linearDiff};
            particleCount = static_cast<i32>(particleAmount);
            _linearDiff   = particleAmount - particleCount;
        },
        [&](emit_burst const& pattern) {
            _burstTimer -= deltaTime;
            if (_burstTimer.count() <= 0) {
                particleCount = static_cast<i32>(pattern.Count);
                if (pattern.Repeats == -1 || --_burstRepeatsLeft > 0) {
                    _burstTimer = pattern.Interval;
                } else {
                    _alive = false;
                }
            }
        });

    auto const& tmpl {Settings.Template};
    auto const& texRegion {system.Material->first_pass().Texture->regions()[tmpl.TextureRegion]}; // TODO texRegion pass

    auto& p {system.Particles};

    for (i32 n {0}; n < particleCount; ++n) {
        isize const idx {system.activate_particle()};

        p.UVRegion[idx] = texRegion;

        auto const dir {point_f::FromDirection(minmax_rng(tmpl.Direction, _rng))};
        p.Velocity[idx]               = dir * minmax_rng(tmpl.Speed, _rng);
        p.LinearAcceleration[idx]     = dir * minmax_rng(tmpl.LinearAcceleration, _rng);
        p.LinearDamping[idx]          = minmax_rng(tmpl.LinearDamping, _rng);
        p.RadialAcceleration[idx]     = minmax_rng(tmpl.RadialAcceleration, _rng);
        p.TangentialAcceleration[idx] = minmax_rng(tmpl.TangentialAcceleration, _rng);
        p.Gravity[idx]                = minmax_rng(tmpl.Gravity, _rng);

        color const col {tmpl.Colors.empty() ? colors::White : tmpl.Colors[_rng(usize {0}, tmpl.Colors.size() - 1)]};
        u8 const    alpha {static_cast<u8>(col.A * (1.0f - std::clamp(minmax_rng(tmpl.Transparency, _rng), 0.0f, 1.0f)))};
        p.Color[idx] = {col.R, col.G, col.B, alpha};

        // get id
        p.ID[idx]        = get_random_ID();
        p.EmitterID[idx] = _id;

        // set life
        milliseconds const life {minmax_rng(tmpl.Lifetime, _rng)};
        p.RemainingLife[idx] = life;
        p.StartingLife[idx]  = life;

        // set scale
        f32 const scaleF {minmax_rng(tmpl.Scale, _rng)};
        p.Scale[idx] = {scaleF, scaleF};

        // set spin and rotation
        p.Spin[idx]     = minmax_rng(tmpl.Spin, _rng).Value;
        p.Rotation[idx] = minmax_rng(tmpl.Rotation, _rng).Value;

        // calculate random position
        f32 const x {_rng(Settings.SpawnArea.left(), Settings.SpawnArea.right()) - (tmpl.Size.Width / 2)};
        f32 const y {_rng(Settings.SpawnArea.top(), Settings.SpawnArea.bottom()) - (tmpl.Size.Height / 2)};

        // set bounds
        p.Bounds[idx] = {x, y, tmpl.Size.Width, tmpl.Size.Height};
        p.Origin[idx] = {x + (tmpl.Size.Width * 0.5f), y + (tmpl.Size.Height * 0.5f)};
    }
}

////////////////////////////////////////////////////////////

}
