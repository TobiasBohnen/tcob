// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/animation/Animation.hpp"

#include <algorithm>
#include <iterator>
#include <map>
#include <unordered_map>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Transform.hpp"

namespace tcob::gfx {

sprite_animation ::sprite_animation(std::vector<frame> const& frames)
    : _frames {frames}
{
    for (auto const& frame : _frames) {
        _duration += frame.Duration;
    }
}

auto sprite_animation ::operator()(f64 t) const -> string_view
{
    return get_frame_at(duration() * t);
}

auto sprite_animation ::get_frame_at(milliseconds time) const -> string_view
{
    if (_frames.empty()) { return ""; }

    for (auto const& frame : _frames) {
        if (time <= frame.Duration) { return frame.Name; }
        time -= frame.Duration;
    }

    return _frames.back().Name;
}

auto sprite_animation ::duration() const -> milliseconds
{
    return _duration;
}

auto sprite_animation ::is_empty() const -> bool
{
    return _frames.empty();
}

////////////////////////////////////////////////////////////

skeletal_animation::skeletal_animation(std::vector<bone> const& bones)
    : _bones {bones}
{
    auto const boneCount {_bones.size()};

    std::unordered_map<string, usize> nameToIndex;
    for (usize i {0}; i < boneCount; ++i) {
        nameToIndex[_bones[i].Name] = i;
    }

    _parentIndices.resize(boneCount, -1);
    for (usize i {0}; i < boneCount; ++i) {
        auto const& bone {_bones[i]};

        // parent index
        if (!bone.Parent.empty()) {
            auto const it {nameToIndex.find(bone.Parent)};
            if (it != nameToIndex.end()) {
                _parentIndices[i] = static_cast<isize>(it->second);
            }
        }

        // track duration
        milliseconds trackDur {};
        for (auto const& frame : bone.Track) {
            trackDur += frame.Duration;
        }
        _duration = std::max(_duration, trackDur);
    }
}

auto skeletal_animation::operator()(f64 t) const -> poses
{
    milliseconds const         time {t * _duration.count()};
    std::map<isize, transform> locals;
    poses                      retValue(_bones.size());

    for (usize i {0}; i < _bones.size(); ++i) {
        auto const& keys {_bones[i].Track};
        if (keys.empty()) { continue; }

        milliseconds cursor {};
        usize        idx {0};
        while (idx + 1 < keys.size() && cursor + keys[idx].Duration <= time) {
            cursor += keys[idx].Duration;
            ++idx;
        }

        if (!keys[idx].Name.empty()) {
            retValue[i].Region = keys[idx].Name;
        }

        auto const& a {keys[idx]};
        auto const& b {idx + 1 < keys.size() ? keys[idx + 1] : keys.front()};
        auto const  duration {idx + 1 < keys.size() ? a.Duration : (_duration - cursor)};
        f32 const   t2 {static_cast<f32>((time - cursor) / duration)};

        auto& xf {locals[i]};
        xf.translate(point_f::Lerp(a.Translation, b.Translation, t2));
        xf.rotate(radian_f::Lerp(a.Rotation, b.Rotation, t2));
        xf.scale(size_f::Lerp(a.Scale, b.Scale, t2));
    }

    compute_pose(locals, retValue);
    return retValue;
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

void skeletal_animation::compute_pose(std::map<isize, transform> const& locals, poses& poses) const
{
    for (usize i {0}; i < _bones.size(); ++i) {
        auto const  it {locals.find(i)};
        auto const  local {it != locals.end() ? it->second : transform::Identity};
        isize const parent {_parentIndices[i]};
        poses[i].Transform = parent < 0 ? _bones[i].Rest * local : poses[parent].Transform * _bones[i].Rest * local;
    }
}

}
