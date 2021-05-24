// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <stack>

#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/core/data/Transform.hpp>

namespace tcob {
class Camera final {
public:
    auto position() const -> PointF;
    void position(const PointF& position);
    void move_by(const PointF& offset);
    void look_at(const PointF& position);

    auto rotation() const -> f32;
    void rotation(f32 angle);
    void rotate_by(f32 angle);

    auto zoom() const -> SizeF;
    void zoom(const SizeF& zoom);
    void zoom_by(const SizeF& factor);

    auto matrix() -> mat4;

    auto convert_world_to_screen(const PointF& point) -> PointF;
    auto convert_world_to_screen(const RectF& rect) -> RectF;
    auto convert_screen_to_world(const PointF& point) -> PointF;
    auto convert_screen_to_world(const RectF& rect) -> RectF;

    auto frustum() const -> RectF;

    auto aspect_ratio() -> f32;
    void aspect_ratio(f32 asp);

private:
    void update_matrix();

    f32 _rotation { 0 };
    SizeF _scale { SizeF::One };
    PointF _position { PointF::Zero };
    bool _transformDirty { true };
    f32 _aspectRatio { 1 };
    Transform _transform;
};
}