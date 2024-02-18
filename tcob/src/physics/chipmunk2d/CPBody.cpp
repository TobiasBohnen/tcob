// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/chipmunk2d/CPBody.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include <chipmunk/chipmunk.h>

    #include "tcob/physics/chipmunk2d/CPSpace.hpp"

namespace tcob::physics::chipmunk2d {

body::body(space* parent)
    : Type {{[&]() {
                 switch (cpBodyGetType(_cpBody)) {
                 case CP_BODY_TYPE_STATIC:
                     return body_type::Static;
                 case CP_BODY_TYPE_DYNAMIC:
                     return body_type::Dynamic;
                 case CP_BODY_TYPE_KINEMATIC:
                     return body_type::Kinematic;
                 }
                 return body_type {};
             },
             [&](auto const& value) {
                 switch (value) {
                 case body_type::Static:
                     cpBodySetType(_cpBody, CP_BODY_TYPE_STATIC);
                     return;
                 case body_type::Dynamic:
                     cpBodySetType(_cpBody, CP_BODY_TYPE_DYNAMIC);
                     return;
                 case body_type::Kinematic:
                     cpBodySetType(_cpBody, CP_BODY_TYPE_KINEMATIC);
                     return;
                 }
             }}}
    , Mass {{[&]() { return cpBodyGetMass(_cpBody); },
             [&](auto const& value) { cpBodySetMass(_cpBody, value); }}}
    , Moment {{[&]() { return cpBodyGetMoment(_cpBody); },
               [&](auto const& value) { cpBodySetMoment(_cpBody, value); }}}
    , Position {{[&]() { return detail::to_point(cpBodyGetPosition(_cpBody)); },
                 [&](auto const& value) { cpBodySetPosition(_cpBody, {value.X, value.Y}); }}}
    , CenterOfGravity {{[&]() { return detail::to_point(cpBodyGetCenterOfGravity(_cpBody)); },
                        [&](auto const& value) { cpBodySetCenterOfGravity(_cpBody, {value.X, value.Y}); }}}
    , Velocity {{[&]() { return detail::to_point(cpBodyGetVelocity(_cpBody)); },
                 [&](auto const& value) { cpBodySetVelocity(_cpBody, {value.X, value.Y}); }}}
    , Force {{[&]() { return detail::to_point(cpBodyGetForce(_cpBody)); },
              [&](auto const& value) { cpBodySetForce(_cpBody, {value.X, value.Y}); }}}
    , Angle {{[&]() -> radian_f { return radian_f {cpBodyGetAngle(_cpBody)}; },
              [&](auto const& value) { cpBodySetAngle(_cpBody, value.Value); }}}
    , AngularVelocity {{[&]() { return cpBodyGetAngularVelocity(_cpBody); },
                        [&](auto const& value) { cpBodySetAngularVelocity(_cpBody, value); }}}
    , Torque {{[&]() { return cpBodyGetTorque(_cpBody); },
               [&](auto const& value) { cpBodySetTorque(_cpBody, value); }}}
    , _cpBody {cpBodyNew(0, 0)}
    , _space {parent}
{
    cpSpaceAddBody(detail::get_impl(parent), _cpBody);
    Type(body_type::Static);
}

body::~body()
{
    if (_cpBody) {
        cpBodyFree(_cpBody);
        _cpBody = nullptr;
    }
}

auto body::get_space() -> space&
{
    return *_space;
}

auto body::get_rotation() const -> point_f
{
    return detail::to_point(cpBodyGetRotation(_cpBody));
}

auto body::get_kinetic_energy() const -> f32
{
    return cpBodyKineticEnergy(_cpBody);
}

auto body::local_to_world(point_f point) const -> point_f
{
    return detail::to_point(cpBodyLocalToWorld(_cpBody, {point.X, point.Y}));
}

auto body::world_to_local(point_f point) const -> point_f
{
    return detail::to_point(cpBodyWorldToLocal(_cpBody, {point.X, point.Y}));
}

void body::apply_force_at_world_point(point_f force, point_f point) const
{
    cpBodyApplyForceAtWorldPoint(_cpBody, {force.X, force.Y}, {point.X, point.Y});
}

void body::apply_force_at_local_point(point_f force, point_f point) const
{
    cpBodyApplyForceAtLocalPoint(_cpBody, {force.X, force.Y}, {point.X, point.Y});
}

void body::apply_impulse_at_world_point(point_f impulse, point_f point) const
{
    cpBodyApplyImpulseAtWorldPoint(_cpBody, {impulse.X, impulse.Y}, {point.X, point.Y});
}

void body::apply_impulse_at_local_point(point_f impulse, point_f point) const
{
    cpBodyApplyImpulseAtLocalPoint(_cpBody, {impulse.X, impulse.Y}, {point.X, point.Y});
}

auto body::get_velocity_at_world_point(point_f point) const -> point_f
{
    return detail::to_point(cpBodyGetVelocityAtWorldPoint(_cpBody, {point.X, point.Y}));
}

auto body::get_velocity_at_local_point(point_f point) const -> point_f
{
    return detail::to_point(cpBodyGetVelocityAtLocalPoint(_cpBody, {point.X, point.Y}));
}

auto body::create_shape(circle_shape_settings const& settings) -> std::shared_ptr<circle_shape>
{
    auto retValue {std::make_shared<circle_shape>(_cpBody, settings.Radius, settings.Offset)};
    _shapes.push_back(retValue);
    return retValue;
}

auto body::create_shape(segment_shape_settings const& settings) -> std::shared_ptr<segment_shape>
{
    auto retValue {std::make_shared<segment_shape>(_cpBody, settings.A, settings.B, settings.Radius)};
    _shapes.push_back(retValue);
    return retValue;
}

auto body::create_shape(poly_shape_settings const& settings) -> std::shared_ptr<poly_shape>
{
    auto retValue {std::make_shared<poly_shape>(_cpBody, settings.Verts, settings.Radius)};
    _shapes.push_back(retValue);
    return retValue;
}

auto body::create_shape(box_shape_settings const& settings) -> std::shared_ptr<box_shape>
{
    auto retValue {std::make_shared<box_shape>(_cpBody, settings.Size, settings.Radius)};
    _shapes.push_back(retValue);
    return retValue;
}

auto body::create_shape(box_shape_settings2 const& settings) -> std::shared_ptr<box_shape>
{
    auto retValue {std::make_shared<box_shape>(_cpBody, settings.Box, settings.Radius)};
    _shapes.push_back(retValue);
    return retValue;
}

void body::remove_shape(std::shared_ptr<shape> const& shapePtr)
{
    _shapes.erase(std::find(_shapes.begin(), _shapes.end(), shapePtr));
    cpSpaceRemoveShape(detail::get_impl(_space), detail::get_impl(shapePtr.get()));
}

void body::activate() const
{
    cpBodyActivate(_cpBody);
}

void body::sleep() const
{
    cpBodySleep(_cpBody);
}

auto body::is_sleeping() const -> bool
{
    return cpBodyIsSleeping(_cpBody);
}

} // namespace chipmunk2d

#endif
