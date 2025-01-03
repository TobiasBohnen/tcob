// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <mutex>
#include <utility>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

template <typename T>
struct particle_event {
    T&           Particle;
    milliseconds DeltaTime;
};

////////////////////////////////////////////////////////////

template <typename Emitter>
class particle_system final : public drawable, public updatable {
    ////////////////////////////////////////////////////////////

    using particle_type = typename Emitter::particle_type;
    using emitter_type  = Emitter;
    using geometry_type = typename Emitter::geometry_type;
    using renderer_type = typename Emitter::renderer_type;

    static_assert(requires(particle_type p, geometry_type* q, milliseconds deltaTime) {
        { p.RemainingLife } -> std::same_as<milliseconds&>;
        { p.convert_to(q) };
        { p.update(deltaTime) }; }, "Invalid particle type");

    static_assert(requires(Emitter t, particle_system<Emitter>& system, milliseconds deltaTime) {
        { t.reset() };
        { t.emit(system, deltaTime) }; }, "Invalid emitter type");

    ////////////////////////////////////////////////////////////

public:
    explicit particle_system(bool multiThreaded = false);

    ~particle_system() override = default;

    signal<particle_event<particle_type> const> ParticleUpdate;

    prop<assets::asset_ptr<material>> Material;

    auto is_running() const -> bool;

    void start();
    void restart();
    void stop();

    auto create_emitter(auto&&... args) -> std::shared_ptr<Emitter>;
    void add_emitter(std::shared_ptr<Emitter> const& emitter);

    void remove_emitter(emitter_type const& emitter);
    void clear_emitters();

    auto particle_count() const -> isize;

    auto activate_particle() -> particle_type&;
    void deactivate_particle(particle_type& particle);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;

    void on_draw_to(render_target& target) override;

private:
    renderer_type              _renderer {buffer_usage_hint::DynamicDraw};
    std::vector<geometry_type> _geometry;

    std::vector<std::shared_ptr<emitter_type>> _emitters {};
    std::vector<particle_type>                 _particles {};
    isize                                      _aliveParticleCount {0};

    std::mutex _mutex {};
    bool       _multiThreaded;

    bool _isRunning {false};
};

////////////////////////////////////////////////////////////

class TCOB_API quad_particle final {
public:
    ////////////////////////////////////////////////////////////

    struct settings {
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

        auto operator==(settings const& other) const -> bool = default;
    };

    ////////////////////////////////////////////////////////////

    std::any UserData;

    size_f Scale {size_f::One};
    rect_f Bounds {rect_f::Zero};

    degree_f Spin {0.0f};
    degree_f Rotation {0};

    f32      Acceleration {0.0f};
    f32      Speed {0.0};
    degree_f Direction {0.0f};

    color Color {colors::White};

    milliseconds StartingLife {0};
    milliseconds RemainingLife {0};

    texture_region Region {};

    void convert_to(quad* quad) const;

    void update(milliseconds deltaTime);

private:
    transform _transform {};
};

////////////////////////////////////////////////////////////

class TCOB_API quad_particle_emitter final {
public:
    ////////////////////////////////////////////////////////////

    struct settings {
        quad_particle::settings Template;

        rect_f                      SpawnArea {rect_f::Zero};
        f32                         SpawnRate {0};
        std::optional<milliseconds> Lifetime {};

        auto operator==(settings const& other) const -> bool = default;
    };

    ////////////////////////////////////////////////////////////

    using particle_type = quad_particle;
    using geometry_type = quad;
    using renderer_type = quad_renderer;

    settings Settings;

    auto is_alive() const -> bool;

    void reset();

    void emit(particle_system<quad_particle_emitter>& system, milliseconds deltaTime);

private:
    rng          _randomGen;
    milliseconds _remainingLife {1000};
    f64          _emissionDiff {0};
};

////////////////////////////////////////////////////////////

class TCOB_API point_particle final {
public:
    ////////////////////////////////////////////////////////////

    struct settings {
        std::pair<f32, f32>           Acceleration;
        std::pair<f32, f32>           Speed;
        std::pair<degree_f, degree_f> Direction;

        string              Texture;
        color               Color;
        std::pair<f32, f32> Transparency;

        std::pair<milliseconds, milliseconds> Lifetime;

        auto operator==(settings const& other) const -> bool = default;
    };

    ////////////////////////////////////////////////////////////

    std::any UserData;

    point_f Position {point_f::Zero};

    f32      Acceleration {0.0f};
    f32      Speed {0.0};
    degree_f Direction {0.0f};

    color Color {colors::White};

    milliseconds StartingLife {0};
    milliseconds RemainingLife {0};

    texture_region Region {};

    void convert_to(vertex* vertex) const;

    void update(milliseconds deltaTime);
};

////////////////////////////////////////////////////////////

class TCOB_API point_particle_emitter final {
public:
    ////////////////////////////////////////////////////////////

    struct settings {
        point_particle::settings Template;

        rect_f                      SpawnArea {rect_f::Zero};
        f32                         SpawnRate {0};
        std::optional<milliseconds> Lifetime {};

        auto operator==(settings const& other) const -> bool = default;
    };

    ////////////////////////////////////////////////////////////

    using particle_type = point_particle;
    using geometry_type = vertex;
    using renderer_type = point_renderer;

    settings Settings;

    auto is_alive() const -> bool;

    void reset();

    void emit(particle_system<point_particle_emitter>& system, milliseconds deltaTime);

private:
    rng          _randomGen;
    milliseconds _remainingLife {1000};
    f64          _emissionDiff {0};
};

////////////////////////////////////////////////////////////

using quad_particle_system  = particle_system<quad_particle_emitter>;
using point_particle_system = particle_system<point_particle_emitter>;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void Serialize(quad_particle::settings const& v, auto&& s)
{
    s["acceleration"] = v.Acceleration;
    s["direction"]    = v.Direction;
    s["lifetime"]     = v.Lifetime;
    s["scale"]        = v.Scale;
    s["size"]         = v.Size;
    s["speed"]        = v.Speed;
    s["spin"]         = v.Spin;
    s["rotation"]     = v.Rotation;
    s["texture"]      = v.Texture;
    s["color"]        = v.Color;
    s["transparency"] = v.Transparency;
}

auto Deserialize(quad_particle::settings& v, auto&& s) -> bool
{
    return s.try_get(v.Acceleration, "acceleration")
        && s.try_get(v.Direction, "direction")
        && s.try_get(v.Lifetime, "lifetime")
        && s.try_get(v.Scale, "scale")
        && s.try_get(v.Size, "size")
        && s.try_get(v.Speed, "speed")
        && s.try_get(v.Spin, "spin")
        && s.try_get(v.Rotation, "rotation")
        && s.try_get(v.Texture, "texture")
        && s.try_get(v.Color, "color")
        && s.try_get(v.Transparency, "transparency");
}
void Serialize(quad_particle_emitter::settings const& v, auto&& s)
{
    s["template"]   = v.Template;
    s["spawn_area"] = v.SpawnArea;
    s["spawn_rate"] = v.SpawnRate;
    if (v.Lifetime) {
        s["lifetime"] = *v.Lifetime;
    }
}

auto Deserialize(quad_particle_emitter::settings& v, auto&& s) -> bool
{
    if (s.has("lifetime")) {
        v.Lifetime = s["lifetime"].template as<milliseconds>();
    }
    return s.try_get(v.Template, "template")
        && s.try_get(v.SpawnArea, "spawn_area")
        && s.try_get(v.SpawnRate, "spawn_rate");
}

void Serialize(point_particle::settings const& v, auto&& s)
{
    s["acceleration"] = v.Acceleration;
    s["direction"]    = v.Direction;
    s["lifetime"]     = v.Lifetime;
    s["speed"]        = v.Speed;
    s["texture"]      = v.Texture;
    s["color"]        = v.Color;
    s["transparency"] = v.Transparency;
}

auto Deserialize(point_particle::settings& v, auto&& s) -> bool
{
    return s.try_get(v.Acceleration, "acceleration")
        && s.try_get(v.Direction, "direction")
        && s.try_get(v.Lifetime, "lifetime")
        && s.try_get(v.Speed, "speed")
        && s.try_get(v.Texture, "texture")
        && s.try_get(v.Color, "color")
        && s.try_get(v.Transparency, "transparency");
}
void Serialize(point_particle_emitter::settings const& v, auto&& s)
{
    s["template"]   = v.Template;
    s["spawn_area"] = v.SpawnArea;
    s["spawn_rate"] = v.SpawnRate;
    if (v.Lifetime) {
        s["lifetime"] = *v.Lifetime;
    }
}

auto Deserialize(point_particle_emitter::settings& v, auto&& s) -> bool
{
    if (s.has("lifetime")) {
        v.Lifetime = s["lifetime"].template as<milliseconds>();
    }
    return s.try_get(v.Template, "template")
        && s.try_get(v.SpawnArea, "spawn_area")
        && s.try_get(v.SpawnRate, "spawn_rate");
}

}

#include "ParticleSystem.inl"
