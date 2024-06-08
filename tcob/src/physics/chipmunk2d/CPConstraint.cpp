// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/chipmunk2d/CPConstraint.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include <chipmunk/chipmunk.h>

namespace tcob::physics::chipmunk2d {

constraint::constraint(cpConstraint* constraint, space* parent)
    : _cpConstraint {constraint}
    , _space {parent}
{
    cpSpaceAddConstraint(detail::get_impl(parent), _cpConstraint);
}

constraint::~constraint()
{
    if (_cpConstraint) {
        cpConstraintFree(_cpConstraint);
        _cpConstraint = nullptr;
    }
}

auto static to_cpVect(point_f p) -> cpVect
{
    return {p.X, p.Y};
}

pin_joint::pin_joint(cpBody* a, cpBody* b, point_f anchorA, point_f anchorB, space* parent)
    : constraint {cpPinJointNew(a, b, to_cpVect(anchorA), to_cpVect(anchorB)), parent}
{
}

slide_joint::slide_joint(cpBody* a, cpBody* b, point_f anchorA, point_f anchorB, f32 min, f32 max, space* parent)
    : constraint {cpSlideJointNew(a, b, to_cpVect(anchorA), to_cpVect(anchorB), min, max), parent}
{
}

pivot_joint::pivot_joint(cpBody* a, cpBody* b, point_f pivot, space* parent)
    : constraint {cpPivotJointNew(a, b, to_cpVect(pivot)), parent}
{
}

pivot_joint::pivot_joint(cpBody* a, cpBody* b, point_f anchorA, point_f anchorB, space* parent)
    : constraint {cpPivotJointNew2(a, b, to_cpVect(anchorA), to_cpVect(anchorB)), parent}
{
}

groove_joint::groove_joint(cpBody* a, cpBody* b, point_f grooveA, point_f grooveB, point_f anchorB, space* parent)
    : constraint {cpGrooveJointNew(a, b, to_cpVect(grooveA), to_cpVect(grooveB), to_cpVect(anchorB)), parent}
{
}

damped_spring::damped_spring(cpBody* a, cpBody* b, point_f anchorA, point_f anchorB, f32 restLength, f32 stiffness, f32 damping, space* parent)
    : constraint {cpDampedSpringNew(a, b, to_cpVect(anchorA), to_cpVect(anchorB), restLength, stiffness, damping), parent}
{
}

damped_rotary_spring::damped_rotary_spring(cpBody* a, cpBody* b, f32 restAngle, f32 stiffness, f32 damping, space* parent)
    : constraint {cpDampedRotarySpringNew(a, b, restAngle, stiffness, damping), parent}
{
}

rotary_limit_joint::rotary_limit_joint(cpBody* a, cpBody* b, f32 min, f32 max, space* parent)
    : constraint {cpRotaryLimitJointNew(a, b, min, max), parent}
{
}

ratchet_joint::ratchet_joint(cpBody* a, cpBody* b, f32 phase, f32 ratchet, space* parent)
    : constraint {cpRatchetJointNew(a, b, phase, ratchet), parent}
{
}

gear_joint::gear_joint(cpBody* a, cpBody* b, f32 phase, f32 ratio, space* parent)
    : constraint {cpGearJointNew(a, b, phase, ratio), parent}
{
}

simple_motor::simple_motor(cpBody* a, cpBody* b, f32 rate, space* parent)
    : constraint {cpSimpleMotorNew(a, b, rate), parent}
{
}

} // namespace chipmunk2d

#endif
