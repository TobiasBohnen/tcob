// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <stack>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API camera final {
    friend auto constexpr operator==(camera const&, camera const&) -> bool;

public:
    camera(render_target& parent);

    point_f ViewOffset;

    size_f  Zoom {size_f::One};
    point_f Position;

    u32 VisibilityMask {0xFFFFFFFF};

    auto get_matrix() const -> mat4;                 // TODO: get_
    auto get_viewport() const -> rect_f;             // TODO: get_
    auto get_transformed_viewport() const -> rect_f; // TODO: get_

    void move_by(point_f offset);
    void look_at(point_f position);
    auto get_look_at() const -> point_f;
    void zoom_by(size_f factor);

    auto convert_world_to_screen(rect_f const& rect) const -> rect_i;
    auto convert_world_to_screen(point_f point) const -> point_i;

    auto convert_screen_to_world(rect_i const& rect) const -> rect_f;
    auto convert_screen_to_world(point_i point) const -> point_f;

    void push_state();
    void pop_state();

private:
    auto get_transform() const -> transform;

    struct xform_state {
        size_f  Zoom;
        point_f Position;
    };

    std::stack<xform_state> _states;
    render_target&          _parent;
};

}
