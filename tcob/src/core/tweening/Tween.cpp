// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/tweening/Tween.hpp"

namespace tcob::tweening {
using namespace std::chrono_literals;

tween_base::tween_base(milliseconds duration)
    : _duration {duration}
{
}

tween_base::~tween_base()
{
    stop();
}

auto tween_base::get_progress() const -> f64
{
    auto const retValue {_duration.count() == 0 ? 1.0 : static_cast<f64>(_elapsedTime / _duration)};

    switch (_mode) {
    case playback_style::Normal:
    case playback_style::Looped:
        return retValue;
    case playback_style::Reversed:
    case playback_style::ReversedLooped:
        return 1. - retValue;
    case playback_style::Alternated:
    case playback_style::AlternatedLooped:
        return retValue <= 0.5
            ? retValue * 2.
            : (1. - retValue) * 2.;
    }

    return 0.;
}

auto tween_base::get_status() const -> playback_status
{
    return _status;
}

auto tween_base::get_mode() const -> playback_style
{
    return _mode;
}

void tween_base::start(playback_style mode)
{
    if (_status == playback_status::Stopped) { // start if stopped
        _mode            = mode;
        _status          = playback_status::Running;
        _elapsedTime     = 0ms;
        _currentInterval = 0ms;
        update(0ms);
    } else if (_status == playback_status::Paused) { // resume if paused
        _mode = mode;
        resume();
    }
}

void tween_base::stop()
{
    if (_status != playback_status::Stopped) { // stop if running or paused
        _status          = playback_status::Stopped;
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
    if (_status == playback_status::Running) { // pause if running
        _status = playback_status::Paused;
    }
}

void tween_base::resume()
{
    if (_status == playback_status::Paused) { // resume if paused
        _status = playback_status::Running;
    }
}

void tween_base::toggle_pause()
{
    if (_status == playback_status::Paused) {
        resume();
    } else {
        pause();
    }
}

auto tween_base::is_looping() const -> bool
{
    return _mode == playback_style::Looped
        || _mode == playback_style::AlternatedLooped
        || _mode == playback_style::ReversedLooped;
}

void tween_base::on_update(milliseconds deltaTime)
{
    if (_status != playback_status::Running) {
        return;
    }

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
        if (shouldStop) {
            stop();
            Finished();
        }
    }
}

////////////////////////////////////////////////////////////

void queue::start(playback_style mode)
{
    if (!_isRunning && !_queue.empty()) {
        _isRunning = true;
        _mode      = mode;
        _isLooping = mode == playback_style::AlternatedLooped || mode == playback_style::Looped || mode == playback_style::ReversedLooped;
        _queue.front()->start(_mode);
    }
}

void queue::stop()
{
    if (_isRunning) {
        _isRunning = false;
        std::queue<std::shared_ptr<tween_base>> empty {};
        _queue.swap(empty);
    }
}

auto queue::is_empty() const -> bool
{
    return _queue.empty();
}

void queue::pop()
{
    _queue.pop();
}

void queue::on_update(milliseconds deltaTime)
{
    if (!_isRunning || is_empty()) {
        return;
    }

    auto* autom {_queue.front().get()};
    autom->update(deltaTime);
    if (autom->get_status() != playback_status::Running) {
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
