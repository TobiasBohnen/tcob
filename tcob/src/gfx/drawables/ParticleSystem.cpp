// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/ParticleSystem.hpp"

namespace tcob::gfx {

using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

void static calc_velocity(auto&& particle, point_f pos, f32 seconds)
{
    point_f const radial {pos * particle.RadialAcceleration};
    point_f const tangential {pos.perpendicular() * particle.TangentialAcceleration};
    particle.Velocity += (radial + tangential + particle.LinearAcceleration + particle.Gravity) * seconds;
    particle.Velocity *= 1.0f / (1.0f + (particle.LinearDamping * seconds));
}

////////////////////////////////////////////////////////////

void point_particle::convert_to(vertex* vertex) const
{
    vertex->Position  = Position;
    vertex->Color     = Color;
    vertex->TexCoords = {Region.UVRect.left(), Region.UVRect.top(), static_cast<f32>(Region.Level)};
}

void point_particle::update(milliseconds deltaTime)
{
    f32 const seconds {static_cast<f32>(deltaTime.count() / 1000)};

    // age
    RemainingLife -= deltaTime;

    // move
    point_f const pos {(Position - Origin).as_normalized()};
    calc_velocity(*this, pos, seconds);
    Position += Velocity * seconds;
}

////////////////////////////////////////////////////////////

void quad_particle::convert_to(quad* quad) const
{
    geometry::set_position(*quad, Bounds, _transform);
    geometry::set_color(*quad, Color);
    geometry::set_texcoords(*quad, Region);
}

void quad_particle::update(milliseconds delta)
{
    f32 const seconds {static_cast<f32>(delta.count() / 1000)};

    // age
    RemainingLife -= delta;

    // move
    point_f const pos {(Bounds.center() - Origin).as_normalized()};
    calc_velocity(*this, pos, seconds);
    Bounds = {{Bounds.Position + Velocity * seconds}, Bounds.Size};

    // spin
    Rotation += degree_f {Spin.Value * static_cast<f32>(seconds)};

    // transform
    point_f const origin {Bounds.center()};
    _transform.to_identity();
    if (Scale != size_f::One) { _transform.scale_at(Scale, origin); }
    if (Rotation != degree_f {0}) { _transform.rotate_at(Rotation, origin); }
}

////////////////////////////////////////////////////////////

auto static minmax_rng(min_max<f32> const& range, auto&& rng) -> f32
{
    return rng(range.first, range.second);
}

auto static minmax_rng(min_max<point_f> const& range, auto&& rng) -> point_f
{
    return point_f {rng(range.first.X, range.second.X), rng(range.first.Y, range.second.Y)};
}

auto static minmax_rng(min_max<degree_f> const& range, auto&& rng) -> degree_f
{
    return degree_f {rng(range.first.Value, range.second.Value)};
}

auto static minmax_rng(min_max<milliseconds> const& range, auto&& rng) -> milliseconds
{
    return milliseconds {rng(range.first.count(), range.second.count())};
}

void static setup_particle(auto&& particle, auto&& tmpl, auto&& _randomGen)
{
    auto const dir {point_f::FromDirection(minmax_rng(tmpl.Direction, _randomGen))};
    particle.Velocity               = dir * minmax_rng(tmpl.Speed, _randomGen);
    particle.LinearAcceleration     = dir * minmax_rng(tmpl.LinearAcceleration, _randomGen);
    particle.LinearDamping          = minmax_rng(tmpl.LinearDamping, _randomGen);
    particle.RadialAcceleration     = minmax_rng(tmpl.RadialAcceleration, _randomGen);
    particle.TangentialAcceleration = minmax_rng(tmpl.TangentialAcceleration, _randomGen);
    particle.Gravity                = minmax_rng(tmpl.Gravity, _randomGen);

    auto const col {tmpl.Colors.empty() ? colors::White : tmpl.Colors[_randomGen(usize {0}, tmpl.Colors.size() - 1)]};
    u8 const   alpha {static_cast<u8>(col.A * (1.0f - std::clamp(minmax_rng(tmpl.Transparency, _randomGen), 0.0f, 1.0f)))};
    particle.Color = color {col.R, col.G, col.B, alpha};

    // reset userdata
    particle.UserData.reset();

    // set life
    auto const life {minmax_rng(tmpl.Lifetime, _randomGen)};
    particle.RemainingLife = life;
    particle.StartingLife  = life;
}

////////////////////////////////////////////////////////////

auto point_particle_emitter::is_alive() const -> bool
{
    return _alive && (!Settings.Lifetime || _remainingLife > 0ms);
}

void point_particle_emitter::reset()
{
    if (Settings.Lifetime) { _remainingLife = *Settings.Lifetime; }
    _alive = true;
}

void point_particle_emitter::emit(particle_system<point_particle_emitter>& system, milliseconds deltaTime)
{
    if (!is_alive()) { return; }

    _remainingLife -= deltaTime;

    i32 particleCount {0};
    if (Settings.Explosion) {
        particleCount = static_cast<i32>(Settings.SpawnRate);
        _alive        = false;
    } else {
        f64 const particleAmount {(Settings.SpawnRate * (deltaTime.count() / 1000)) + _emissionDiff};
        particleCount = static_cast<i32>(particleAmount);
        _emissionDiff = particleAmount - particleCount;
    }

    auto const& tmpl {Settings.Template};
    auto const& texRegion {system.Material->Texture->get_region(tmpl.Texture)};

    for (i32 i {0}; i < particleCount; ++i) {
        auto& particle {system.activate_particle()};

        particle.Region = texRegion;

        setup_particle(particle, tmpl, _randomGen);

        // calculate random postion
        f32 const x {_randomGen(Settings.SpawnArea.left(), Settings.SpawnArea.right())};
        f32 const y {_randomGen(Settings.SpawnArea.top(), Settings.SpawnArea.bottom())};

        // set position
        particle.Position = {x, y};
        particle.Origin   = particle.Position;
    }
}

////////////////////////////////////////////////////////////

auto quad_particle_emitter::is_alive() const -> bool
{
    return _alive && (!Settings.Lifetime || _remainingLife > 0ms);
}

void quad_particle_emitter::reset()
{
    if (Settings.Lifetime) { _remainingLife = *Settings.Lifetime; }
    _alive = true;
}

void quad_particle_emitter::emit(particle_system<quad_particle_emitter>& system, milliseconds deltaTime)
{
    if (!is_alive()) { return; }

    _remainingLife -= deltaTime;

    i32 particleCount {0};
    if (Settings.Explosion) {
        particleCount = static_cast<i32>(Settings.SpawnRate);
        _alive        = false;
    } else {
        f64 const particleAmount {(Settings.SpawnRate * (deltaTime.count() / 1000)) + _emissionDiff};
        particleCount = static_cast<i32>(particleAmount);
        _emissionDiff = particleAmount - particleCount;
    }

    auto const& tmpl {Settings.Template};
    auto const& texRegion {system.Material->Texture->get_region(tmpl.Texture)};

    for (i32 i {0}; i < particleCount; ++i) {
        auto& particle {system.activate_particle()};

        particle.Region = texRegion;

        setup_particle(particle, tmpl, _randomGen);

        // set scale
        f32 const scaleF {minmax_rng(tmpl.Scale, _randomGen)};
        particle.Scale = {scaleF, scaleF};

        // set spin and rotation
        particle.Spin     = minmax_rng(tmpl.Spin, _randomGen);
        particle.Rotation = minmax_rng(tmpl.Rotation, _randomGen);

        // calculate random postion
        f32 const x {_randomGen(Settings.SpawnArea.left(), Settings.SpawnArea.right()) - (tmpl.Size.Width / 2)};
        f32 const y {_randomGen(Settings.SpawnArea.top(), Settings.SpawnArea.bottom()) - (tmpl.Size.Height / 2)};

        // set bounds
        particle.Bounds = {{x, y}, tmpl.Size};
        particle.Origin = particle.Bounds.center();
    }
}

////////////////////////////////////////////////////////////
}
