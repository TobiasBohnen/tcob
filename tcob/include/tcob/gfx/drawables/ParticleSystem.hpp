// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <mutex>
#include <optional>
#include <tuple>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Serialization.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API particle final {
public:
    ////////////////////////////////////////////////////////////

    class settings {
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

        static auto constexpr Members()
        {
            return std::tuple {
                member<&particle::settings::Speed> {"speed"},
                member<&particle::settings::Direction> {"direction"},

                member<&particle::settings::LinearAcceleration> {"linear_acceleration"},
                member<&particle::settings::LinearDamping> {"linear_dampling"},
                member<&particle::settings::RadialAcceleration> {"radial_acceleration"},
                member<&particle::settings::TangentialAcceleration> {"tangential_acceleration"},

                member<&particle::settings::Gravity> {"gravity"},

                member<&particle::settings::TextureRegion> {"texture_region"},
                member<&particle::settings::Colors> {"colors"},
                member<&particle::settings::Transparency> {"transparency"},

                member<&particle::settings::Lifetime> {"lifetime"},
            };
        }
    };

    ////////////////////////////////////////////////////////////

    uid ID {};

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

    size_f Scale {size_f::One};
    rect_f Bounds {rect_f::Zero};

    degree_f Spin {0.0f};
    degree_f Rotation {0.0f};

    point_f Position {point_f::Zero};
    point_f Origin {point_f::Zero};

    auto is_alive() const -> bool;

    void update(milliseconds deltaTime);

    void init(settings const& tmpl, texture_region const& texRegion, rect_f const& spawnArea, rng& rng);

    void convert_to(quad* quad) const;

private:
    transform _transform {};
};

////////////////////////////////////////////////////////////

struct particle_event {
    particle&    Particle;
    milliseconds DeltaTime;
};

////////////////////////////////////////////////////////////

class TCOB_API particle_emitter final : public non_copyable {
public:
    ////////////////////////////////////////////////////////////

    class settings {
    public:
        typename particle::settings Template;

        bool                        IsExplosion {false};
        rect_f                      SpawnArea {rect_f::Zero};
        f32                         SpawnRate {0};
        std::optional<milliseconds> Lifetime {};

        auto operator==(settings const& other) const -> bool = default;

        static auto constexpr Members()
        {
            return std::tuple {
                member<&particle_emitter::settings::Template> {"template"},
                member<&particle_emitter::settings::SpawnArea> {"spawn_area"},
                member<&particle_emitter::settings::SpawnRate> {"spawn_rate"},
                member<&particle_emitter::settings::IsExplosion> {"is_explosion"},
                member<&particle_emitter::settings::Lifetime, std::nullopt> {"lifetime"},
            };
        }
    };

    ////////////////////////////////////////////////////////////

    settings Settings;

    auto is_alive() const -> bool;

    void reset();

    void emit(particle_system& system, milliseconds deltaTime);

private:
    rng          _rng;
    milliseconds _remainingLife {1000};
    f64          _emissionDiff {0};
    bool         _alive {true};
};

////////////////////////////////////////////////////////////

class TCOB_API particle_system final : public drawable, public updatable {
public:
    explicit particle_system(bool multiThreaded = false, isize reservedParticleCount = 0);

    ~particle_system() override = default;

    signal<particle_event> ParticleUpdate;

    prop<asset_ptr<material>> Material;

    auto is_running() const -> bool;

    void start();
    void restart();
    void stop();

    auto create_emitter() -> particle_emitter&;
    auto remove_emitter(particle_emitter const& emitter) -> bool;
    void clear();

    auto particle_count() const -> isize;

    auto activate_particle() -> particle&;
    void deactivate_particle(particle& particle);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;

    void on_draw_to(render_target& target, transform const& xform) override;

private:
    renderer          _renderer {buffer_usage_hint::DynamicDraw};
    std::vector<quad> _geometry;

    std::vector<std::unique_ptr<particle_emitter>> _emitters {};
    std::vector<particle>                          _particles {};
    isize                                          _aliveParticleCount {0};

    std::mutex _mutex {};
    bool       _multiThreaded;

    bool _isRunning {false};
};

////////////////////////////////////////////////////////////

}
