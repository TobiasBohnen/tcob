// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/sfx/ALObjects.hpp>
#include <tcob/sfx/AudioSource.hpp>

namespace tcob {
class Sound final : public AudioSource {
public:
    Sound();
    ~Sound();

    auto load(const std::string& filename) -> bool;

    void start(bool looped = false) override;
    void stop() override;

    auto duration() const -> f32;
    auto playback_position() const -> f32;

private:
    std::shared_ptr<al::Buffer> _buffer;
};
}