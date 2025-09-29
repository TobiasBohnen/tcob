// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <memory>
#include <span>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/physics/B2DShape.hpp"
#include "tcob/physics/Physics.hpp"


namespace tcob::physics {
////////////////////////////////////////////////////////////

class TCOB_API rotation {
public:
    f32 Cosine {0.0f};
    f32 Sine {0.0f};

    auto x_axis() const -> point_f;
    auto y_axis() const -> point_f;

    static auto FromAngle(radian_f angle) -> rotation;
};

struct body_transform {
    /// The world position of the body. Avoid creating bodies at the origin
    /// since this can lead to many overlapping shapes.
    point_f Center {point_f::Zero};

    /// The world angle of the body in radians.
    radian_f Angle {0.0f};

    auto operator==(body_transform const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API body final {
    friend class world;
    friend class joint; // for _impl

public:
    struct settings {
        /// The body type: static, kinematic, or dynamic.
        /// Note: if a dynamic body would have zero mass, the mass is set to one.
        body_type Type {body_type::Static};

        /// The initial linear velocity of the body's origin. Typically in meters per second.
        point_f LinearVelocity {point_f::Zero};

        /// The initial angular velocity of the body. Radians per second.
        radian_f AngularVelocity {0.0f};

        /// Linear damping is use to reduce the linear velocity. The damping parameter
        /// can be larger than 1 but the damping effect becomes sensitive to the
        /// time step when the damping parameter is large.
        ///	Generally linear damping is undesirable because it makes objects move slowly
        ///	as if they are floating.
        f32 LinearDamping {0.0f};

        /// Angular damping is use to reduce the angular velocity. The damping parameter
        /// can be larger than 1.0f but the damping effect becomes sensitive to the
        /// time step when the damping parameter is large.
        ///	Angular damping can be use slow down rotating bodies.
        f32 AngularDamping {0.0f};

        /// Set this flag to false if this body should never fall asleep. Note that
        /// this increases CPU usage.
        bool EnableSleep {true};

        /// Is this body initially awake or sleeping?
        bool IsAwake {true};

        /// Should this body be prevented from rotating? Useful for characters.
        bool IsFixedRotation {false};

        /// Is this a fast moving body that should be prevented from tunneling through
        /// other moving bodies? Note that all bodies are prevented from tunneling through
        /// kinematic and static bodies. This setting is only considered on dynamic bodies.
        /// @warning You should use this flag sparingly since it increases processing time.
        bool IsBullet {false};

        /// Does this body start out enabled?
        bool IsEnabled {true};

        /// Scale the gravity applied to this body.
        f32 GravityScale {1.0f};

        /// Sleep velocity threshold, default is 0.05 meter per second
        f32 SleepThreshold {0.05f};

        /// This allows this body to bypass rotational speed limits. Should only be used
        ///	for circular objects, like wheels.
        bool AllowFastRotation {false};
    };

    ~body();

    prop_fn<body_type> Type;
    prop_fn<point_f>   LinearVelocity;
    prop_fn<radian_f>  AngularVelocity;
    prop_fn<f32>       LinearDamping;
    prop_fn<f32>       AngularDamping;
    prop_fn<bool>      EnableSleep;
    prop_fn<bool>      IsAwake;
    prop_fn<bool>      IsFixedRotation;
    prop_fn<bool>      IsBullet;
    prop_fn<bool>      Enabled;
    prop_fn<f32>       GravityScale;
    prop_fn<f32>       SleepThreshold;

    prop_fn<string>         Name;
    prop_fn<body_transform> Transform;
    prop_fn<mass_data>      MassData;

    std::any UserData;

    auto operator==(body const& other) const -> bool;

    auto world_center_of_mass() const -> point_f;
    auto local_center_of_mass() const -> point_f;

    auto mass() const -> f32;

    auto aabb() const -> rect_f;

    auto position() const -> point_f;
    auto rotation() const -> radian_f;

    auto rotational_inertia() const -> f32;

    auto world_to_local_point(point_f pos) const -> point_f;
    auto local_to_world_point(point_f pos) const -> point_f;
    auto world_to_local_vector(point_f pos) const -> point_f;
    auto local_to_world_vector(point_f pos) const -> point_f;

    auto get_local_point_velocity(point_f pos) const -> point_f;
    auto get_world_point_velocity(point_f pos) const -> point_f;

    void set_target_transform(body_transform xform, f32 timeStep) const;

    auto parent() -> world&;

    auto shapes() -> std::span<std::shared_ptr<shape>>;

    template <typename T>
    auto create_shape(T::settings const& settings) -> std::shared_ptr<T>;
    void remove_shape(shape const& shapePtr);

    auto create_chain(chain::settings const& chainSettings) -> std::shared_ptr<chain>;
    void remove_chain(chain const& chainPtr);

    void apply_force(point_f force, point_f point, bool wake = true) const;
    void apply_force_to_center(point_f force, bool wake = true) const;
    void apply_linear_impulse(point_f imp, point_f point, bool wake = true) const;
    void apply_linear_impulse_to_center(point_f imp, bool wake = true) const;
    void apply_torque(f32 torque, bool wake = true) const;
    void apply_angular_impulse(f32 impulse, bool wake = true) const;

    void apply_mass_from_shapes() const;

    void wake_up() const;
    void sleep() const;

    void enable_contact_events(bool enable) const;
    void enable_hit_events(bool enable) const;

    auto get_impl() -> detail::b2d_body*;

private:
    body(world& world, detail::b2d_world* b2dWorld, body_transform const& xform, settings const& bodySettings);

    std::unique_ptr<detail::b2d_body>   _impl;
    world&                              _world;
    std::vector<std::shared_ptr<shape>> _shapes;
    std::vector<std::shared_ptr<chain>> _chains;
};

template <typename T>
inline auto body::create_shape(T::settings const& settings) -> std::shared_ptr<T>
{
    return std::static_pointer_cast<T>(_shapes.emplace_back(std::shared_ptr<T> {new T {*this, _impl.get(), settings, settings.Shape}}));
}

}
