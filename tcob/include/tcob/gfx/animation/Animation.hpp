// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/gfx/animation/Tween.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct frame {
    string       Name {};
    milliseconds Duration {};
};

void Serialize(frame const& v, auto&& s)
{
    s["name"]     = v.Name;
    s["duration"] = v.Duration;
}

auto Deserialize(frame& v, auto&& s) -> bool
{
    return s.try_get(v.Name, "name") && s.try_get(v.Duration, "duration");
}

////////////////////////////////////////////////////////////

class TCOB_API frame_animation final {
public:
    std::vector<frame> Frames {};

    auto operator()(f64 t) const -> string;

    auto get_frame_at(milliseconds time) const -> string;
    auto duration() const -> milliseconds;

    static inline char const* asset_name {"frame_animation"};
};

using frame_animation_tween = callable_tween<frame_animation>;

}
