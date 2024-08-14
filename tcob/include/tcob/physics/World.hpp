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
    #include "tcob/physics/Joint.hpp"
    #include "tcob/physics/Physics.hpp"

namespace tcob::physics {

////////////////////////////////////////////////////////////

class TCOB_API world final : public updatable, public non_copyable {
public:
    world();
    ~world() override;

    i32           SubSteps {4};
    prop<point_f> Gravity;
    prop<bool>    AllowSleeping;

    auto get_bodies() -> std::span<std::shared_ptr<body>>;
    auto get_joints() -> std::span<std::shared_ptr<joint>>;

    auto create_body(body_transform const& xform, body_settings const& settings) -> std::shared_ptr<body>;
    void destroy_body(std::shared_ptr<body> const& bodyPtr);

    template <typename T>
    auto create_joint(auto&& jointSettings) -> std::shared_ptr<T>
    {
        return std::static_pointer_cast<T>(_joints.emplace_back(std::shared_ptr<T> {new T {_impl.get(), jointSettings}}));
    }

    void destroy_joint(std::shared_ptr<joint> const& jointPtr);

private:
    void on_update(milliseconds deltaTime) override;

    std::unique_ptr<detail::b2dWorld> _impl;

    std::vector<std::shared_ptr<body>>  _bodies;
    std::vector<std::shared_ptr<joint>> _joints;
};

}

#endif
