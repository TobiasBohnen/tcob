// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Timer.hpp"

#include "tcob/core/Stopwatch.hpp"

namespace tcob {

timer::timer() = default;

timer::~timer()
{
    stop();
}

void timer::start(milliseconds interval, mode mode, bool looping)
{
    stop();

    _thread = std::jthread([=, this](std::stop_token const& stoken) {
        stopwatch sw;
        while (!stoken.stop_requested()) {
            sw.start();

            if (mode == mode::BusyLoop) {
                while (!stoken.stop_requested() && sw.get_elapsed() < interval) {
                    std::this_thread::yield();
                }
            } else {
                std::this_thread::sleep_for(interval);
            }

            sw.stop();
            Tick(sw.get_elapsed());

            if (!looping) { break; }
        }

        _isRunning = false;
    });

    _isRunning = true;
}

void timer::stop()
{
    _thread.request_stop();
    _isRunning = false;
}

auto timer::is_running() const -> bool
{
    return _isRunning;
}

}
