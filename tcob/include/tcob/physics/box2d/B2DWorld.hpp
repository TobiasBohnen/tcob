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
    #include "tcob/core/Signal.hpp"
    #include "tcob/physics/box2d/B2D.hpp"
    #include "tcob/physics/box2d/B2DBody.hpp"
    #include "tcob/physics/box2d/B2DFixture.hpp"
    #include "tcob/physics/box2d/B2DJoint.hpp"

namespace tcob::physics::box2d {
////////////////////////////////////////////////////////////

struct iterations {
    i32 Velocity {6};
    i32 Position {2};
};

////////////////////////////////////////////////////////////

struct contact_event {
    std::shared_ptr<fixture> FixtureA;
    std::shared_ptr<fixture> FixtureB;
    bool                     IsTouching;
    bool                     Enabled;
    f32                      Friction;
    f32                      Restitution;
};

////////////////////////////////////////////////////////////

class TCOB_API world final : public updatable, public non_copyable {
    friend class detail::contact_listener;
    friend auto operator==(world const& left, world const& right) -> bool;

public:
    world();
    ~world() override;

    signal<contact_event&> BeginContact;
    signal<contact_event&> EndContact;
    signal<contact_event&> PreSolve;
    signal<contact_event&> PostSolve;

    iterations    Iterations;
    prop<point_f> Gravity;
    prop<bool>    AllowSleeping;

    auto get_bodies() -> std::span<std::shared_ptr<body>>;
    auto get_joints() -> std::span<std::shared_ptr<joint>>;
    auto is_locked() const -> bool;

    auto create_body(body_transform const& xform, body_settings const& settings) -> std::shared_ptr<body>;
    void destroy_body(std::shared_ptr<body> const& bodyPtr);

    auto create_joint(distance_joint_settings const& joint) -> std::shared_ptr<distance_joint>;
    auto create_joint(friction_joint_settings const& joint) -> std::shared_ptr<friction_joint>;
    auto create_joint(gear_joint_settings const& joint) -> std::shared_ptr<gear_joint>;
    auto create_joint(motor_joint_settings const& joint) -> std::shared_ptr<motor_joint>;
    auto create_joint(mouse_joint_settings const& joint) -> std::shared_ptr<mouse_joint>;
    auto create_joint(prismatic_joint_settings const& joint) -> std::shared_ptr<prismatic_joint>;
    auto create_joint(pulley_joint_settings const& joint) -> std::shared_ptr<pulley_joint>;
    auto create_joint(revolute_joint_settings const& joint) -> std::shared_ptr<revolute_joint>;
    auto create_joint(weld_joint_settings const& joint) -> std::shared_ptr<weld_joint>;
    auto create_joint(wheel_joint_settings const& joint) -> std::shared_ptr<wheel_joint>;
    void destroy_joint(std::shared_ptr<joint> const& jointPtr);

    void do_debug_draw(debug_draw const& draw);

private:
    void on_update(milliseconds deltaTime) override;

    auto find_fixture(b2Fixture* b2fixture) -> std::shared_ptr<fixture>;

    std::unique_ptr<b2World>                  _b2World;
    std::vector<std::shared_ptr<body>>        _bodies;
    std::vector<std::shared_ptr<joint>>       _joints;
    std::unique_ptr<detail::contact_listener> _listener;
};

inline auto operator==(world const& left, world const& right) -> bool
{
    return left._b2World.get() == right._b2World.get();
}

}

#endif
