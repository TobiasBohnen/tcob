// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <atomic>

#include "tcob/core/Signal.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API timer final { // TODO: add to task_manager
public:
    enum class mode : u8 {
        BusyLoop,
        Sleep
    };

    timer() = default;
    ~timer();

    signal<milliseconds const> Tick;

    auto is_running() const -> bool;

    void start(milliseconds interval, mode mode = mode::Sleep, bool looping = true);
    void stop();

private:
    std::atomic_bool _isRunning {false};
    std::atomic_bool _stopRequested {false};
};
}
