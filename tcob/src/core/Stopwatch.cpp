// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Stopwatch.hpp"

#include <chrono>
#include <ratio>

namespace tcob {

auto stopwatch::elapsed() const -> duration
{
    if (_isRunning) { return tcob::clock::now() - _start; }

    return _stop - _start;
}

auto stopwatch::elapsed_milliseconds() const -> f64
{
    return std::chrono::duration_cast<std::chrono::duration<f64, std::milli>>(elapsed()).count();
}

auto stopwatch::elapsed_nanoseconds() const -> f64
{
    return std::chrono::duration_cast<std::chrono::duration<f64, std::nano>>(elapsed()).count();
}

auto stopwatch::StartNew() -> stopwatch
{
    stopwatch retValue {};
    retValue.start();
    return retValue;
}

void stopwatch::start()
{
    if (_isRunning) { return; }

    _start     = tcob::clock::now();
    _isRunning = true;
}

void stopwatch::restart()
{
    reset();
    start();
}

void stopwatch::stop()
{
    _stop      = tcob::clock::now();
    _isRunning = false;
}

void stopwatch::reset()
{
    _start     = {};
    _stop      = {};
    _isRunning = false;
}

auto stopwatch::is_running() const -> bool
{
    return _isRunning;
}

}
