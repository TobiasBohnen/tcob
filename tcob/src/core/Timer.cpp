// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/Timer.hpp>

#include <SDL2/SDL_timer.h>

namespace tcob {

auto callback(u32 interval, void* param) -> u32
{
    reinterpret_cast<Timer*>(param)->Tick(MilliSeconds { interval });
    return interval;
}

Timer::Timer() = default;

Timer::~Timer()
{
    stop();
}

void Timer::start(MilliSeconds interval)
{
    _id = SDL_AddTimer(static_cast<u32>(interval.count()), &callback, this);
}

void Timer::stop()
{
    if (_id) {
        SDL_RemoveTimer(_id);
        _id = 0;
    }
}

auto Timer::is_running() const -> bool
{
    return _id;
}

} // namespace tcob
