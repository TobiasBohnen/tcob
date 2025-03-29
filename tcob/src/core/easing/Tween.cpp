// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/easing/Tween.hpp"

#include <chrono>
#include <cmath>
#include <memory>
#include <queue>
#include <utility>

#include "tcob/core/Common.hpp"

namespace tcob {
using namespace std::chrono_literals;

tween_base::tween_base(milliseconds duration)
    : _duration {duration}
{
}

tween_base::~tween_base()
{
    stop();
}

auto tween_base::progress() const -> f64
{
    auto const retValue {_duration.count() == 0 ? 1.0 : static_cast<f64>(_elapsedTime / _duration)};

    switch (_mode) {
    case playback_mode::Normal:
    case playback_mode::Looped:
        return retValue;
    case playback_mode::Reversed:
    case playback_mode::ReversedLooped:
        return 1. - retValue;
    case playback_mode::Alternated:
    case playback_mode::AlternatedLooped:
        return retValue <= 0.5
            ? retValue * 2.
            : (1. - retValue) * 2.;
    }

    return 0.;
}

auto tween_base::state() const -> playback_state
{
    return _state;
}

void tween_base::start(playback_mode mode)
{
    if (_state == playback_state::Stopped) { // start if stopped
        _mode            = mode;
        _state           = playback_state::Running;
        _elapsedTime     = 0ms;
        _currentInterval = 0ms;
        update(0ms);
    } else if (_state == playback_state::Paused) { // resume if paused
        _mode = mode;
        resume();
    }
}

void tween_base::stop()
{
    if (_state != playback_state::Stopped) { // stop if running or paused
        _state           = playback_state::Stopped;
        _elapsedTime     = 0ms;
        _currentInterval = 0ms;
        Finished();
    }
}

void tween_base::restart()
{
    stop();
    start();
}

void tween_base::pause()
{
    if (_state == playback_state::Running) { // pause if running
        _state = playback_state::Paused;
    }
}

void tween_base::resume()
{
    if (_state == playback_state::Paused) { // resume if paused
        _state = playback_state::Running;
    }
}

void tween_base::toggle_pause()
{
    if (_state == playback_state::Paused) {
        resume();
    } else {
        pause();
    }
}

auto tween_base::is_looping() const -> bool
{
    return _mode == playback_mode::Looped
        || _mode == playback_mode::AlternatedLooped
        || _mode == playback_mode::ReversedLooped;
}

void tween_base::on_update(milliseconds deltaTime)
{
    if (_state != playback_state::Running) { return; }

    _elapsedTime += deltaTime;

    if (Interval.has_value()) {
        _currentInterval += deltaTime;
        if (_currentInterval >= Interval) {
            _currentInterval = 0ms;
        }
    }

    if (!Interval.has_value() || _currentInterval == 0ms) {
        bool shouldStop {false};
        if (_elapsedTime > _duration) {
            if (is_looping()) {
                _elapsedTime = milliseconds {std::fmod(_elapsedTime.count(), _duration.count())};
            } else {
                _elapsedTime = _duration;
                shouldStop   = true;
            }
        }

        update_values();
        if (shouldStop) { stop(); }
    }
}

////////////////////////////////////////////////////////////

auto tween_queue::is_empty() const -> bool
{
    return _queue.empty();
}

void tween_queue::start(playback_mode mode)
{
    if (!_isRunning && !is_empty()) {
        _isRunning = true;
        _mode      = mode;
        _isLooping = mode == playback_mode::AlternatedLooped || mode == playback_mode::Looped || mode == playback_mode::ReversedLooped;
        _queue.front()->start(_mode);
    }
}

void tween_queue::stop()
{
    if (_isRunning) {
        _isRunning = false;
        _queue     = {};
    }
}

void tween_queue::pop()
{
    _queue.pop();
}

void tween_queue::on_update(milliseconds deltaTime)
{
    if (!_isRunning || is_empty()) {
        return;
    }

    auto* autom {_queue.front().get()};
    autom->update(deltaTime);
    if (autom->state() != playback_state::Running) {
        if (_isLooping) {
            _queue.push(std::move(_queue.front()));
        }
        pop();

        if (!is_empty()) {
            _queue.front()->start(_mode);
        } else {
            stop();
        }
    }
}

}
