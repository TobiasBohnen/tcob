// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_VORBIS)

    #include <optional>

    #include <vorbis/vorbisenc.h>
    #include <vorbis/vorbisfile.h>

    #include "tcob/audio/AudioSource.hpp"
    #include "tcob/core/io/Stream.hpp"

namespace tcob::audio::detail {
////////////////////////////////////////////////////////////

class vorbis_decoder final : public decoder {
public:
    vorbis_decoder();
    ~vorbis_decoder() override;

    void seek_from_start(milliseconds pos) override;

protected:
    auto open() -> std::optional<buffer::info> override;
    auto decode(std::span<f32> outputSamples) -> i32 override;

private:
    buffer::info   _info {};
    OggVorbis_File _file {};
    i32            _section {0};
};

////////////////////////////////////////////////////////////

class vorbis_encoder final : public encoder {
public:
    auto encode(std::span<f32 const> samples, buffer::info const& info, ostream& out) const -> bool override;

private:
    void flush(ostream& out, ogg_stream_state& os, ogg_page& og, ogg_packet& op, vorbis_dsp_state& vd, vorbis_block& vb) const;
};

}

#endif
