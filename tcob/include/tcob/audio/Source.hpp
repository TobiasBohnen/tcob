// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>

#include "tcob/audio/Audio.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Property.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API source {
public:
    source();
    source(source const& other) noexcept;
    auto operator=(source const& other) noexcept -> source&;
    virtual ~source();

    std::any DecoderContext;

    prop_fn<f32> Volume;

    auto virtual duration() const -> milliseconds          = 0;
    auto virtual playback_position() const -> milliseconds = 0;
    auto get_status() const -> playback_status; // TODO: get_
    auto is_looping() const -> bool;

    void play(bool looping = false);
    void stop();
    void restart();

    void pause();
    void resume();
    void toggle_pause();

protected:
    auto virtual on_start() -> bool = 0;
    auto virtual on_stop() -> bool  = 0;

    auto get_source() const -> audio::al::al_source*;

private:
    std::shared_ptr<audio::al::al_source> _source;
};

}
