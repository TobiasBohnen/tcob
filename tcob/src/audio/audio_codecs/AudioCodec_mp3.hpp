// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)

    #include <optional>
    #include <span>

    #define DR_MP3_NO_STDIO
    #include <dr_libs/dr_mp3.h>

    #include "tcob/audio/Buffer.hpp"

namespace tcob::audio::detail {
////////////////////////////////////////////////////////////

class mp3_decoder final : public decoder {
public:
    ~mp3_decoder() override;

    void seek_from_start(milliseconds pos) override;

    auto decode(std::span<f32> outputSamples) -> isize override;

protected:
    auto open() -> std::optional<buffer::information> override;

private:
    buffer::information _info {};
    drmp3               _mp3 {};
};

}

#endif
