// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/Automation.hpp>

namespace tcob {
using namespace std::chrono_literals;

AutomationBase::AutomationBase(MilliSeconds duration)
    : _duration { duration }
{
}

void AutomationBase::start(bool looped)
{
    if (_isRunning || _duration <= 0ms) {
        //TODO: log error
        return;
    }

    _looped = looped;

    _isRunning = true;
    _elapsedTime = 0ms;
    _currentInterval = 0ms;

    update(0ms);
}

void AutomationBase::restart()
{
    stop();
    start(_looped);
}

void AutomationBase::toggle_pause()
{
    _isRunning = !_isRunning;
}

void AutomationBase::stop()
{
    _isRunning = false;
    _elapsedTime = 0ms;
    _currentInterval = 0ms;
}

auto AutomationBase::is_running() const -> bool
{
    return _isRunning;
}

auto AutomationBase::progress() const -> f32
{
    return static_cast<f32>(_elapsedTime / _duration);
}

void AutomationBase::update(MilliSeconds deltaTime)
{
    if (!_isRunning)
        return;

    _elapsedTime += deltaTime;

    if (_interval > 0ms) {
        _currentInterval += deltaTime;
        if (_currentInterval >= _interval) {
            _currentInterval = 0ms;
        }
    }

    if (_interval == 0ms || _currentInterval == 0ms) {
        if (_elapsedTime >= _duration) {
            if (_looped) {
                _elapsedTime = MilliSeconds { std::fmod(_elapsedTime.count(), _duration.count()) };
            } else {
                _elapsedTime = _duration;
                _isRunning = false;
            }
        }

        update_values();
    }
}

void AutomationBase::interval(MilliSeconds interval)
{
    _interval = std::min(interval, _duration);
}

////////////////////////////////////////////////////////////

void AutomationQueue::start(bool looped)
{
    if (_isRunning || _queue.empty()) {
        //TODO: log error
        return;
    }

    _looped = looped;
    _isRunning = true;
    _queue.front()->start();
}

void AutomationQueue::stop_and_clear()
{
    _queue = {};
    _isRunning = false;
}

auto AutomationQueue::is_empty() const -> bool
{
    return _queue.empty();
}

void AutomationQueue::update(MilliSeconds deltaTime)
{
    if (!_isRunning || _queue.empty()) {
        return;
    }

    auto* autom { _queue.front().get() };
    autom->update(deltaTime);
    if (!autom->is_running()) {
        if (_looped)
            _queue.push(std::move(_queue.front()));

        _queue.pop();

        if (!_queue.empty()) {
            _queue.front()->start();
        } else {
            _isRunning = false;
        }
    }
}

}