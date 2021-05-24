// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/Stopwatch.hpp>

namespace tcob {
auto Stopwatch::StartNew() -> Stopwatch
{
    Stopwatch retValue {};
    retValue.start();
    return retValue;
}

void Stopwatch::start()
{
    if (_isRunning) {
        return;
    }

    _start = _clock.now().time_since_epoch();
    _isRunning = true;
}

void Stopwatch::restart()
{
    reset();
    start();
}

void Stopwatch::stop()
{
    _stop = _clock.now().time_since_epoch();
    _isRunning = false;
}

void Stopwatch::reset()
{
    _start = {};
    _stop = {};
    _isRunning = false;
}

auto Stopwatch::is_running() const -> bool
{
    return _isRunning;
}

auto Stopwatch::elapsed() -> std::chrono::high_resolution_clock::duration
{
    if (_isRunning) {
        return _clock.now().time_since_epoch() - _start;
    } else {
        return _stop - _start;
    }
}

auto Stopwatch::elapsed_milliseconds() -> std::chrono::high_resolution_clock::duration::rep
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed()).count();
}

auto Stopwatch::elapsed_nanoseconds() -> std::chrono::high_resolution_clock::duration::rep
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed()).count();
}
}