// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Camera.hpp"

#include <algorithm>
#include <utility>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {

camera::camera(render_target& parent)
    : _parent {parent}
{
    Zoom.Changed.connect([this](auto const&) { _dirty = true; });
    Position.Changed.connect([this](auto const&) { _dirty = true; });
    Rotation.Changed.connect([this](auto const&) { _dirty = true; });
    ViewOffset.Changed.connect([this](auto const&) { _dirty = true; });
}

auto camera::get_transform() const -> transform
{
    if (_dirty) {
        _cached = rebuild_transform();
        _dirty  = false;
    }
    return _cached;
}

auto camera::rebuild_transform() const -> transform
{
    transform     xform;
    auto const    size {size_f {*_parent.Size}};
    point_f const center {size.Width / 2.0f, size.Height / 2.0f};

    if (Zoom != size_f::One) {
        xform.scale_at(*Zoom, center);
    }
    if (*Rotation != 0.0f) {
        xform.rotate_at(*Rotation, center);
    }
    if (Position != point_f::Zero) {
        xform.translate({-Position->X, -Position->Y});
    }
    return xform;
}

auto camera::matrix() const -> mat4
{
    return get_transform().as_matrix4();
}

auto camera::viewport() const -> rect_i
{
    return {*ViewOffset, *_parent.Size};
}

auto camera::transformed_viewport() const -> rect_f
{
    return convert_screen_to_world(viewport());
}

void camera::zoom_by(size_f factor)
{
    Zoom = (*Zoom * factor);
}

void camera::zoom_to(size_f target)
{
    Zoom = target;
}

void camera::move_by(point_f offset)
{
    Position = (*Position + offset);
}

void camera::rotate_by(degree_f degrees)
{
    Rotation = (*Rotation + degrees);
}

void camera::look_at(point_f pos)
{
    point_f const half {transformed_viewport().local_center()
                        * point_f {Zoom->Width, Zoom->Height}};
    Position = pos - half;
}

auto camera::get_look_at() const -> point_f
{
    return *Position
        + transformed_viewport().local_center()
        * point_f {Zoom->Width, Zoom->Height};
}

void camera::reset()
{
    Zoom     = size_f::One;
    Position = point_f::Zero;
    Rotation = degree_f {0};
}

auto camera::convert_world_to_screen(point_f point) const -> point_i
{
    return point_i {get_transform() * point} + *ViewOffset;
}

auto camera::convert_world_to_screen(rect_f const& rect) const -> rect_i
{
    point_f const tl {convert_world_to_screen(rect.top_left())};
    point_f const br {convert_world_to_screen(rect.bottom_right())};
    return rect_i::FromLTRB(
        static_cast<i32>(tl.X), static_cast<i32>(tl.Y),
        static_cast<i32>(br.X), static_cast<i32>(br.Y));
}

auto camera::convert_screen_to_world(point_i point) const -> point_f
{
    return get_transform().as_inverted() * point_f {point - *ViewOffset};
}

auto camera::convert_screen_to_world(rect_i const& rect) const -> rect_f
{
    point_f const tl {convert_screen_to_world(rect.top_left())};
    point_f const br {convert_screen_to_world(rect.bottom_right())};
    return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
}

void camera::push_state()
{
    _states.emplace(*Position, *Zoom, *Rotation);
    reset();
}

void camera::pop_state()
{
    if (_states.empty()) { return; }
    auto const& s {_states.top()};
    Position = s.Position;
    Zoom     = s.Zoom;
    Rotation = s.Rotation;
    _states.pop();
}

////////////////////////////////////////////////////////////

camera_controller::camera_controller(camera& cam)
    : _camera {cam}
    , _basePosition {cam.get_look_at()}
    , _baseRotation {*cam.Rotation}
    , _rng {}
{
}

void camera_controller::add_trauma(f32 trauma)
{
    _trauma = std::min(_trauma + trauma, 1.0f);
}

void camera_controller::pan(std::vector<waypoint> path)
{
    if (path.empty()) { return; }
    _panPath      = std::move(path);
    _panIndex     = 0;
    _panElapsed   = milliseconds {0};
    _basePosition = _camera.get_look_at();
    _baseRotation = *_camera.Rotation;
    _panOrigin    = {
        .Position = _basePosition,
        .Zoom     = *_camera.Zoom,
        .Rotation = _baseRotation,
    };
    _panning = true;
}

void camera_controller::move_by(point_f offset)
{
    _basePosition = _basePosition + offset;
}

void camera_controller::update(milliseconds deltaTime)
{
    if (_panning) {
        auto const& wp {_panPath[_panIndex]};
        _panElapsed += deltaTime;
        f32 const factor {static_cast<f32>(std::clamp(_panElapsed.count() / wp.TimeToArrive.count(), 0.0, 1.0))};

        _basePosition = point_f::Lerp(_panOrigin.Position, wp.Position, factor);
        _baseRotation = degree_f::Lerp(_panOrigin.Rotation, wp.Rotation.value_or(_panOrigin.Rotation), factor);
        _camera.Zoom  = size_f::Lerp(_panOrigin.Zoom, wp.Zoom.value_or(_panOrigin.Zoom), factor);

        if (_panElapsed >= wp.TimeToArrive) {
            _panOrigin = {
                .Position = wp.Position,
                .Zoom     = *_camera.Zoom,
                .Rotation = wp.Rotation.value_or(_panOrigin.Rotation),
            };
            _panElapsed = milliseconds {0};
            ++_panIndex;
            if (_panIndex >= _panPath.size()) {
                _panning = false;
            }
        }
    }

    if (_trauma > 0.0f) {
        _trauma = std::max(0.0f, _trauma - static_cast<f32>(TraumaDecay * deltaTime.count() / 1000.0));
        f32 const shake {_trauma * _trauma};
        _shakeOffset = point_f {
            _rng(-1.0f, 1.0f) * MaxShakePixels * shake,
            _rng(-1.0f, 1.0f) * MaxShakePixels * shake};
        _shakeAngle = degree_f {
            _rng(-1.0f, 1.0f) * MaxShakeDegrees * shake};
    } else {
        _shakeOffset = point_f::Zero;
        _shakeAngle  = degree_f {0};
    }

    _camera.look_at(_basePosition);
    _camera.Rotation = (_baseRotation + _shakeAngle);
}

}
