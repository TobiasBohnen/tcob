// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

namespace tcob {
class AudioBuffer final {
public:
    AudioBuffer();
    ~AudioBuffer();

    auto load(const std::string& filename) -> bool;

    void play();
    void stop();

private:
    u32 _source { 0 };
    u32 _buffer { 0 };
};
}