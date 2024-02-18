// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <thread>

#include "tcob/core/Signal.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API timer final {
public:
    enum class mode : u8 {
        BusyLoop,
        Sleep
    };

    timer();
    ~timer();

    signal<milliseconds const> Tick;

    auto is_running() const -> bool;

    void start(milliseconds interval, mode mode = mode::Sleep);
    void stop();

private:
    std::jthread _thread;
    bool         _isRunning {false};
};
}
