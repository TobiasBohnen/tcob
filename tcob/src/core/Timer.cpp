// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Timer.hpp"

#include <thread>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Stopwatch.hpp"
#include "tcob/core/TaskManager.hpp"

namespace tcob {

timer::~timer()
{
    stop();
}

void timer::start(milliseconds interval, mode mode, bool looping)
{
    stop();

    _isRunning = true;
    locate_service<task_manager>().run_async<void>([interval, mode, looping, this] {
        stopwatch sw;
        while (!_stopRequested) {
            sw.start();

            if (mode == mode::BusyLoop) {
                while (!_stopRequested && sw.elapsed() < interval) {
                    std::this_thread::yield();
                }
            } else {
                std::this_thread::sleep_for(interval);
            }

            sw.stop();
            Tick(sw.elapsed());

            if (!looping) { break; }
        }

        _isRunning = false;
    });
}

void timer::stop()
{
    if (!_isRunning) { return; }

    _stopRequested = true;
    while (_isRunning) { std::this_thread::yield(); }
    _stopRequested = false;
}

auto timer::is_running() const -> bool
{
    return _isRunning;
}

}
