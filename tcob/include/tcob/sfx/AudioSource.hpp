// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/sfx/ALObjects.hpp>

namespace tcob {
class AudioSource {
public:
    AudioSource();
    virtual ~AudioSource();

    virtual void start(bool looped = false) = 0;
    virtual void stop() = 0;
    void restart();
    void toggle_pause();

    auto volume() const -> f32;
    void volume(f32 vol) const;

    auto state() const -> AudioState;

    virtual auto duration() const -> f32 = 0;
    virtual auto playback_position() const -> f32 = 0;

protected:
    auto source() const -> al::Source*;

private:
    std::unique_ptr<al::Source> _source;
};
}