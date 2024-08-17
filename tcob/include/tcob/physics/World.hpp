// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <span>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/DebugDraw.hpp"
    #include "tcob/physics/Joint.hpp"
    #include "tcob/physics/Shape.hpp"

namespace tcob::physics {

////////////////////////////////////////////////////////////

struct body_move_event {
    body_transform Transform;
    body*          Body {nullptr};
    bool           FellAsleep {};
};

struct body_events {
    std::vector<body_move_event> Move;
};

struct contact_begin_touch_event {
    shape* ShapeA {};
    shape* ShapeB {};
};

struct contact_end_touch_event {
    shape* ShapeA {};
    shape* ShapeB {};
};

struct contact_hit_event {
    shape*  ShapeA {};
    shape*  ShapeB {};
    point_f Point {};
    point_f Normal {};
    f32     ApproachSpeed {};
};

struct contact_events {
    std::vector<contact_begin_touch_event> BeginTouch;
    std::vector<contact_end_touch_event>   EndTouch;
    std::vector<contact_hit_event>         Hit;
};

struct sensor_begin_touch_event {
    shape* Sensor {};
    shape* Visitor {};
};

struct sensor_end_touch_event {
    shape* Sensor {};
    shape* Visitor {};
};

struct sensor_events {
    std::vector<sensor_begin_touch_event> BeginTouch;
    std::vector<sensor_end_touch_event>   EndTouch;
};

////////////////////////////////////////////////////////////

class TCOB_API world final : public updatable, public non_copyable {
public:
    class TCOB_API settings {
    public:
        /// Gravity vector.
        point_f Gravity {0, 10.f};

        /// Restitution velocity threshold, usually in m/s. Collisions above this
        /// speed have restitution applied (will bounce).
        f32 RestitutionThreshold {1.0f};

        /// This parameter controls how fast overlap is resolved and has units of meters per second
        f32 ContactPushoutVelocity {3.0f};

        /// Threshold velocity for hit events. Usually meters per second.
        f32 HitEventThreshold {1.0f};

        /// Contact stiffness. Cycles per second.
        f32 ContactHertz {30};

        /// Contact bounciness. Non-dimensional.
        f32 ContactDampingRatio {10};

        /// Joint stiffness. Cycles per second.
        f32 JointHertz {60};

        /// Joint bounciness. Non-dimensional.
        f32 JointDampingRatio {2.0f};

        /// Maximum linear velocity. Usually meters per second.
        f32 MaximumLinearVelocity {400.0f};

        /// Can bodies go to sleep to improve performance
        bool EnableSleeping {true};

        /// Enable continuous collision
        bool EnableContinuous {true};
    };

    world();
    explicit world(settings const& settings);
    ~world() override;

    i32              SubSteps {4};
    prop_fn<point_f> Gravity;
    prop<bool>       EnableSleeping;
    prop<bool>       EnableContinuous;
    prop<f32>        RestitutionThreshold;
    prop<f32>        HitEventThreshold;

    auto get_bodies() -> std::span<std::shared_ptr<body>>;

    auto create_body(body_transform const& xform, body::settings const& settings) -> std::shared_ptr<body>;
    void remove_body(body const& body);

    auto find_body(shape const& s) -> std::shared_ptr<body>;

    auto get_joints() -> std::span<std::shared_ptr<joint>>;

    template <typename T>
    auto create_joint(auto&& jointSettings) -> std::shared_ptr<T>;
    void remove_joint(joint const& joint);

    auto get_body_events() const -> body_events;
    auto get_contact_events() const -> contact_events;
    auto get_sensor_events() const -> sensor_events;

    void draw(debug_draw const& draw) const;

    void explode(point_f pos, f32 radius, f32 impulse) const;

private:
    void on_update(milliseconds deltaTime) override;

    std::unique_ptr<detail::b2d_world> _impl;

    std::vector<std::shared_ptr<body>>  _bodies;
    std::vector<std::shared_ptr<joint>> _joints;
};

template <typename T>
inline auto world::create_joint(auto&& jointSettings) -> std::shared_ptr<T>
{
    return std::static_pointer_cast<T>(_joints.emplace_back(std::shared_ptr<T> {new T {*this, _impl.get(), jointSettings}}));
}

}

#endif
