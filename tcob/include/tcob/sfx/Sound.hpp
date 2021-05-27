// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/sfx/ALObjects.hpp>

namespace tcob {
class Sound final {
public:
    Sound();
    ~Sound();

    Sound(const Sound& other);
    auto operator=(const Sound& other) -> Sound&;

    auto load(const std::string& filename) -> bool;

    void play();
    void stop();

    auto duration() const -> f32;
    auto playback_position() const -> f32;

private:
    std::shared_ptr<al::Buffer> _buffer;
    std::unique_ptr<al::Source> _source;
};
}