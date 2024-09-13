// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Animation.hpp"

namespace tcob::gfx {
using namespace std::chrono_literals;
using namespace tcob::tweening;

auto frame_animation::operator()(f64 t) const -> string
{
    return get_frame_at(get_duration() * t);
}

auto frame_animation::get_frame_at(milliseconds time) const -> string
{
    for (auto const& frame : Frames) {
        if (time <= frame.Duration) {
            return frame.Name;
        }
        time -= frame.Duration;
    }

    return Frames[Frames.size() - 1].Name;
}

auto frame_animation::get_duration() const -> milliseconds
{
    milliseconds retValue {0};
    for (auto const& frame : Frames) {
        retValue += frame.Duration;
    }

    return retValue;
}

}
