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
#include <variant>
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

class particle_emitter;
class particle_system;
class parallax_background;

////////////////////////////////////////////////////////////

class TCOB_API particle_settings final {

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

    min_max<f32> Scale {1.f, 1.f};
    size_f       Size {size_f::One};

    min_max<degree_f> Spin;
    min_max<degree_f> Rotation;

    auto operator==(particle_settings const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&particle_settings::Speed> {"speed"},
            member<&particle_settings::Direction> {"direction"},

            member<&particle_settings::LinearAcceleration> {"linear_acceleration"},
            member<&particle_settings::LinearDamping> {"linear_dampling"},
            member<&particle_settings::RadialAcceleration> {"radial_acceleration"},
            member<&particle_settings::TangentialAcceleration> {"tangential_acceleration"},

            member<&particle_settings::Gravity> {"gravity"},

            member<&particle_settings::TextureRegion> {"texture_region"},
            member<&particle_settings::Colors> {"colors"},
            member<&particle_settings::Transparency> {"transparency"},

            member<&particle_settings::Lifetime> {"lifetime"},
        };
    }
};

////////////////////////////////////////////////////////////

class TCOB_API particle_emitter final : public non_copyable {
public:
    ////////////////////////////////////////////////////////////

    class emit_linear {
    public:
        f32 Rate {0};

        auto operator==(emit_linear const& other) const -> bool = default;

        static auto constexpr Members() { return std::tuple {member<&particle_emitter::emit_linear::Rate> {"rate"}}; }
    };

    class emit_burst {
    public:
        f32          Count {0};
        milliseconds Interval {0};
        i32          Repeats {1};

        auto operator==(emit_burst const& other) const -> bool = default;

        static auto constexpr Members() { return std::tuple {
            member<&particle_emitter::emit_burst::Count> {"count"},
            member<&particle_emitter::emit_burst::Interval> {"interval"},
            member<&particle_emitter::emit_burst::Repeats> {"repeats"},
        }; }
    };

    using emit_pattern = std::variant<emit_linear, emit_burst>;

    class settings {
    public:
        particle_settings Template;

        bool Transient {false};

        emit_pattern Pattern;
        rect_f       SpawnArea {rect_f::Zero};

        std::optional<milliseconds> Lifetime {};

        auto operator==(settings const& other) const -> bool = default;

        static auto constexpr Members()
        {
            return std::tuple {
                member<&particle_emitter::settings::Template> {"template"},
                member<&particle_emitter::settings::Transient> {"transient"},
                member<&particle_emitter::settings::Pattern> {"pattern"},
                member<&particle_emitter::settings::SpawnArea> {"spawn_area"},
                member<&particle_emitter::settings::Lifetime, std::nullopt> {"lifetime"},
            };
        }
    };

    ////////////////////////////////////////////////////////////

    explicit particle_emitter(uid id);

    settings Settings;

    auto is_alive() const -> bool;
    auto id() const -> uid;

    void reset();

    void emit(particle_system& system, milliseconds deltaTime);

private:
    uid _id;

    rng          _rng;
    milliseconds _remainingLife {1000};
    bool         _alive {true};

    // linear
    f64 _linearDiff {0};

    // burst
    milliseconds _burstTimer {0};
    i32          _burstRepeatsLeft {0};
};

////////////////////////////////////////////////////////////

class TCOB_API particles final {
public:
    // hot
    std::vector<milliseconds> RemainingLife;
    std::vector<point_f>      Velocity;
    std::vector<point_f>      LinearAcceleration;
    std::vector<f32>          LinearDamping;
    std::vector<f32>          RadialAcceleration;
    std::vector<f32>          TangentialAcceleration;
    std::vector<point_f>      Gravity;
    std::vector<rect_f>       Bounds;
    std::vector<f32>          Rotation;
    std::vector<f32>          Spin;
    std::vector<size_f>       Scale;
    std::vector<point_f>      Origin;

    // cold
    std::vector<milliseconds>   StartingLife;
    std::vector<color>          Color;
    std::vector<texture_region> UVRegion;
    std::vector<uid>            EmitterID;
    std::vector<uid>            ID;

    void reserve(isize count);
    void push_back();
    void swap(isize a, isize b);
};

////////////////////////////////////////////////////////////

struct particle_event {
    particles&   Particles;
    isize        Index;
    milliseconds DeltaTime;
};

class TCOB_API particle_system final : public drawable, public updatable {
    friend class particle_emitter;

public:
    explicit particle_system(bool multiThreaded = false, isize reservedParticleCount = 0);

    ~particle_system() override = default;

    signal<particle_event const, particle_system> ParticleUpdate;
    signal<particle_event const, particle_system> ParticleDeath;

    particles Particles;

    prop<asset_ptr<material>> Material;

    auto is_running() const -> bool;

    void start();
    void restart();
    void stop();

    auto create_emitter() -> particle_emitter&;
    auto remove_emitter(particle_emitter const& emitter) -> bool;
    void clear();

    auto particle_count() const -> isize;

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;

    void on_draw_to(render_target& target, transform const& xform) override;

private:
    auto activate_particle() -> isize;
    void deactivate_particle(isize index);

    void reserve(isize count);

    renderer          _renderer {buffer_usage_hint::DynamicDraw};
    std::vector<quad> _geometry;
    std::vector<u32>  _indices {};

    std::vector<std::unique_ptr<particle_emitter>> _emitters {};
    isize                                          _aliveParticleCount {0};

    std::mutex _mutex {};
    bool       _multiThreaded;

    bool _isRunning {false};
};

////////////////////////////////////////////////////////////

}
