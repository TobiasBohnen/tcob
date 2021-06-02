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
    virtual void restart() = 0;
    virtual void toggle_pause() = 0;
    virtual void stop() = 0;

    auto volume() const -> f32;
    void volume(f32 vol) const;

protected:
    auto source() const -> al::Source*;

private:
    std::unique_ptr<al::Source> _source;
};
}