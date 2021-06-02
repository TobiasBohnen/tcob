// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <functional>
#include <list>
#include <optional>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/Random.hpp>
#include <tcob/gfx/Transformable.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>

namespace tcob {
class Particle final : public RectTransformable, public Updatable {
public:
    Particle();

    void update(f64 deltaTime) override;

    auto direction() -> f32;
    void direction(f32 dir);

    auto speed() -> f64;
    void speed(f64 speed);

    auto acceleration() -> f32;
    void acceleration(f32 acc);

    auto spin() -> f32;
    void spin(f32 spin);

    void lifetime(f64 life);
    auto life_ratio() const -> f32;
    auto is_alive() const -> bool;

    auto is_visible() const -> bool;
    void show();
    void hide();

    void material(ResourcePtr<Material>, const std::string& texRegion);

    auto color() const -> Color;
    void color(const Color& color);
    void transparency(f32 trans);

    i32 Stage { 0 };

private:
    friend class ParticleSystem;

    f32 _dirInDegrees { 0 };
    PointF _direction { PointF::Zero };
    f64 _speed { 0 };
    f32 _spin { 0 };
    f32 _acc { 0 };
    f64 _startingLife { 0 };
    f64 _remainingLife { 0 };

    Color _color;
    ResourcePtr<Material> _material;
    TextureRegion _region;
    bool _visible { true };
};

////////////////////////////////////////////////////////////

class ParticleEmitter final {
public:
    ParticleEmitter();

    auto is_alive() const -> bool;
    void reset();

    void texture_region(const std::string& texRegion);

    void spawnarea(const RectF& area);
    void spawnrate(f32 rate);
    void lifetime(f64 life);
    void loop(bool loop);

    void particle_acceleration(f32 min, std::optional<f32> max = std::nullopt);
    void particle_direction(f32 min, std::optional<f32> max = std::nullopt);
    void particle_lifetime(f32 min, std::optional<f32> max = std::nullopt);
    void particle_scale(f32 min, std::optional<f32> max = std::nullopt);
    void particle_size(SizeF size);
    void particle_speed(f32 min, std::optional<f32> max = std::nullopt);
    void particle_spin(f32 min, std::optional<f32> max = std::nullopt);
    void particle_transparency(f32 min, std::optional<f32> max = std::nullopt);

    void emit_particles(ParticleSystem& system, f64 time);

private:
    Random _randomGen;

    std::string _texture;

    RectF _spawnArea { RectF::Zero };
    std::pair<f32, f32> _areaXDist;
    std::pair<f32, f32> _areaYDist;

    f32 _spawnRate { 1 };
    f64 _startLife { 1000 };
    f64 _remainLife { 1000 };
    bool _loop { false };

    SizeF _parSize { SizeF::Zero };

    std::pair<f32, f32> _parAcceleration;
    std::pair<f32, f32> _parDirectionDegrees;
    std::pair<f32, f32> _parLife;
    std::pair<f32, f32> _parScale;
    std::pair<f32, f32> _parSpeed;
    std::pair<f32, f32> _parSpin;
    std::pair<f32, f32> _parTransparency;

    f64 _emissionDiff { 0 };
};

////////////////////////////////////////////////////////////

class ParticleSystem final : public Drawable {
public:
    ParticleSystem() = default;
    ~ParticleSystem() = default;

    ParticleSystem(const ParticleSystem& other);
    auto operator=(const ParticleSystem& other) -> ParticleSystem&;

    void start();
    void restart();
    void stop();

    auto create_emitter() -> ParticleEmitter&;
    void remove_all_emitters();

    void add_affector(const std::function<void(Particle&)>& func);

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material);

    auto position() const -> PointF;
    void position(const PointF& position);

    auto particle_count() const -> u32;

    void update(f64 deltaTime) override;

    void draw(gl::RenderTarget& target) override;

private:
    friend void ParticleEmitter::emit_particles(ParticleSystem& system, f64 time);
    auto activate_particle() -> Particle&;
    void deactivate_particle(isize index);

    bool _started { false };
    PointF _position { PointF::Zero };

    gl::DynamicQuadRenderer _renderer {};
    ResourcePtr<Material> _material;
    std::vector<std::function<void(Particle&)>> _affectors;
    std::vector<Particle> _particles;
    u32 _aliveParticleCount { 0 };
    std::vector<ParticleEmitter> _emitters;
};
}