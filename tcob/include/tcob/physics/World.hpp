// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <span>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/physics/Body.hpp"
    #include "tcob/physics/DebugDraw.hpp"
    #include "tcob/physics/Joint.hpp"
    #include "tcob/physics/Shape.hpp"

namespace tcob::physics {

////////////////////////////////////////////////////////////

struct contact_begin_touch_event {
    std::shared_ptr<shape> ShapeA;
    std::shared_ptr<shape> ShapeB;
};

struct contact_end_touch_event {
    std::shared_ptr<shape> ShapeA;
    std::shared_ptr<shape> ShapeB;
};

struct contact_hit_event {
    std::shared_ptr<shape> ShapeA;
    std::shared_ptr<shape> ShapeB;
    point_f                Point;
    point_f                Normal;
    f32                    ApproachSpeed;
};

struct contact_events {
    std::vector<contact_begin_touch_event> BeginTouch;
    std::vector<contact_end_touch_event>   EndTouch;
    std::vector<contact_hit_event>         Hit;
};

////////////////////////////////////////////////////////////

class TCOB_API world final : public updatable, public non_copyable {
    friend auto detail::get_impl(world const& t) -> detail::b2d_world*;

public:
    world();
    ~world() override;

    i32           SubSteps {4};
    prop<point_f> Gravity;
    prop<bool>    AllowSleeping;

    auto get_bodies() -> std::span<std::shared_ptr<body>>;

    auto create_body(body_transform const& xform, body::settings const& settings) -> std::shared_ptr<body>;
    void destroy_body(body const& body);

    auto find_body(shape const& s) -> std::shared_ptr<body>;

    auto get_joints() -> std::span<std::shared_ptr<joint>>;

    template <typename T>
    auto create_joint(auto&& jointSettings) -> std::shared_ptr<T>;
    void destroy_joint(joint const& joint);

    auto get_contact_events() const -> contact_events;

    void draw(debug_draw const& draw) const;

private:
    void on_update(milliseconds deltaTime) override;

    std::unique_ptr<detail::b2d_world> _impl;

    std::vector<std::shared_ptr<body>>  _bodies;
    std::vector<std::shared_ptr<joint>> _joints;
};

template <typename T>
inline auto world::create_joint(auto&& jointSettings) -> std::shared_ptr<T>
{
    return std::static_pointer_cast<T>(_joints.emplace_back(std::shared_ptr<T> {new T {*this, jointSettings}}));
}

}

#endif
