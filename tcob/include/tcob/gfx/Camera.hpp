// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API camera final {
    friend auto constexpr operator==(camera const&, camera const&) -> bool;

public:
    u32 VisibilityMask {0xFFFFFFFF};

    auto get_matrix() -> mat4;
    auto get_viewport() const -> rect_f;
    auto get_transformed_viewport() -> rect_f;

    // TODO: meh vvvvvv
    auto get_size() const -> size_f;
    void set_size(size_f size);

    auto get_offset() const -> point_f;
    void set_offset(point_f pos);

    auto get_zoom() const -> size_f;
    void set_zoom(size_f zoom);

    auto get_position() const -> point_f;
    void set_position(point_f pos);
    // TODO: meh ^^^^^^

    void move_by(point_f offset);
    void look_at(point_f position);
    auto get_look_at() -> point_f;
    void zoom_by(size_f factor);

    auto convert_world_to_screen(rect_f const& rect) -> rect_i;
    auto convert_world_to_screen(point_f point) -> point_i;

    auto convert_screen_to_world(rect_i const& rect) -> rect_f;
    auto convert_screen_to_world(point_i point) -> point_f;

private:
    void update_transform();

    bool      _transformDirty {true};
    transform _transform {transform::Identity};
    size_f    _targetSize {size_f::Zero};
    size_f    _zoom {size_f::One};
    point_f   _position {point_f::Zero};
    point_f   _offset {point_f::Zero};
};

auto constexpr operator==(camera const& left, camera const& right) -> bool
{
    return left._targetSize == right._targetSize
        && left._zoom == right._zoom
        && left._position == right._position
        && left._offset == right._offset;
}

}
