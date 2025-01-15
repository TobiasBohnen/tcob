// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)

    #include "tcob/audio/Buffer.hpp"
    #include "tcob/audio/synth/SoundFont.hpp"
    #include "tcob/core/assets/Asset.hpp"

namespace tcob::audio::detail {
////////////////////////////////////////////////////////////

class midi_decoder final : public decoder {
public:
    midi_decoder();
    ~midi_decoder() override;

    void seek_from_start(milliseconds pos) override;

protected:
    auto open() -> std::optional<buffer::information> override;
    auto decode(std::span<f32> outputSamples) -> i32 override;

private:
    buffer::information _info {};

    assets::asset_ptr<sound_font> _font;
    tml_message*                  _firstMessage {nullptr};
    tml_message*                  _currentMessage {nullptr};
    f64                           _currentTime {0.0};
};

}

#endif
