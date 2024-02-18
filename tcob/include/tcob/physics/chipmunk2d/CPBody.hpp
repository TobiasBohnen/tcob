// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)

    #include <any>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Property.hpp"
    #include "tcob/physics/Physics.hpp"
    #include "tcob/physics/chipmunk2d/CP.hpp"
    #include "tcob/physics/chipmunk2d/CPShape.hpp"

namespace tcob::physics::chipmunk2d {
////////////////////////////////////////////////////////////

class TCOB_API body final : public non_copyable {
    friend auto detail::get_impl(body const*) -> cpBody*;
    friend auto detail::find_shape(body* s, cpShape* cpshape) -> std::shared_ptr<shape>;
    friend class space;

public:
    ~body();

    /// Wake up any sleeping or idle bodies touching a static body.
    // CP_EXPORT void cpBodyActivateStatic(cpBody* body, cpShape* filter);
    /// Force a body to fall asleep immediately along with other bodies in a group.
    //   CP_EXPORT void cpBodySleepWithGroup(cpBody* body, cpBody* group)
    /// Set the callback used to update a body's velocity.
    // CP_EXPORT void cpBodySetVelocityUpdateFunc(cpBody* body, cpBodyVelocityFunc velocityFunc);
    /// Set the callback used to update a body's position.
    /// NOTE: It's not generally recommended to override this unless you call the default position update function.
    // CP_EXPORT void cpBodySetPositionUpdateFunc(cpBody* body, cpBodyPositionFunc positionFunc);
    /// Default velocity integration function..
    // CP_EXPORT void cpBodyUpdateVelocity(cpBody* body, cpVect gravity, cpFloat damping, cpFloat dt);
    /// Default position integration function.
    // CP_EXPORT void cpBodyUpdatePosition(cpBody* body, cpFloat dt);

    prop_fn<body_type> Type;
    prop_fn<f32>       Mass;
    prop_fn<f32>       Moment;
    prop_fn<point_f>   Position;
    prop_fn<point_f>   CenterOfGravity;
    prop_fn<point_f>   Velocity;
    prop_fn<point_f>   Force;
    prop_fn<radian_f>  Angle;
    prop_fn<f32>       AngularVelocity;
    prop_fn<f32>       Torque;
    prop<std::any>     UserData;

    auto get_space() -> space&;
    auto get_rotation() const -> point_f;
    auto get_kinetic_energy() const -> f32;
    auto is_sleeping() const -> bool;

    auto local_to_world(point_f point) const -> point_f;
    auto world_to_local(point_f point) const -> point_f;

    void apply_force_at_world_point(point_f force, point_f point) const;
    void apply_force_at_local_point(point_f force, point_f point) const;

    void apply_impulse_at_world_point(point_f impulse, point_f point) const;
    void apply_impulse_at_local_point(point_f impulse, point_f point) const;

    auto get_velocity_at_world_point(point_f point) const -> point_f;
    auto get_velocity_at_local_point(point_f point) const -> point_f;

    auto create_shape(circle_shape_settings const& settings) -> std::shared_ptr<circle_shape>;
    auto create_shape(segment_shape_settings const& settings) -> std::shared_ptr<segment_shape>;
    auto create_shape(poly_shape_settings const& settings) -> std::shared_ptr<poly_shape>;
    auto create_shape(box_shape_settings const& settings) -> std::shared_ptr<box_shape>;
    auto create_shape(box_shape_settings2 const& settings) -> std::shared_ptr<box_shape>;
    void remove_shape(std::shared_ptr<shape> const& shapePtr);

    void activate() const;
    void sleep() const;

private:
    body(space* parent);

    std::vector<std::shared_ptr<shape>> _shapes;
    cpBody*                             _cpBody;
    space*                              _space;
};

}

#endif
