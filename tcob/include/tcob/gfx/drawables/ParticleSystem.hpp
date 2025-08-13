// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/Transform.hpp"
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
        { p.is_alive() } -> std::same_as<bool>;
        { p.convert_to(q) };
        { p.update(deltaTime) }; }, "Invalid particle type");

    static_assert(requires(Emitter t, particle_system<Emitter>& system, milliseconds deltaTime) {
        { t.reset() };
        { t.emit(system, deltaTime) }; }, "Invalid emitter type");

    ////////////////////////////////////////////////////////////

public:
    explicit particle_system(bool multiThreaded = false, isize reservedParticleCount = 0);

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

template <typename T>
using min_max = std::pair<T, T>;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API particle_base {
public:
    std::any UserData;

    point_f Velocity {point_f::Zero};
    point_f LinearAcceleration {point_f::Zero};
    f32     LinearDamping {0.0f};
    f32     RadialAcceleration {0.0f};
    f32     TangentialAcceleration {0.0f};
    point_f Gravity {point_f::Zero};

    milliseconds StartingLife {0};
    milliseconds RemainingLife {0};

    color          Color {colors::White};
    texture_region Region {};

    auto is_alive() const -> bool;
};

////////////////////////////////////////////////////////////

class TCOB_API point_particle final : public particle_base {
public:
    ////////////////////////////////////////////////////////////

    class TCOB_API settings {
    public:
        min_max<f32>      Speed;
        min_max<degree_f> Direction;

        min_max<f32> LinearAcceleration;
        min_max<f32> LinearDamping;
        min_max<f32> RadialAcceleration;
        min_max<f32> TangentialAcceleration;

        min_max<point_f> Gravity;

        string             TextureRegion;
        std::vector<color> Colors;
        min_max<f32>       Transparency;

        min_max<milliseconds> Lifetime;

        auto operator==(settings const& other) const -> bool = default;

        auto constexpr members(this auto&& self);
    };

    ////////////////////////////////////////////////////////////

    point_f Position {point_f::Zero};
    point_f Origin {point_f::Zero};

    void convert_to(vertex* vertex) const;

    void update(milliseconds deltaTime);
};

////////////////////////////////////////////////////////////

class TCOB_API quad_particle final : public particle_base {
public:
    ////////////////////////////////////////////////////////////

    class TCOB_API settings {
    public:
        min_max<f32>      Speed;
        min_max<degree_f> Direction;

        min_max<f32> LinearAcceleration;
        min_max<f32> LinearDamping;
        min_max<f32> RadialAcceleration;
        min_max<f32> TangentialAcceleration;

        min_max<point_f> Gravity;

        string             TextureRegion;
        std::vector<color> Colors;
        min_max<f32>       Transparency;

        min_max<milliseconds> Lifetime;

        min_max<f32> Scale;
        size_f       Size;

        min_max<degree_f> Spin;
        min_max<degree_f> Rotation;

        auto operator==(settings const& other) const -> bool = default;

        auto constexpr members(this auto&& self);
    };
    ////////////////////////////////////////////////////////////

    size_f  Scale {size_f::One};
    rect_f  Bounds {rect_f::Zero};
    point_f Origin {point_f::Zero};

    degree_f Spin {0.0f};
    degree_f Rotation {0.0f};

    void convert_to(quad* quad) const;

    void update(milliseconds deltaTime);

private:
    transform _transform {};
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API point_particle_emitter final {
public:
    ////////////////////////////////////////////////////////////

    class TCOB_API settings {
    public:
        point_particle::settings Template;

        bool                        IsExplosion {false};
        rect_f                      SpawnArea {rect_f::Zero};
        f32                         SpawnRate {0};
        std::optional<milliseconds> Lifetime {};

        auto operator==(settings const& other) const -> bool = default;

        void static Serialize(point_particle_emitter::settings const& v, auto&& s);
        auto static Deserialize(point_particle_emitter::settings& v, auto&& s) -> bool;
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
    rng          _rng;
    milliseconds _remainingLife {1000};
    f64          _emissionDiff {0};
    bool         _alive {true};
};

////////////////////////////////////////////////////////////

class TCOB_API quad_particle_emitter final {
public:
    ////////////////////////////////////////////////////////////

    class TCOB_API settings {
    public:
        quad_particle::settings Template;

        bool                        IsExplosion {false};
        rect_f                      SpawnArea {rect_f::Zero};
        f32                         SpawnRate {0};
        std::optional<milliseconds> Lifetime {};

        auto operator==(settings const& other) const -> bool = default;

        void static Serialize(quad_particle_emitter::settings const& v, auto&& s);
        auto static Deserialize(quad_particle_emitter::settings& v, auto&& s) -> bool;
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
    bool         _alive {true};
};

////////////////////////////////////////////////////////////

using point_particle_system = particle_system<point_particle_emitter>;
using quad_particle_system  = particle_system<quad_particle_emitter>;

////////////////////////////////////////////////////////////

}

#include "ParticleSystem.inl"
