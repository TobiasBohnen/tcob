// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API stopwatch final {
    using duration = clock::duration;

public:
    auto static StartNew() -> stopwatch;
    void start();
    void stop();
    void restart();
    void reset();

    auto get_elapsed [[nodiscard]] () const -> duration;
    auto get_elapsed_milliseconds [[nodiscard]] () const -> f64;
    auto get_elapsed_nanoseconds [[nodiscard]] () const -> f64;
    auto is_running [[nodiscard]] () const -> bool;

private:
    duration _start {};
    duration _stop {};
    bool     _isRunning {false};
};
}
