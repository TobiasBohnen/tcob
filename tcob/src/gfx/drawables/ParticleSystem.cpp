// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/ParticleSystem.hpp"

namespace tcob::gfx {

using namespace std::chrono_literals;

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
    Speed += Acceleration * seconds;
    point_f const offset {point_f::FromDirection(Direction) * (Speed * seconds)};
    Bounds = {{Bounds.Position + offset}, Bounds.Size};

    // spin
    Rotation += degree_f {Spin.Value * static_cast<f32>(seconds)};

    // transform
    point_f const origin {Bounds.center()};
    _transform.to_identity();
    if (Scale != size_f::One) { _transform.scale_at(Scale, origin); }
    if (Rotation != degree_f {0}) { _transform.rotate_at(Rotation, origin); }
}

////////////////////////////////////////////////////////////

auto quad_particle_emitter::is_alive() const -> bool
{
    return !Settings.Lifetime || _remainingLife > 0ms;
}

void quad_particle_emitter::reset()
{
    if (Settings.Lifetime) { _remainingLife = *Settings.Lifetime; }
}

void quad_particle_emitter::emit(particle_system<quad_particle_emitter>& system, milliseconds deltaTime)
{
    if (!is_alive()) { return; }

    _remainingLife -= deltaTime;

    f64 const particleAmount {(Settings.SpawnRate * (deltaTime.count() / 1000)) + _emissionDiff};
    i32 const particleCount {static_cast<i32>(particleAmount)};
    _emissionDiff = particleAmount - particleCount;

    auto const& texRegion {system.Material->Texture->get_region(Settings.Template.Texture)};

    for (i32 i {0}; i < particleCount; ++i) {
        auto& particle {system.activate_particle()};

        particle.Region = texRegion;

        u8 const alpha {static_cast<u8>(255 - static_cast<u8>(255 * std::clamp(_randomGen(Settings.Template.Transparency.first, Settings.Template.Transparency.second), 0.0f, 1.0f)))};

        particle.Direction    = degree_f {_randomGen(Settings.Template.Direction.first.Value, Settings.Template.Direction.second.Value)};
        particle.Acceleration = _randomGen(Settings.Template.Acceleration.first, Settings.Template.Acceleration.second);
        particle.Speed        = _randomGen(Settings.Template.Speed.first, Settings.Template.Speed.second);
        particle.Spin         = degree_f {_randomGen(Settings.Template.Spin.first.Value, Settings.Template.Spin.second.Value)};
        particle.Rotation     = degree_f {_randomGen(Settings.Template.Rotation.first, Settings.Template.Rotation.second)};
        particle.Color        = color {Settings.Template.Color.R, Settings.Template.Color.G, Settings.Template.Color.B, static_cast<u8>((Settings.Template.Color.A + alpha) / 2)};

        // reset userdata
        particle.UserData.reset();

        // set scale
        f32 const scaleF {_randomGen(Settings.Template.Scale.first, Settings.Template.Scale.second)};
        particle.Scale = {scaleF, scaleF};

        // set life
        auto const life {milliseconds {_randomGen(Settings.Template.Lifetime.first.count(), Settings.Template.Lifetime.second.count())}};
        particle.RemainingLife = life;
        particle.StartingLife  = life;

        // calculate random postion
        f32 const x {_randomGen(Settings.SpawnArea.left(), Settings.SpawnArea.right()) - (Settings.Template.Size.Width / 2)};
        f32 const y {_randomGen(Settings.SpawnArea.top(), Settings.SpawnArea.bottom()) - (Settings.Template.Size.Height / 2)};

        // set bounds
        particle.Bounds = {{x, y}, Settings.Template.Size};
    }
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
    Speed += Acceleration * seconds;
    point_f const offset {point_f::FromDirection(Direction) * (Speed * seconds)};
    Position += offset;
}

////////////////////////////////////////////////////////////

auto point_particle_emitter::is_alive() const -> bool
{
    return !Settings.Lifetime || _remainingLife > 0ms;
}

void point_particle_emitter::reset()
{
    if (Settings.Lifetime) { _remainingLife = *Settings.Lifetime; }
}

void point_particle_emitter::emit(particle_system<point_particle_emitter>& system, milliseconds deltaTime)
{
    if (!is_alive()) { return; }

    _remainingLife -= deltaTime;

    f64 const particleAmount {(Settings.SpawnRate * (deltaTime.count() / 1000)) + _emissionDiff};
    i32 const particleCount {static_cast<i32>(particleAmount)};
    _emissionDiff = particleAmount - particleCount;

    auto const& texRegion {system.Material->Texture->get_region(Settings.Template.Texture)};

    for (i32 i {0}; i < particleCount; ++i) {
        auto& particle {system.activate_particle()};

        particle.Region = texRegion;

        u8 const alpha {static_cast<u8>(255 - static_cast<u8>(255 * std::clamp(_randomGen(Settings.Template.Transparency.first, Settings.Template.Transparency.second), 0.0f, 1.0f)))};

        particle.Direction    = degree_f {_randomGen(Settings.Template.Direction.first.Value, Settings.Template.Direction.second.Value)};
        particle.Acceleration = _randomGen(Settings.Template.Acceleration.first, Settings.Template.Acceleration.second);
        particle.Speed        = _randomGen(Settings.Template.Speed.first, Settings.Template.Speed.second);
        particle.Color        = color {Settings.Template.Color.R, Settings.Template.Color.G, Settings.Template.Color.B, static_cast<u8>((Settings.Template.Color.A + alpha) / 2)};

        // reset userdata
        particle.UserData.reset();

        // set life
        auto const life {milliseconds {_randomGen(Settings.Template.Lifetime.first.count(), Settings.Template.Lifetime.second.count())}};
        particle.RemainingLife = life;
        particle.StartingLife  = life;

        // calculate random postion
        f32 const x {_randomGen(Settings.SpawnArea.left(), Settings.SpawnArea.right())};
        f32 const y {_randomGen(Settings.SpawnArea.top(), Settings.SpawnArea.bottom())};

        // set position
        particle.Position = {x, y};
    }
}

////////////////////////////////////////////////////////////

}
