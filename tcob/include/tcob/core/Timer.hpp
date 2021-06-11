// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/thirdparty/sigslot/signal.hpp>

namespace tcob {
class Timer final {
public:
    Timer();
    ~Timer();

    sigslot::signal<MilliSeconds> Tick;

    void start(MilliSeconds interval);
    void stop();

    auto is_running() const -> bool;

private:
    i32 _id { 0 };
};
}