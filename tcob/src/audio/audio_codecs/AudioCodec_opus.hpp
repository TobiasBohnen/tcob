// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_OPUS)

    #include <optional>

    #include <opusenc.h>
    #include <opusfile.h>

    #include "tcob/audio/AudioSource.hpp"
    #include "tcob/core/io/Stream.hpp"

namespace tcob::audio::detail {
////////////////////////////////////////////////////////////

class opus_decoder final : public decoder {
public:
    opus_decoder();
    ~opus_decoder() override;

    void seek_from_start(milliseconds pos) override;

protected:
    auto open() -> std::optional<buffer::info> override;
    auto decode(std::span<f32> outputSamples) -> i32 override;

private:
    buffer::info _info {};
    OggOpusFile* _file {nullptr};
};

////////////////////////////////////////////////////////////

class opus_encoder final : public encoder {
public:
    auto encode(std::span<f32 const> samples, buffer::info const& info, ostream& out) const -> bool override;
};

}

#endif
