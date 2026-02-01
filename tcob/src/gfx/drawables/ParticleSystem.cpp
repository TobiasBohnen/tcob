// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/ParticleSystem.hpp"

#include <chrono>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {

using namespace std::chrono_literals;

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

static void setup_particle_base(particle_base& particle, auto&& tmpl, auto&& rng)
{
    auto const dir {point_f::FromDirection(minmax_rng(tmpl.Direction, rng))};
    particle.Velocity               = dir * minmax_rng(tmpl.Speed, rng);
    particle.LinearAcceleration     = dir * minmax_rng(tmpl.LinearAcceleration, rng);
    particle.LinearDamping          = minmax_rng(tmpl.LinearDamping, rng);
    particle.RadialAcceleration     = minmax_rng(tmpl.RadialAcceleration, rng);
    particle.TangentialAcceleration = minmax_rng(tmpl.TangentialAcceleration, rng);
    particle.Gravity                = minmax_rng(tmpl.Gravity, rng);

    auto const col {tmpl.Colors.empty() ? colors::White : tmpl.Colors[rng(usize {0}, tmpl.Colors.size() - 1)]};
    u8 const   alpha {static_cast<u8>(col.A * (1.0f - std::clamp(minmax_rng(tmpl.Transparency, rng), 0.0f, 1.0f)))};
    particle.Color = color {col.R, col.G, col.B, alpha};

    // reset userdata
    particle.UserData.reset();

    // set life
    auto const life {minmax_rng(tmpl.Lifetime, rng)};
    particle.RemainingLife = life;
    particle.StartingLife  = life;
}

////////////////////////////////////////////////////////////

static void calc_velocity(auto&& particle, point_f pos, f32 seconds)
{
    point_f const radial {pos * particle.RadialAcceleration};
    point_f const tangential {pos.as_perpendicular() * particle.TangentialAcceleration};
    particle.Velocity += (radial + tangential + particle.LinearAcceleration + particle.Gravity) * seconds;
    particle.Velocity *= 1.0f / (1.0f + (particle.LinearDamping * seconds));
}

////////////////////////////////////////////////////////////

void point_particle::convert_to(vertex* vertex) const
{
    vertex->Position  = Position;
    vertex->Color     = Color;
    vertex->TexCoords = {.U = Region.UVRect.left(), .V = Region.UVRect.top(), .Level = static_cast<f32>(Region.Level)};
}

void point_particle::update(milliseconds deltaTime)
{
    f32 const seconds {static_cast<f32>(deltaTime.count() / 1000)};

    // age
    RemainingLife -= std::chrono::abs(deltaTime);

    // move
    point_f const pos {(Position - Origin).as_normalized()};
    calc_velocity(*this, pos, seconds);
    Position += Velocity * seconds;
}

void point_particle::init(settings const& tmpl, texture_region const& texRegion, rect_f const& spawnArea, rng& randomGen)
{
    Region = texRegion;

    setup_particle_base(*this, tmpl, randomGen);

    // calculate random postion
    f32 const x {randomGen(spawnArea.left(), spawnArea.right())};
    f32 const y {randomGen(spawnArea.top(), spawnArea.bottom())};

    // set position
    Position = {x, y};
    Origin   = Position;
}

////////////////////////////////////////////////////////////

void quad_particle::convert_to(quad* quad) const
{
    geometry::set_position(*quad, Bounds, _transform);
    geometry::set_color(*quad, Color);
    geometry::set_texcoords(*quad, Region);
}

void quad_particle::update(milliseconds deltaTime)
{
    f32 const seconds {static_cast<f32>(deltaTime.count() / 1000)};

    // age
    RemainingLife -= std::chrono::abs(deltaTime);

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

void quad_particle::init(settings const& tmpl, texture_region const& texRegion, rect_f const& spawnArea, rng& randomGen)
{
    Region = texRegion;

    setup_particle_base(*this, tmpl, randomGen);

    // set scale
    f32 const scaleF {minmax_rng(tmpl.Scale, randomGen)};
    Scale = {scaleF, scaleF};

    // set spin and rotation
    Spin     = minmax_rng(tmpl.Spin, randomGen);
    Rotation = minmax_rng(tmpl.Rotation, randomGen);

    // calculate random postion
    f32 const x {randomGen(spawnArea.left(), spawnArea.right()) - (tmpl.Size.Width / 2)};
    f32 const y {randomGen(spawnArea.top(), spawnArea.bottom()) - (tmpl.Size.Height / 2)};

    // set bounds
    Bounds = {{x, y}, tmpl.Size};
    Origin = Bounds.center();
}

////////////////////////////////////////////////////////////

auto particle_base::is_alive() const -> bool
{
    return RemainingLife.count() > 0;
}

////////////////////////////////////////////////////////////
}
