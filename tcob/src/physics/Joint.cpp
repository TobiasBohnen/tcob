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

joint::joint(std::unique_ptr<detail::b2dJoint> impl)
    : _impl {std::move(impl)}
{
}

distance_joint::distance_joint(detail::b2dWorld* world, distance_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2dJoint>(world, jointSettings)}
{
}

motor_joint::motor_joint(detail::b2dWorld* world, motor_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2dJoint>(world, jointSettings)}
{
}

mouse_joint::mouse_joint(detail::b2dWorld* world, mouse_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2dJoint>(world, jointSettings)}
{
}

prismatic_joint::prismatic_joint(detail::b2dWorld* world, prismatic_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2dJoint>(world, jointSettings)}
{
}

revolute_joint::revolute_joint(detail::b2dWorld* world, revolute_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2dJoint>(world, jointSettings)}
{
}

weld_joint::weld_joint(detail::b2dWorld* world, weld_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2dJoint>(world, jointSettings)}
{
}

wheel_joint::wheel_joint(detail::b2dWorld* world, wheel_joint_settings const& jointSettings)
    : joint {std::make_unique<detail::b2dJoint>(world, jointSettings)}
{
}

} // namespace box2d

#endif
