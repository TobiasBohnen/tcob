// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/animation/Animation.hpp"

#include <algorithm>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Transform.hpp"

namespace tcob::gfx {

frame_animation::frame_animation(std::vector<frame> const& frames)
    : _frames {frames}
{
    for (auto const& frame : _frames) {
        _duration += frame.Duration;
    }
}

auto frame_animation::operator()(f64 t) const -> string_view
{
    return get_frame_at(duration() * t);
}

auto frame_animation::get_frame_at(milliseconds time) const -> string_view
{
    if (_frames.empty()) { return ""; }

    for (auto const& frame : _frames) {
        if (time <= frame.Duration) { return frame.Name; }
        time -= frame.Duration;
    }

    return _frames.back().Name;
}

auto frame_animation::duration() const -> milliseconds
{
    return _duration;
}

auto frame_animation::is_empty() const -> bool
{
    return _frames.empty();
}

////////////////////////////////////////////////////////////

skeletal_animation::skeletal_animation(std::vector<bone> const& bones, tracks const& tracks)
    : _bones {bones}
    , _tracks {tracks}
{
    for (auto const& [_, keys] : tracks) {
        if (!keys.empty()) {
            _duration = std::max(_duration, keys.back().Timestamp);
        }
    }
}

auto skeletal_animation::operator()(f64 t) const -> std::vector<transform>
{
    milliseconds const       time {t * _duration.count()};
    std::map<i32, transform> locals;

    for (auto const& [boneIdx, keys] : _tracks) {
        if (keys.empty()) { continue; }

        auto make_transform {[](keyframe const& kf) {
            transform xf {};
            xf.scale(kf.Scale);
            xf.rotate(kf.Rotation);
            xf.translate(kf.Translation);
            return xf;
        }};

        if (time <= keys.front().Timestamp) {
            locals[boneIdx] = make_transform(keys.front());
            continue;
        }
        if (time >= keys.back().Timestamp) {
            locals[boneIdx] = make_transform(keys.back());
            continue;
        }

        auto        it {std::ranges::lower_bound(keys, time, {}, &keyframe::Timestamp)};
        auto const& b {*it};
        auto const& a {*std::prev(it)};
        f32 const   t2 {static_cast<f32>((time - a.Timestamp) / (b.Timestamp - a.Timestamp))};

        keyframe kf {};
        kf.Translation = a.Translation + (b.Translation - a.Translation) * t2;
        kf.Rotation    = a.Rotation + (b.Rotation - a.Rotation) * t2;
        kf.Scale       = {a.Scale.Width + ((b.Scale.Width - a.Scale.Width) * t2),
                          a.Scale.Height + ((b.Scale.Height - a.Scale.Height) * t2)};

        locals[boneIdx] = make_transform(kf);
    }

    std::vector<transform> pose(_bones.size());
    compute_pose(locals, pose);
    return pose;
}

auto skeletal_animation::duration() const -> milliseconds
{
    return _duration;
}

auto skeletal_animation::bone_count() const -> isize
{
    return std::ssize(_bones);
}

auto skeletal_animation::is_empty() const -> bool
{
    return _bones.empty();
}

void skeletal_animation::compute_pose(std::map<i32, transform> const& locals, std::vector<transform>& pose) const
{
    for (i32 i {0}; i < std::ssize(_bones); ++i) {
        auto const&     b {_bones[i]};
        auto const      it {locals.find(i)};
        transform const local {it != locals.end() ? it->second : transform::Identity};
        pose[i] = b.Parent < 0 ? b.Rest * local : pose[b.Parent] * b.Rest * local;
    }
}

}
