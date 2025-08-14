// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Body.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <cassert>
    #include <memory>
    #include <span>

    #include "B2D.hpp"

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Common.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/physics/Physics.hpp"
    #include "tcob/physics/Shape.hpp"
    #include "tcob/physics/World.hpp"

namespace tcob::physics {

body::body(world& world, detail::b2d_world* b2dWorld, body_transform const& xform, settings const& bodySettings)
    : Type {{[this] { return _impl->get_type(); },
             [this](auto const& value) { _impl->set_type(value); }}}
    , LinearVelocity {{[this] -> point_f { return _impl->get_linear_velocity(); },
                       [this](auto const& value) { _impl->set_linear_velocity(value); }}}
    , AngularVelocity {{[this] -> radian_f { return radian_f {_impl->get_angular_velocity()}; },
                        [this](auto const& value) { _impl->set_angular_velocity(radian_f {value.Value}); }}}
    , LinearDamping {{[this] -> f32 { return _impl->get_linear_damping(); },
                      [this](auto const& value) { _impl->set_linear_damping(value); }}}
    , AngularDamping {{[this] -> f32 { return _impl->get_angular_damping(); },
                       [this](auto const& value) { _impl->set_angular_damping(value); }}}
    , EnableSleep {{[this] -> bool { return _impl->get_enable_sleep(); },
                    [this](auto const& value) { _impl->set_enable_sleep(value); }}}
    , IsAwake {{[this] -> bool { return _impl->get_awake(); },
                [this](auto const& value) { _impl->set_awake(value); }}}
    , IsFixedRotation {{[this] -> bool { return _impl->get_fixed_rotation(); },
                        [this](auto const& value) { _impl->set_fixed_rotation(value); }}}
    , IsBullet {{[this] -> bool { return _impl->get_bullet(); },
                 [this](auto const& value) { _impl->set_bullet(value); }}}
    , Enabled {{[this] -> bool { return _impl->get_enabled(); },
                [this](auto const& value) { _impl->set_enabled(value); }}}
    , GravityScale {{[this] -> f32 { return _impl->get_gravity_scale(); },
                     [this](auto const& value) { _impl->set_gravity_scale(value); }}}
    , SleepThreshold {{[this] -> f32 { return _impl->get_sleep_threshold(); },
                       [this](auto const& value) { _impl->set_sleep_threshold(value); }}}
    , Name {{[this] -> string { return _impl->get_name(); },
             [this](auto const& value) { _impl->set_name(value); }}}
    , Transform {{[this] -> body_transform { return _impl->get_transform(); },
                  [this](auto const& value) { _impl->set_transform(value); }}}
    , MassData {{[this] -> mass_data { return _impl->get_mass_data(); },
                 [this](auto const& value) { _impl->set_mass_data(value); }}}
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

void body::apply_mass_from_shapes() const
{
    _impl->apply_mass_from_shapes();
}

auto body::position() const -> point_f
{
    return _impl->get_position();
}

auto body::rotation() const -> radian_f
{
    return _impl->get_rotation();
}

auto body::rotational_inertia() const -> f32
{
    return _impl->get_rotational_inertia();
}

auto body::world_to_local_point(point_f pos) const -> point_f
{
    return _impl->get_local_point(pos);
}

auto body::local_to_world_point(point_f pos) const -> point_f
{
    return _impl->get_world_point(pos);
}

auto body::world_to_local_vector(point_f pos) const -> point_f
{
    return _impl->get_local_vector(pos);
}

auto body::local_to_world_vector(point_f pos) const -> point_f
{
    return _impl->get_world_vector(pos);
}

auto body::get_local_point_velocity(point_f pos) const -> point_f
{
    return _impl->get_local_point_velocity(pos);
}

auto body::get_world_point_velocity(point_f pos) const -> point_f
{
    return _impl->get_world_point_velocity(pos);
}

void body::set_target_transform(body_transform xform, f32 timeStep) const
{
    _impl->set_target_transform(xform, timeStep);
}

auto body::operator==(body const& other) const -> bool
{
    return _impl.get() == other._impl.get();
}

auto body::world_center_of_mass() const -> point_f
{
    return _impl->get_center();
}

auto body::local_center_of_mass() const -> point_f
{
    return _impl->get_local_center();
}

auto body::mass() const -> f32
{
    return _impl->get_mass();
}

auto body::parent() -> world&
{
    return _world;
}

auto body::shapes() -> std::span<std::shared_ptr<shape>>
{
    return _shapes;
}

void body::remove_shape(shape const& shapePtr)
{
    helper::erase_first(_shapes, [&shapePtr](auto const& val) { return val.get() == &shapePtr; });
}

auto body::create_chain(chain::settings const& chainSettings) -> std::shared_ptr<chain>
{
    return _chains.emplace_back(std::shared_ptr<chain> {new chain {*this, _impl.get(), chainSettings}});
}

void body::remove_chain(chain const& chainPtr)
{
    helper::erase_first(_chains, [&chainPtr](auto const& val) { return val.get() == &chainPtr; });
}

void body::wake_up() const
{
    _impl->set_awake(true);
}

void body::sleep() const
{
    _impl->set_awake(false);
}

void body::enable_contact_events(bool enable) const
{
    _impl->enable_contact_events(enable);
}

void body::enable_hit_events(bool enable) const
{
    _impl->enable_hit_events(enable);
}

auto body::aabb() const -> rect_f
{
    return _impl->compute_aabb();
}

////////////////////////////////////////////////////////////

auto rotation::x_axis() const -> point_f
{
    return {Cosine, Sine};
}

auto rotation::y_axis() const -> point_f
{
    return {-Sine, Cosine};
}

auto rotation::FromAngle(radian_f angle) -> rotation
{
    return detail::rot_from_angle(angle);
}

}

#endif
