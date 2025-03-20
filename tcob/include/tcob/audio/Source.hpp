// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <memory>

#include "tcob/audio/Audio.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API source : public non_copyable {
public:
    source();
    virtual ~source();

    std::any DecoderContext;

    prop<f32> Volume;

    auto virtual duration() const -> milliseconds = 0;
    auto status() const -> playback_status;

    void play();
    void stop();
    void restart();

    void pause();
    void resume();
    void toggle_pause();

protected:
    auto virtual on_start() -> bool = 0;
    auto virtual on_stop() -> bool  = 0;

    void create_output(buffer::information const& info);
    auto get_output() -> detail::audio_stream&;

private:
    std::unique_ptr<detail::audio_stream> _output;
};

}
