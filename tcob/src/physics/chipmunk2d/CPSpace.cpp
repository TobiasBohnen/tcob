// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/chipmunk2d/CPSpace.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include <cassert>

    #include <chipmunk/chipmunk.h>

namespace tcob::physics::chipmunk2d {

space::space()
    : Iterations {{[&]() { return cpSpaceGetIterations(_cpSpace); },
                   [&](auto const& value) { cpSpaceSetIterations(_cpSpace, value); }}}
    , Gravity {{[&]() -> point_f {
                    auto retValue {cpSpaceGetGravity(_cpSpace)};
                    return {retValue.x, retValue.y};
                },
                [&](auto const& value) { cpSpaceSetGravity(_cpSpace, {value.X, value.Y}); }}}
    , Damping {{[&]() { return cpSpaceGetDamping(_cpSpace); },
                [&](auto const& value) { cpSpaceSetDamping(_cpSpace, value); }}}
    , IdleSpeedThreshold {{[&]() { return cpSpaceGetIdleSpeedThreshold(_cpSpace); },
                           [&](auto const& value) { cpSpaceSetIdleSpeedThreshold(_cpSpace, value); }}}
    , SleepTimeThreshold {{[&]() { return cpSpaceGetSleepTimeThreshold(_cpSpace); },
                           [&](auto const& value) { cpSpaceSetSleepTimeThreshold(_cpSpace, value); }}}
    , CollisionSlop {{[&]() { return cpSpaceGetCollisionSlop(_cpSpace); },
                      [&](auto const& value) { cpSpaceSetCollisionSlop(_cpSpace, value); }}}
    , CollisionBias {{[&]() { return cpSpaceGetCollisionBias(_cpSpace); },
                      [&](auto const& value) { cpSpaceSetCollisionBias(_cpSpace, value); }}}
    , CollisionPersistence {{[&]() { return cpSpaceGetCollisionPersistence(_cpSpace); },
                             [&](auto const& value) { cpSpaceSetCollisionPersistence(_cpSpace, value); }}}
    , _cpSpace {cpSpaceNew()}
{
    auto* handler {cpSpaceAddDefaultCollisionHandler(_cpSpace)};
    handler->beginFunc    = &detail::begin_func;
    handler->separateFunc = &detail::separate_func;
    handler->userData     = this;
}

space::~space()
{
    if (_cpSpace) {
        cpSpaceFree(_cpSpace);
        _cpSpace = nullptr;
    }
}

auto space::get_current_time_step() const -> f32
{
    return cpSpaceGetCurrentTimeStep(_cpSpace);
}

auto space::create_body() -> std::shared_ptr<body>
{
    assert(_cpSpace);
    return _bodies.emplace_back(std::shared_ptr<body> {new body {this}});
}

void space::remove_body(std::shared_ptr<body> const& bodyPtr)
{
    assert(_cpSpace);
    _bodies.erase(std::find(_bodies.begin(), _bodies.end(), bodyPtr));
    cpSpaceRemoveBody(_cpSpace, detail::get_impl(bodyPtr.get()));
}

auto space::create_constraint(pin_joint_settings const& settings) -> std::shared_ptr<pin_joint>
{
    assert(_cpSpace);
    std::shared_ptr<pin_joint> retValue {
        new pin_joint {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                       settings.AnchorA, settings.AnchorB, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(slide_joint_settings const& settings) -> std::shared_ptr<slide_joint>
{
    assert(_cpSpace);
    std::shared_ptr<slide_joint> retValue {
        new slide_joint {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                         settings.AnchorA, settings.AnchorB, settings.Min, settings.Max, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(pivot_joint_settings const& settings) -> std::shared_ptr<pivot_joint>
{
    assert(_cpSpace);
    std::shared_ptr<pivot_joint> retValue {
        new pivot_joint {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                         settings.Pivot, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(pivot_joint_settings2 const& settings) -> std::shared_ptr<pivot_joint>
{
    assert(_cpSpace);
    std::shared_ptr<pivot_joint> retValue {
        new pivot_joint {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                         settings.AnchorA, settings.AnchorB, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(groove_joint_settings const& settings) -> std::shared_ptr<groove_joint>
{
    assert(_cpSpace);
    std::shared_ptr<groove_joint> retValue {
        new groove_joint {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                          settings.GrooveA, settings.GrooveB, settings.AnchorB, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(damped_spring_settings const& settings) -> std::shared_ptr<damped_spring>
{
    assert(_cpSpace);
    std::shared_ptr<damped_spring> retValue {
        new damped_spring {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                           settings.AnchorA, settings.AnchorB, settings.RestLength, settings.Stiffness, settings.Damping, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(damped_rotary_spring_settings const& settings) -> std::shared_ptr<damped_rotary_spring>
{
    assert(_cpSpace);
    std::shared_ptr<damped_rotary_spring> retValue {
        new damped_rotary_spring {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                                  settings.RestAngle, settings.Stiffness, settings.Damping, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(rotary_limit_joint_settings const& settings) -> std::shared_ptr<rotary_limit_joint>
{
    assert(_cpSpace);
    std::shared_ptr<rotary_limit_joint> retValue {
        new rotary_limit_joint {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                                settings.Min, settings.Max, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(ratchet_joint_settings const& settings) -> std::shared_ptr<ratchet_joint>
{
    assert(_cpSpace);
    std::shared_ptr<ratchet_joint> retValue {
        new ratchet_joint {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                           settings.Phase, settings.Ratchet, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(gear_joint_settings const& settings) -> std::shared_ptr<gear_joint>
{
    assert(_cpSpace);
    std::shared_ptr<gear_joint> retValue {
        new gear_joint {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                        settings.Phase, settings.Ratio, this}};
    _constraints.push_back(retValue);
    return retValue;
}

auto space::create_constraint(simple_motor_settings const& settings) -> std::shared_ptr<simple_motor>
{
    assert(_cpSpace);
    std::shared_ptr<simple_motor> retValue {
        new simple_motor {detail::get_impl(settings.A.get()), detail::get_impl(settings.B.get()),
                          settings.Rate, this}};
    _constraints.push_back(retValue);
    return retValue;
}

void space::remove_constraint(std::shared_ptr<constraint> const& constraintPtr)
{
    assert(_cpSpace);
    _constraints.erase(std::find(_constraints.begin(), _constraints.end(), constraintPtr));
    cpSpaceRemoveConstraint(_cpSpace, detail::get_impl(constraintPtr.get()));
}

auto space::is_locked() const -> bool
{
    return cpSpaceIsLocked(_cpSpace);
}

void space::on_update(milliseconds deltaTime)
{
    assert(_cpSpace);
    cpSpaceStep(_cpSpace, static_cast<f32>(deltaTime.count() / 1000));
}

} // namespace chipmunk2d

#endif
