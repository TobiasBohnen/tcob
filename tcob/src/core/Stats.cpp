// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Stats.hpp"

#include <numeric>

namespace tcob {
stats::stats() = default;

auto stats::get_time() const -> f32
{
    return _time;
}

auto stats::get_average_FPS() const -> f32
{
    return _averageFrames;
}

auto stats::get_best_FPS() const -> f32
{
    return _bestFrames;
}

auto stats::get_worst_FPS() const -> f32
{
    return _worstFrames;
}

void stats::update(milliseconds delta)
{
    f32 const count {static_cast<f32>(delta.count())};

    _time += count;

    _frameTimes[_frameCount % FRAME_VALUES] = count;
    ++_frameCount;

    if (_frameCount >= FRAME_VALUES) {
        _averageFrames = 1000.0f / (std::accumulate(_frameTimes.begin(), _frameTimes.end(), 0.0f) / FRAME_VALUES);
        _bestFrames    = std::max(_bestFrames, 1000.0f / *std::min_element(_frameTimes.begin(), _frameTimes.end()));
        _worstFrames   = std::min(_worstFrames, 1000.0f / *std::max_element(_frameTimes.begin(), _frameTimes.end()));
    } else {
        _averageFrames = 1000.0f / (std::accumulate(_frameTimes.begin(), _frameTimes.end(), 0.0f) / _frameCount);
    }
}

void stats::reset()
{
    _averageFrames = 0;
    _worstFrames   = std::numeric_limits<f32>::max();
    _bestFrames    = 0;

    _frameTimes.fill(0);
    _frameCount = 0;
}
}
