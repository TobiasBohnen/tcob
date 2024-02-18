// Copyright (c) 2023 Tobias Bohnen
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

    f64 Speed {0.};
    f32 Acceleration {0.0f};

    degree_f Direction {0.0f};
    degree_f Spin {0.0f};
    degree_f Rotation {0};

    color  Color {colors::White};
    rect_f Bounds {rect_f::Zero};
    size_f Scale {size_f::One};

    auto get_lifetime_ratio() const -> f32;
    auto is_alive() const -> bool;

    void set_lifetime(milliseconds life);

    void show();
    void hide();
    auto is_visible() const -> bool;

    void set_texture_region(texture_region const& texRegion);

    void to_quad(quad* quad);

    void update(milliseconds deltaTime);

private:
    texture_region _region {};
    transform      _transform {};
    bool           _visible {true};
    milliseconds   _startingLife {0};
    milliseconds   _remainingLife {0};
};

////////////////////////////////////////////////////////////

struct particle_template {
    std::pair<f32, f32>                   Acceleration;
    std::pair<degree_f, degree_f>         Direction;
    std::pair<milliseconds, milliseconds> Lifetime;
    std::pair<f32, f32>                   Scale;
    size_f                                Size;
    std::pair<f32, f32>                   Speed;
    std::pair<degree_f, degree_f>         Spin;
    string                                Texture;
    std::pair<f32, f32>                   Transparency;
};

class TCOB_API particle_emitter final {
public:
    particle_emitter();

    particle_template Template;
    rect_f            SpawnArea {rect_f::Zero};
    f32               SpawnRate {0};
    bool              IsLooping {false};
    milliseconds      Lifetime {1000};

    auto is_alive() const -> bool;

    void reset();

    void emit_particles(particle_system& system, milliseconds time);

private:
    rng          _randomGen;
    milliseconds _remainLife {1000};
    f64          _emissionDiff {0};
};

////////////////////////////////////////////////////////////

class TCOB_API particle_system final : public drawable {
public:
    particle_system();
    ~particle_system() override = default;

    signal<particle> ParticleSpawn;
    signal<particle> ParticleDeath;
    signal<particle> ParticleUpdate;

    prop<assets::asset_ptr<material>> Material;
    point_f                           Position;

    auto get_particle_count() const -> isize;

    void start();
    void restart();
    void stop();

    auto create_emitter() -> particle_emitter&; // TODO: change to shared_ptr
    void remove_all_emitters();

    auto activate_particle() -> particle&;

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    void deactivate_particle(particle& particle);

    bool                          _isRunning {false};
    quad_renderer                 _renderer {buffer_usage_hint::DynamicDraw};
    std::vector<particle>         _particles {};
    isize                         _aliveParticleCount {0};
    std::vector<particle_emitter> _emitters {};
};
}
