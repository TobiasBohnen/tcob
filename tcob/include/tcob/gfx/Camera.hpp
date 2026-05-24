// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <stack>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob::gfx {

class render_target;

////////////////////////////////////////////////////////////

struct camera_state {
    size_f   Zoom {size_f::One};
    point_f  Position {point_f::Zero};
    degree_f Rotation {0.0f};
};

////////////////////////////////////////////////////////////

class TCOB_API camera {
public:
    explicit camera(render_target& parent);

    prop<size_f>   Zoom {size_f::One};
    prop<point_f>  Position {point_f::Zero};
    prop<degree_f> Rotation {};
    prop<point_i>  ViewOffset {};

    u32 VisibilityMask {0xFFFFFFFF};

    std::optional<rect_f> Bounds;

    auto viewport() const -> rect_i;
    auto transformed_viewport() const -> rect_f;

    auto get_transform() const -> transform;
    auto matrix() const -> mat4;

    void zoom_by(size_f factor);
    void zoom_to(size_f target);
    void move_by(point_f offset);
    void rotate_by(degree_f degrees);

    void look_at(point_f pos);
    auto get_look_at() const -> point_f;
    void reset();

    auto convert_world_to_screen(point_f point) const -> point_i;
    auto convert_world_to_screen(rect_f const&) const -> rect_i;
    auto convert_screen_to_world(point_i point) const -> point_f;
    auto convert_screen_to_world(rect_i const&) const -> rect_f;

    void push_state();
    void pop_state();

private:
    auto rebuild_transform() const -> transform;

    render_target&           _parent;
    std::stack<camera_state> _states;

    mutable bool      _dirty {true};
    mutable transform _cached {};
};

////////////////////////////////////////////////////////////

class TCOB_API camera_controller {
public:
    struct waypoint {
        point_f      Position {};
        milliseconds TimeToArrive {};
    };

    explicit camera_controller(camera& cam);

    void update(milliseconds deltaTime);
    void add_trauma(f32 trauma);

    void pan(std::vector<waypoint> path);

private:
    camera& _camera;

    // shake
    f32      _trauma {0.0f};
    point_f  _shakeOffset {point_f::Zero};
    degree_f _shakeAngle {degree_f {0}};

    static constexpr f32 MaxShakePixels {12.0f};
    static constexpr f32 MaxShakeDegrees {3.0f};
    static constexpr f32 TraumaDecay {1.5f};

    rng _rng;

    // pan
    std::vector<waypoint> _panPath;
    usize                 _panIndex {0};
    milliseconds          _panElapsed {0};
    point_f               _panOrigin {point_f::Zero};
    bool                  _panning {false};
};

}
