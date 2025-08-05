// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <limits>

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API render_statistics {
    static constexpr i32 FRAME_VALUES = 100;

public:
    render_statistics();

    auto current_time() const -> f32;

    auto average_FPS() const -> f32;
    auto best_FPS() const -> f32;
    auto worst_FPS() const -> f32;

    void update(milliseconds delta);
    void reset();

private:
    std::array<f32, FRAME_VALUES> _frameTimes {};
    u64                           _frameCount {0};

    f32 _averageFrames {0};
    f32 _worstFrames {std::numeric_limits<f32>::max()};
    f32 _bestFrames {0};
    f32 _time {0};
};
}
