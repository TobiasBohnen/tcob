// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <chrono>

namespace tcob {
class Stopwatch final {
public:
    static auto StartNew() -> Stopwatch;
    void start();
    void restart();
    void stop();
    void reset();

    auto is_running() const -> bool;

    auto elapsed() -> std::chrono::high_resolution_clock::duration;
    auto elapsed_milliseconds() -> std::chrono::high_resolution_clock::duration::rep;
    auto elapsed_nanoseconds() -> std::chrono::high_resolution_clock::duration::rep;

private:
    std::chrono::high_resolution_clock _clock {};
    std::chrono::high_resolution_clock::duration _start {};
    std::chrono::high_resolution_clock::duration _stop {};
    bool _isRunning { false };
};
}