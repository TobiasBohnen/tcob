// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/tweening/Tween.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct frame {
    string       Name {};
    milliseconds Duration {};
};

class TCOB_API frame_animation final {
public:
    std::vector<frame> Frames {};

    auto operator()(f64 t) const -> string;

    auto get_frame_at(milliseconds time) const -> string;
    auto get_duration() const -> milliseconds;

    static inline char const* asset_name {"frame_animation"};
};

using frame_animation_tween = tweening::callable_tween<frame_animation>;

}
