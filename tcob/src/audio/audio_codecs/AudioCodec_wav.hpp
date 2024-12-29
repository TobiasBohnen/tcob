// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)

    #include <optional>

    #define DR_WAV_NO_STDIO
    #include <dr_libs/dr_wav.h>

    #include "tcob/audio/Buffer.hpp"

namespace tcob::audio::detail {
////////////////////////////////////////////////////////////

class wav_decoder final : public decoder {
public:
    ~wav_decoder() override;

    void seek_from_start(milliseconds pos) override;

protected:
    auto open() -> std::optional<buffer::information> override;
    auto decode(std::span<f32> outputSamples) -> i32 override;

private:
    buffer::information _info {};
    drwav               _wav {};
};

////////////////////////////////////////////////////////////

class wav_encoder final : public encoder {
public:
    auto encode(std::span<f32 const> samples, buffer::information const& info, io::ostream& out) const -> bool override;
};

}

#endif
