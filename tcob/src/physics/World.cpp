// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/World.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <B2D.hpp>

    #include <memory>

namespace tcob::physics {

world::world()
    : _impl {std::make_unique<detail::b2d_world>(point_f::Zero)}
{
    Gravity.Changed.connect([&](point_f value) { _impl->set_gravity(value); });
    Gravity(point_f::Zero);

    AllowSleeping.Changed.connect([&](bool value) { _impl->set_enable_sleeping(value); });
    AllowSleeping(true);
}

world::~world() = default;

auto world::get_bodies() -> std::span<std::shared_ptr<body>>
{
    return _bodies;
}

auto world::get_joints() -> std::span<std::shared_ptr<joint>>
{
    return _joints;
}

auto world::create_body(body_transform const& xform, body_settings const& bodySettings) -> std::shared_ptr<body>
{
    return _bodies.emplace_back(std::shared_ptr<body> {new body {_impl.get(), xform, bodySettings}});
}

void world::destroy_body(body const& body)
{
    _bodies.erase(std::find_if(_bodies.begin(), _bodies.end(), [ptr = &body](auto const& val) {
        return val.get() == ptr;
    }));
}

auto world::find_body(shape const& s) -> std::shared_ptr<body>
{
    for (auto& body : _bodies) {
        for (auto& shape : body->get_shapes()) {
            if (*shape == s) {
                return body;
            }
        }
    }

    return nullptr;
}

void world::destroy_joint(joint const& joint)
{
    _joints.erase(std::find_if(_joints.begin(), _joints.end(), [ptr = &joint](auto const& val) {
        return val.get() == ptr;
    }));
}

auto world::get_contact_events() const -> contact_events
{
    return _impl->get_contact_events(_bodies);
}

void world::draw(debug_draw const& draw) const
{
    _impl->draw(draw._impl.get(), draw.Settings);
}

void world::on_update(milliseconds deltaTime)
{
    _impl->step(static_cast<f32>(deltaTime.count() / 1000), SubSteps);
}

}

#endif
