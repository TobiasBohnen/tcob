// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/World.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <memory>

    #include <B2D.hpp>

    #include "tcob/core/Common.hpp"

namespace tcob::physics {

world::world()
    : world {settings {}}
{
}

world::world(settings const& settings)
    : Gravity {{[&]() -> point_f { return _impl->get_gravity(); },
                [&](auto const& value) { _impl->set_gravity(value); }}}
    , _impl {std::make_unique<detail::b2d_world>(settings)}
{
    EnableSleeping(settings.EnableSleeping);
    EnableSleeping.Changed.connect([&](bool value) { _impl->set_enable_sleeping(value); });

    EnableContinuous(settings.EnableContinuous);
    EnableContinuous.Changed.connect([&](bool value) { _impl->set_enable_continuous(value); });

    RestitutionThreshold(settings.RestitutionThreshold);
    RestitutionThreshold.Changed.connect([&](bool value) { _impl->set_restitution_threshold(value); });

    HitEventThreshold(settings.HitEventThreshold);
    HitEventThreshold.Changed.connect([&](bool value) { _impl->set_hit_event_threshold(value); });
}

world::~world() = default;

auto world::bodies() -> std::span<std::shared_ptr<body>>
{
    return _bodies;
}

auto world::joints() -> std::span<std::shared_ptr<joint>>
{
    return _joints;
}

auto world::create_body(body_transform const& xform, body::settings const& bodySettings) -> std::shared_ptr<body>
{
    return _bodies.emplace_back(std::shared_ptr<body> {new body {*this, _impl.get(), xform, bodySettings}});
}

void world::remove_body(body const& body)
{
    helper::erase(_bodies, [ptr = &body](auto const& val) { return val.get() == ptr; });
}

auto world::find_body(shape const& s) -> std::shared_ptr<body>
{
    for (auto& body : _bodies) {
        for (auto& shape : body->shapes()) {
            if (*shape == s) {
                return body;
            }
        }
    }

    return nullptr;
}

void world::remove_joint(joint const& joint)
{
    helper::erase(_joints, [ptr = &joint](auto const& val) { return val.get() == ptr; });
}

auto world::body_events() const -> physics::body_events
{
    return _impl->get_body_events();
}

auto world::contact_events() const -> physics::contact_events
{
    return _impl->get_contact_events();
}

auto world::sensor_events() const -> physics::sensor_events
{
    return _impl->get_sensor_events();
}

void world::draw(debug_draw const& draw) const
{
    _impl->draw(draw._impl.get(), draw.Settings);
}

void world::explode(point_f pos, f32 radius, f32 impulse) const
{
    _impl->explode(pos, radius, impulse);
}

void world::on_update(milliseconds deltaTime)
{
    _impl->step(static_cast<f32>(deltaTime.count() / 1000), SubSteps);
}

}

#endif
