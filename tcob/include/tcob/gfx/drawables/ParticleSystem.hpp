// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class particle_system;

////////////////////////////////////////////////////////////

class TCOB_API particle final {
public:
    std::any UserData;

    size_f Scale {size_f::One};
    rect_f Bounds {rect_f::Zero};

    degree_f Spin {0.0f};
    degree_f Rotation {0};

    f32      Acceleration {0.0f};
    f32      Speed {0.0};
    degree_f Direction {0.0f};

    color Color {colors::White};

    auto get_lifetime_ratio() const -> f32;
    auto is_alive() const -> bool;
    void set_lifetime(milliseconds life);

    void set_texture_region(texture_region const& texRegion);

    void to_quad(quad* quad);

    void update(milliseconds deltaTime);

private:
    texture_region _region {};
    milliseconds   _startingLife {0};
    milliseconds   _remainingLife {0};
};

////////////////////////////////////////////////////////////

struct [[nodiscard]] particle_template {
    std::pair<f32, f32> Scale;
    size_f              Size;

    std::pair<degree_f, degree_f> Spin;
    std::pair<f32, f32>           Rotation;

    std::pair<f32, f32>           Acceleration;
    std::pair<f32, f32>           Speed;
    std::pair<degree_f, degree_f> Direction;

    string              Texture;
    color               Color;
    std::pair<f32, f32> Transparency;

    std::pair<milliseconds, milliseconds> Lifetime;

    auto operator==(particle_template const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API particle_emitter_base {
public:
    virtual ~particle_emitter_base() = default;

    void virtual reset() = 0;

    void virtual emit_particles(particle_system& system, milliseconds time) = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API particle_emitter final : public particle_emitter_base {
public:
    particle_emitter();
    ~particle_emitter() override;

    particle_template           Template;
    rect_f                      SpawnArea {rect_f::Zero};
    f32                         SpawnRate {0};
    std::optional<milliseconds> Lifetime {};

    auto is_alive() const -> bool;

    void reset() override;

    void emit_particles(particle_system& system, milliseconds time) override;

private:
    rng          _randomGen;
    milliseconds _remainLife {1000};
    f64          _emissionDiff {0};
};

////////////////////////////////////////////////////////////

struct particle_event {
    particle&    Particle;
    milliseconds Time;
};

////////////////////////////////////////////////////////////

class TCOB_API particle_system final : public drawable {
public:
    explicit particle_system(bool multiThreaded = false);
    ~particle_system() override = default;

    signal<particle_event const> ParticleUpdate;

    prop<assets::asset_ptr<material>> Material;
    point_f                           Position;

    void start();
    void restart();
    void stop();

    template <typename T = particle_emitter>
    auto create_emitter(auto&&... args) -> std::shared_ptr<T>;
    template <typename T = particle_emitter>
    void add_emitter(std::shared_ptr<T> const& emitter);

    void remove_emitter(particle_emitter_base const& emitter);
    void clear_emitters();

    auto get_particle_count() const -> isize;

    auto activate_particle() -> particle&;
    void deactivate_particle(particle& particle);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    bool                                                _isRunning {false};
    quad_renderer                                       _renderer {buffer_usage_hint::DynamicDraw};
    std::vector<particle>                               _particles {};
    isize                                               _aliveParticleCount {0};
    std::vector<std::shared_ptr<particle_emitter_base>> _emitters {};

    std::mutex _mutex {};
    bool       _multiThreaded;
};

}

#include "ParticleSystem.inl"
