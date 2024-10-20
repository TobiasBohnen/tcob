// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Body.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <algorithm>
    #include <cassert>

    #include "B2D.hpp"

namespace tcob::physics {

body::body(world& world, detail::b2d_world* b2dWorld, body_transform const& xform, settings const& bodySettings)
    : Type {{[&]() { return _impl->get_type(); },
             [&](auto const& value) { _impl->set_type(value); }}}
    , LinearVelocity {{[&]() -> point_f { return _impl->get_linear_velocity(); },
                       [&](auto const& value) { _impl->set_linear_velocity(value); }}}
    , AngularVelocity {{[&]() -> radian_f { return radian_f {_impl->get_angular_velocity()}; },
                        [&](auto const& value) { _impl->set_angular_velocity(value.Value); }}}
    , LinearDamping {{[&]() -> f32 { return _impl->get_linear_damping(); },
                      [&](auto const& value) { _impl->set_linear_damping(value); }}}
    , AngularDamping {{[&]() -> f32 { return _impl->get_angular_damping(); },
                       [&](auto const& value) { _impl->set_angular_damping(value); }}}
    , EnableSleep {{[&]() -> bool { return _impl->get_enable_sleep(); },
                    [&](auto const& value) { _impl->set_enable_sleep(value); }}}
    , IsAwake {{[&]() -> bool { return _impl->get_awake(); },
                [&](auto const& value) { _impl->set_awake(value); }}}
    , IsFixedRotation {{[&]() -> bool { return _impl->get_fixed_rotation(); },
                        [&](auto const& value) { _impl->set_fixed_rotation(value); }}}
    , IsBullet {{[&]() -> bool { return _impl->get_bullet(); },
                 [&](auto const& value) { _impl->set_bullet(value); }}}
    , Enabled {{[&]() -> bool { return _impl->get_enabled(); },
                [&](auto const& value) { _impl->set_enabled(value); }}}
    , GravityScale {{[&]() { return _impl->get_gravity_scale(); },
                     [&](auto const& value) { _impl->set_gravity_scale(value); }}}
    , Transform {{[&]() -> body_transform { return _impl->get_transform(); },
                  [&](auto const& value) { _impl->set_transform(value); }}}
    , _impl {std::make_unique<detail::b2d_body>(b2dWorld, xform, bodySettings)}
    , _world {world}
{
    _impl->set_user_data(this);
}

body::~body() = default;

void body::apply_force(point_f force, point_f point, bool wake) const
{
    _impl->apply_force({force.X, force.Y}, {point.X, point.Y}, wake);
}

void body::apply_force_to_center(point_f force, bool wake) const
{
    _impl->apply_force_to_center({force.X, force.Y}, wake);
}

void body::apply_linear_impulse(point_f imp, point_f point, bool wake) const
{
    _impl->apply_linear_impulse({imp.X, imp.Y}, {point.X, point.Y}, wake);
}

void body::apply_linear_impulse_to_center(point_f imp, bool wake) const
{
    _impl->apply_linear_impulse_to_center({imp.X, imp.Y}, wake);
}

void body::apply_torque(f32 torque, bool wake) const
{
    _impl->apply_torque(torque, wake);
}

void body::apply_angular_impulse(f32 impulse, bool wake) const
{
    _impl->apply_angular_impulse(impulse, wake);
}

auto body::operator==(body const& other) const -> bool
{
    return _impl.get() == other._impl.get();
}

auto body::get_center() const -> point_f
{
    return _impl->get_center();
}

auto body::get_local_center() const -> point_f
{
    return _impl->get_local_center();
}

auto body::get_world() -> world&
{
    return _world;
}

auto body::get_shapes() -> std::span<std::shared_ptr<shape>>
{
    return _shapes;
}

void body::remove_shape(shape const& shapePtr)
{
    _shapes.erase(std::ranges::find_if(_shapes, [&shapePtr](auto const& val) {
        return val.get() == &shapePtr;
    }));
}

void body::wake_up() const
{
    _impl->set_awake(true);
}

void body::sleep() const
{
    _impl->set_awake(false);
}

}

#endif
