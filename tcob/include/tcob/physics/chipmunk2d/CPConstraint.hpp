// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/physics/chipmunk2d/CP.hpp"

namespace tcob::physics::chipmunk2d {
////////////////////////////////////////////////////////////

class TCOB_API constraint : public non_copyable {
    friend auto detail::get_impl(constraint const* b) -> cpConstraint*;

public:
    ~constraint();

protected:
    constraint(cpConstraint* constraint, space* parent);

private:
    cpConstraint* _cpConstraint;
    space*        _space;
};

////////////////////////////////////////////////////////////

struct pin_joint_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    point_f               AnchorA;
    point_f               AnchorB;
};

class TCOB_API pin_joint final : public constraint {
    friend class space;

private:
    pin_joint(cpBody* a, cpBody* b, point_f anchorA, point_f anchorB, space* parent);
};

////////////////////////////////////////////////////////////

struct slide_joint_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    point_f               AnchorA;
    point_f               AnchorB;
    f32                   Min;
    f32                   Max;
};

class TCOB_API slide_joint final : public constraint {
    friend class space;

private:
    slide_joint(cpBody* a, cpBody* b, point_f anchorA, point_f anchorB, f32 min, f32 max, space* parent);
};

////////////////////////////////////////////////////////////

struct pivot_joint_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    point_f               Pivot;
};

struct pivot_joint_settings2 {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    point_f               AnchorA;
    point_f               AnchorB;
};

class TCOB_API pivot_joint final : public constraint {
    friend class space;

private:
    pivot_joint(cpBody* a, cpBody* b, point_f pivot, space* parent);
    pivot_joint(cpBody* a, cpBody* b, point_f anchorA, point_f anchorB, space* parent);
};

////////////////////////////////////////////////////////////

struct groove_joint_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    point_f               GrooveA;
    point_f               GrooveB;
    point_f               AnchorB;
};

class TCOB_API groove_joint final : public constraint {
    friend class space;

private:
    groove_joint(cpBody* a, cpBody* b, point_f grooveA, point_f grooveB, point_f anchorB, space* parent);
};

////////////////////////////////////////////////////////////

struct damped_spring_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    point_f               AnchorA;
    point_f               AnchorB;
    f32                   RestLength;
    f32                   Stiffness;
    f32                   Damping;
};

class TCOB_API damped_spring final : public constraint {
    friend class space;

private:
    damped_spring(cpBody* a, cpBody* b, point_f anchorA, point_f anchorB, f32 restLength, f32 stiffness, f32 damping, space* parent);
};

////////////////////////////////////////////////////////////

struct damped_rotary_spring_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    f32                   RestAngle;
    f32                   Stiffness;
    f32                   Damping;
};

class TCOB_API damped_rotary_spring final : public constraint {
    friend class space;

private:
    damped_rotary_spring(cpBody* a, cpBody* b, f32 restAngle, f32 stiffness, f32 damping, space* parent);
};

////////////////////////////////////////////////////////////

struct rotary_limit_joint_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    f32                   Min;
    f32                   Max;
};

class TCOB_API rotary_limit_joint final : public constraint {
    friend class space;

private:
    rotary_limit_joint(cpBody* a, cpBody* b, f32 min, f32 max, space* parent);
};

////////////////////////////////////////////////////////////

struct ratchet_joint_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    f32                   Phase;
    f32                   Ratchet;
};

class TCOB_API ratchet_joint final : public constraint {
    friend class space;

private:
    ratchet_joint(cpBody* a, cpBody* b, f32 phase, f32 ratchet, space* parent);
};

////////////////////////////////////////////////////////////

struct gear_joint_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    f32                   Phase;
    f32                   Ratio;
};

class TCOB_API gear_joint final : public constraint {
    friend class space;

private:
    gear_joint(cpBody* a, cpBody* b, f32 phase, f32 ratio, space* parent);
};

////////////////////////////////////////////////////////////

struct simple_motor_settings {
    std::shared_ptr<body> A;
    std::shared_ptr<body> B;
    f32                   Rate;
};

class TCOB_API simple_motor final : public constraint {
    friend class space;

private:
    simple_motor(cpBody* a, cpBody* b, f32 rate, space* parent);
};

}

#endif
