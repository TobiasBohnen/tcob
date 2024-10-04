// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API camera final {
    friend auto constexpr operator==(camera const&, camera const&) -> bool;

public:
    camera();
    camera(camera const& other) noexcept;
    auto operator=(camera const& other) noexcept -> camera&;

    prop<size_f>  Size;
    prop<size_f>  Zoom;
    prop<point_f> Position;
    prop<point_f> Offset;

    u32 VisibilityMask {0xFFFFFFFF};

    auto get_matrix() const -> mat4;
    auto get_viewport() const -> rect_f;
    auto get_transformed_viewport() const -> rect_f;

    void move_by(point_f offset);
    void look_at(point_f position);
    auto get_look_at() const -> point_f;
    void zoom_by(size_f factor);

    auto convert_world_to_screen(rect_f const& rect) const -> rect_i;
    auto convert_world_to_screen(point_f point) const -> point_i;

    auto convert_screen_to_world(rect_i const& rect) const -> rect_f;
    auto convert_screen_to_world(point_i point) const -> point_f;

private:
    void update_transform();

    transform _transform {transform::Identity};
};

auto constexpr operator==(camera const& left, camera const& right) -> bool
{
    return left.VisibilityMask == right.VisibilityMask
        && left.Size == right.Size
        && left.Zoom == right.Zoom
        && left.Position == right.Position
        && left.Offset == right.Offset;
}

}
