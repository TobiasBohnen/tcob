// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/drawables/ParticleSystem.hpp>

#include <algorithm>

#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>

namespace tcob {
using namespace std::chrono_literals;

ParticleSystem::ParticleSystem(const ParticleSystem& other)
{
    *this = other;
}

auto ParticleSystem::operator=(const ParticleSystem& other) -> ParticleSystem&
{
    _position = other._position;
    material(other._material);
    _affectors = other._affectors;
    _emitters = other._emitters;
    return *this;
}

void ParticleSystem::start()
{
    if (_started) {
        return;
    }

    _started = true;
    for (auto& emitter : _emitters) {
        emitter.reset();
    }

    _particles.reserve(100);
    _particles.clear();
    _aliveParticleCount = 0;
}

void ParticleSystem::restart()
{
    stop();
    start();
}

void ParticleSystem::stop()
{
    _started = false;

    _renderer.reset();
    _particles.clear();
    _aliveParticleCount = 0;
}

void ParticleSystem::update(MilliSeconds delta)
{
    std::lock_guard lock { _mutex };

    if (!_started) {
        return;
    }

    for (auto& emitter : _emitters) {
        if (emitter.is_alive()) {
            emitter.emit_particles(*this, delta);
        }
    }

    for (u32 i { 0 }; i < _aliveParticleCount; ++i) {
        auto& particle { _particles[i] };
        if (particle.is_alive()) {
            // call affectors on particle
            for (auto& affector : _affectors) {
                affector(particle);
            }

            // update
            particle.update(delta);

        } else {
            deactivate_particle(i);
            i--;
        }
    }
}

auto ParticleSystem::create_emitter() -> ParticleEmitter&
{
    return _emitters.emplace_back();
}

auto ParticleSystem::material() const -> ResourcePtr<Material>
{
    return _material;
}

void ParticleSystem::material(ResourcePtr<Material> material)
{
    _material = std::move(material);
    if (_material) {
        _renderer.material(_material.object());
    }
}

auto ParticleSystem::position() const -> PointF
{
    return _position;
}

void ParticleSystem::position(const PointF& pos)
{
    _position = pos;
}

auto ParticleSystem::particle_count() const -> u32
{
    return _aliveParticleCount;
}

auto ParticleSystem::activate_particle() -> Particle&
{
    if (_aliveParticleCount == _particles.size()) {
        _aliveParticleCount++;
        return _particles.emplace_back();
    } else {
        return _particles[_aliveParticleCount++];
    }
}

void ParticleSystem::deactivate_particle(isize index)
{
    std::swap(_particles[index], _particles[--_aliveParticleCount]);
}

void ParticleSystem::remove_all_emitters()
{
    _emitters.clear();
    stop();
}

void ParticleSystem::add_affector(const std::function<void(Particle&)>& func)
{
    _affectors.push_back(func);
}

void ParticleSystem::draw(gl::RenderTarget& target)
{
    std::lock_guard lock { _mutex };

    if (_started && _aliveParticleCount > 0 && _material) {
        isize count { 0 };
        Quad* quad { _renderer.map(_aliveParticleCount) };
        for (u32 i { 0 }; i < _aliveParticleCount; ++i) {
            auto& particle { _particles[i] };
            if (particle.is_visible()) {
                quad->position(particle.bounds(), particle.transform());
                quad->color(particle._color);
                quad->texcoords(particle._region);
                quad++;
                count++;
            }
        }
        _renderer.unmap(count);

        _renderer.render_to_target(target);
    }
}

////////////////////////////////////////////////////////////

ParticleEmitter::ParticleEmitter()
    : _parAcceleration { std::pair<f32, f32> { 0.0f, 0.0f } }
    , _parDirectionDegrees { std::pair<f32, f32> { 0.0f, 0.0f } }
    , _parLife { std::pair<f32, f32> { 1000.0f, 1000.0f } }
    , _parScale { std::pair<f32, f32> { 1.0f, 1.0f } }
    , _parSpeed { std::pair<f32, f32> { 0.0f, 0.0f } }
    , _parSpin { std::pair<f32, f32> { 0.0f, 0.0f } }
    , _parTransparency { std::pair<f32, f32> { 0.0f, 0.0f } }
{
}

auto ParticleEmitter::is_alive() const -> bool
{
    return _remainLife > 0ms || _loop;
}

void ParticleEmitter::reset()
{
    _remainLife = _startLife;
}

void ParticleEmitter::texture_region(const std::string& texRegion)
{
    _texture = texRegion;
}

void ParticleEmitter::spawnarea(const RectF& area)
{
    _spawnArea = area;
    _areaXDist = std::pair<f32, f32> { _spawnArea.Left, _spawnArea.Left + _spawnArea.Width };
    _areaYDist = std::pair<f32, f32> { _spawnArea.Top, _spawnArea.Top + _spawnArea.Height };
}

void ParticleEmitter::spawnrate(f32 rate)
{
    _spawnRate = rate;
}

void ParticleEmitter::lifetime(MilliSeconds life)
{
    _startLife = life;
}

void ParticleEmitter::loop(bool loop)
{
    _loop = loop;
}

void ParticleEmitter::particle_size(SizeF size)
{
    _parSize = size;
}

void ParticleEmitter::particle_scale(f32 min, std::optional<f32> max)
{
    if (!max.has_value()) {
        max = min;
    }
    _parScale = std::pair<f32, f32> { min, max.value() };
}

void ParticleEmitter::particle_spin(f32 min, std::optional<f32> max)
{
    if (!max.has_value()) {
        max = min;
    }
    _parSpin = std::pair<f32, f32> { min, max.value() };
}

void ParticleEmitter::particle_direction(f32 min, std::optional<f32> max)
{
    if (!max.has_value()) {
        max = min;
    }
    min = static_cast<f32>(std::fmod(min, 360));
    max = static_cast<f32>(std::fmod(max.value(), 360));
    _parDirectionDegrees = std::pair<f32, f32> { min, max.value() };
}

void ParticleEmitter::particle_speed(f32 min, std::optional<f32> max)
{
    if (!max.has_value()) {
        max = min;
    }
    _parSpeed = std::pair<f32, f32> { min, max.value() };
}

void ParticleEmitter::particle_lifetime(MilliSeconds min, std::optional<MilliSeconds> max)
{
    if (!max.has_value()) {
        max = min;
    }
    _parLife = std::pair<f32, f32> { static_cast<f32>(min.count()), static_cast<f32>(max.value().count()) };
}

void ParticleEmitter::particle_transparency(f32 min, std::optional<f32> max)
{
    if (!max.has_value()) {
        max = min;
    }
    _parTransparency = std::pair<f32, f32> { min, max.value() };
}

void ParticleEmitter::particle_acceleration(f32 min, std::optional<f32> max)
{
    if (!max.has_value()) {
        max = min;
    }
    _parAcceleration = std::pair<f32, f32> { min, max.value() };
}

void ParticleEmitter::emit_particles(ParticleSystem& system, MilliSeconds time)
{
    _remainLife -= time;

    const f64 particleAmount { _spawnRate * (time.count() / 1000) + _emissionDiff };
    const u32 particleCount { static_cast<u32>(particleAmount) };
    _emissionDiff = particleAmount - particleCount;

    for (u32 i = 0; i < particleCount; ++i) {
        auto& particle { system.activate_particle() };
        particle.Stage = 0;

        particle.material(system._material, _texture);

        // calculate random particle velocity
        particle.direction(_randomGen(_parDirectionDegrees.first, _parDirectionDegrees.second));
        particle.speed(_randomGen(_parSpeed.first, _parSpeed.second));
        particle.acceleration(_randomGen(_parAcceleration.first, _parAcceleration.second));

        // set spin
        particle.spin(_randomGen(_parSpin.first, _parSpin.second));

        // set size
        const f32 scaleF { _randomGen(_parScale.first, _parScale.second) };
        particle.scale({ scaleF, scaleF });
        particle.size(_parSize);

        // set life
        particle.lifetime(MilliSeconds { _randomGen(_parLife.first, _parLife.second) });

        // calculate random postion
        const f32 x { _randomGen(_areaXDist.first, _areaXDist.second) - _parSize.Width / 2 };
        const f32 y { _randomGen(_areaYDist.first, _areaYDist.second) - _parSize.Height / 2 };
        const PointF& offset { system._position };
        particle.position({ x + offset.X, y + offset.Y });

        // set transparency
        particle.transparency(_randomGen(_parTransparency.first, _parTransparency.second));

        // defaults
        particle.color(Colors::White);
        particle.show();
    }
}

////////////////////////////////////////////////////////////

Particle::Particle()
{
    color(Colors::White);
}

auto Particle::is_alive() const -> bool
{
    return _remainingLife > 0ms;
}

void Particle::update(MilliSeconds delta)
{
    // age
    _remainingLife -= delta;

    const f64 s { delta.count() / 1000 };

    // move
    PointF offset;
    _speed += _acc * s;
    const f64 var { _speed * s };
    offset.X = static_cast<f32>(_direction.X * var);
    offset.Y = static_cast<f32>(_direction.Y * var);
    move_by(offset);

    // spin
    rotate_by(static_cast<f32>(_spin * s));
    update_transform();
}

void Particle::direction(f32 dir)
{
    _dirInDegrees = std::fmod(dir, 360.f);
    const f64 radians { (_dirInDegrees - 90) * TAU / 360.0f };
    _direction = {
        static_cast<f32>(std::cos(radians)),
        static_cast<f32>(std::sin(radians))
    };
}

auto Particle::direction() -> f32
{
    return _dirInDegrees;
}

void Particle::speed(f64 speed)
{
    _speed = speed;
}

auto Particle::speed() -> f64
{
    return _speed;
}

void Particle::acceleration(f32 acc)
{
    _acc = acc;
}

auto Particle::acceleration() -> f32
{
    return _acc;
}

void Particle::spin(f32 spin)
{
    _spin = spin;
}

auto Particle::spin() -> f32
{
    return _spin;
}

void Particle::lifetime(MilliSeconds life)
{
    _startingLife = life;
    _remainingLife = life;
}

auto Particle::life_ratio() const -> f32
{
    return static_cast<f32>(_remainingLife / _startingLife);
}

void Particle::material(ResourcePtr<Material> material, const std::string& texRegion)
{
    _material = std::move(material);
    if (!_material) {
        return;
    }

    auto& regions { _material->Texture->regions() };
    _region = regions[texRegion];
}

auto Particle::color() const -> Color
{
    return _color;
}

void Particle::color(const Color& color)
{
    _color = color;
}

void Particle::transparency(f32 trans)
{
    _color.A = 255 - static_cast<u8>(255 * std::clamp(trans, 0.f, 1.f));
}

auto Particle::is_visible() const -> bool
{
    return _visible;
}

void Particle::show()
{
    _visible = true;
}

void Particle::hide()
{
    _visible = false;
}

}