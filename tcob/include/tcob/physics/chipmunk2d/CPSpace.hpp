// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/core/Signal.hpp"
    #include "tcob/physics/chipmunk2d/CP.hpp"
    #include "tcob/physics/chipmunk2d/CPBody.hpp"
    #include "tcob/physics/chipmunk2d/CPConstraint.hpp"
    #include "tcob/physics/chipmunk2d/CPShape.hpp"

namespace tcob::physics::chipmunk2d {
////////////////////////////////////////////////////////////

struct contact_event {
    std::shared_ptr<body> BodyA;
    std::shared_ptr<body> BodyB;

    std::shared_ptr<shape> ShapeA;
    std::shared_ptr<shape> ShapeB;

    bool IsFirstContact;
    bool IsRemoval;
};

////////////////////////////////////////////////////////////

class TCOB_API space final : public updatable, public non_copyable {
    friend auto detail::get_impl(space const*) -> cpSpace*;
    friend auto detail::find_body(space*, cpBody*) -> std::shared_ptr<body>;

public:
    space();
    ~space() override;

    signal<contact_event const> BeginContact;
    signal<contact_event const> EndContact;

    prop_fn<i32>     Iterations;
    prop_fn<point_f> Gravity;
    prop_fn<f32>     Damping;
    prop_fn<f32>     IdleSpeedThreshold;
    prop_fn<f32>     SleepTimeThreshold;
    prop_fn<f32>     CollisionSlop;
    prop_fn<f32>     CollisionBias;
    prop_fn<u32>     CollisionPersistence;

    auto get_current_time_step() const -> f32;
    auto is_locked() const -> bool;

    auto create_body() -> std::shared_ptr<body>;
    void remove_body(std::shared_ptr<body> const& bodyPtr);

    auto create_constraint(pin_joint_settings const& settings) -> std::shared_ptr<pin_joint>;
    auto create_constraint(slide_joint_settings const& settings) -> std::shared_ptr<slide_joint>;
    auto create_constraint(pivot_joint_settings const& settings) -> std::shared_ptr<pivot_joint>;
    auto create_constraint(pivot_joint_settings2 const& settings) -> std::shared_ptr<pivot_joint>;
    auto create_constraint(groove_joint_settings const& settings) -> std::shared_ptr<groove_joint>;
    auto create_constraint(damped_spring_settings const& settings) -> std::shared_ptr<damped_spring>;
    auto create_constraint(damped_rotary_spring_settings const& settings) -> std::shared_ptr<damped_rotary_spring>;
    auto create_constraint(rotary_limit_joint_settings const& settings) -> std::shared_ptr<rotary_limit_joint>;
    auto create_constraint(ratchet_joint_settings const& settings) -> std::shared_ptr<ratchet_joint>;
    auto create_constraint(gear_joint_settings const& settings) -> std::shared_ptr<gear_joint>;
    auto create_constraint(simple_motor_settings const& settings) -> std::shared_ptr<simple_motor>;
    void remove_constraint(std::shared_ptr<constraint> const& constraintPtr);

private:
    auto find_body(cpBody* cpbody) -> std::shared_ptr<body>;

    void on_update(milliseconds deltaTime) override;

    std::vector<std::shared_ptr<body>>       _bodies;
    std::vector<std::shared_ptr<constraint>> _constraints;
    cpSpace*                                 _cpSpace;
};

}

#endif
