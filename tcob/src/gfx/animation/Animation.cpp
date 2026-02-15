// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/animation/Animation.hpp"

#include <vector>

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
}
