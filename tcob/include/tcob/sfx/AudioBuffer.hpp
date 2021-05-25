// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/sfx/ALObjects.hpp>

namespace tcob {
class AudioBuffer final {
public:
    AudioBuffer();
    ~AudioBuffer();

    AudioBuffer(const AudioBuffer& other);
    auto operator=(const AudioBuffer& other) -> AudioBuffer& ;

    auto load(const std::string& filename) -> bool;

    void play();
    void stop();

private:
    std::shared_ptr<al::Buffer> _buffer;
    std::unique_ptr<al::Source> _source;
};
}