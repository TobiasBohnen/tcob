// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/animation/Animation.hpp"

namespace tcob::gfx {

auto frame_animation::operator()(f64 t) const -> string
{
    return get_frame_at(duration() * t);
}

auto frame_animation::get_frame_at(milliseconds time) const -> string
{
    for (auto const& frame : Frames) {
        if (time <= frame.Duration) { return frame.Name; }
        time -= frame.Duration;
    }

    return Frames[Frames.size() - 1].Name;
}

auto frame_animation::duration() const -> milliseconds
{
    milliseconds retValue {0};
    for (auto const& frame : Frames) {
        retValue += frame.Duration;
    }

    return retValue;
}

}
