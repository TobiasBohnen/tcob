// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#define DR_FLAC_NO_STDIO
#include <dr_libs/dr_flac.h>
#define DR_MP3_NO_STDIO
#include <dr_libs/dr_mp3.h>
#define DR_WAV_NO_STDIO
#include <dr_libs/dr_wav.h>
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>

#include <tcob/sfx/Music.hpp>

namespace tcob::detail {
class WavDecoder final : public AudioDecoder {
public:
    WavDecoder(const std::string& filename);
    ~WavDecoder();

    auto info() const -> AudioInfo override;

    auto seek(f32 pos) -> bool override;

protected:
    auto read_data(i16* data, i32& frameCount) -> bool override;

private:
    AudioInfo _info;
    drwav _wav;
};
}