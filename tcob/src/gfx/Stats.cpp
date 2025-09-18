// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Stats.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

namespace tcob::gfx {
auto render_statistics::current_time() const -> f32
{
    return _time;
}

auto render_statistics::average_FPS() const -> f32
{
    return _averageFrames;
}

auto render_statistics::best_FPS() const -> f32
{
    return _bestFrames;
}

auto render_statistics::worst_FPS() const -> f32
{
    return _worstFrames;
}

void render_statistics::update(milliseconds delta)
{
    f32 const count {static_cast<f32>(delta.count())};

    _time += count;

    _frameTimes[_frameCount % FRAME_VALUES] = count;
    ++_frameCount;

    if (_frameCount >= FRAME_VALUES) {
        _averageFrames = 1000.0f / (std::accumulate(_frameTimes.begin(), _frameTimes.end(), 0.0f) / FRAME_VALUES);
        _bestFrames    = std::max(_bestFrames, 1000.0f / *std::ranges::min_element(_frameTimes));
        _worstFrames   = std::min(_worstFrames, 1000.0f / *std::ranges::max_element(_frameTimes));
    } else {
        _averageFrames = 1000.0f / (std::accumulate(_frameTimes.begin(), _frameTimes.end(), 0.0f) / static_cast<f32>(_frameCount));
    }
}

void render_statistics::reset()
{
    _averageFrames = 0;
    _worstFrames   = std::numeric_limits<f32>::max();
    _bestFrames    = 0;

    _frameTimes.fill(0);
    _frameCount = 0;
}
}
