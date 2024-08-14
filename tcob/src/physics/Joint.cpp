// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/Joint.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <box2d/box2d.h>

    #include "B2D.hpp"

namespace tcob::physics {

joint::~joint() = default;

auto joint::operator==(joint const& other) const -> bool
{
    return _impl.get() == other._impl.get();
}

joint::joint(std::unique_ptr<detail::b2d_joint> impl)
    : _impl {std::move(impl)}
{
}

auto joint::get_body_impl(body const& body) const -> detail::b2d_body&
{
    return *body._impl;
}

distance_joint::distance_joint(detail::b2d_world* world, distance_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2d_joint>(world, get_body_impl(*jointSettings.BodyA), get_body_impl(*jointSettings.BodyB), jointSettings)}
{
}

motor_joint::motor_joint(detail::b2d_world* world, motor_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2d_joint>(world, get_body_impl(*jointSettings.BodyA), get_body_impl(*jointSettings.BodyB), jointSettings)}
{
}

mouse_joint::mouse_joint(detail::b2d_world* world, mouse_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2d_joint>(world, get_body_impl(*jointSettings.BodyA), get_body_impl(*jointSettings.BodyB), jointSettings)}
{
}

prismatic_joint::prismatic_joint(detail::b2d_world* world, prismatic_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2d_joint>(world, get_body_impl(*jointSettings.BodyA), get_body_impl(*jointSettings.BodyB), jointSettings)}
{
}

revolute_joint::revolute_joint(detail::b2d_world* world, revolute_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2d_joint>(world, get_body_impl(*jointSettings.BodyA), get_body_impl(*jointSettings.BodyB), jointSettings)}
{
}

weld_joint::weld_joint(detail::b2d_world* world, weld_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2d_joint>(world, get_body_impl(*jointSettings.BodyA), get_body_impl(*jointSettings.BodyB), jointSettings)}
{
}

wheel_joint::wheel_joint(detail::b2d_world* world, wheel_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2d_joint>(world, get_body_impl(*jointSettings.BodyA), get_body_impl(*jointSettings.BodyB), jointSettings)}
{
}

} // namespace box2d

#endif
