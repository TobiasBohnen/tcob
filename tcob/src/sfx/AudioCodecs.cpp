// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodecs.hpp"

#include "AudioIO.hpp"
#include <tcob/core/io/FileStream.hpp>

namespace tcob::detail {

WavDecoder::WavDecoder(const std::string& filename)
    : AudioDecoder { filename }
{
    if (drwav_init(&_wav, &read, &seek_wav, stream(), nullptr)) {
        _info.Channels = _wav.channels;
        _info.Frequency = _wav.sampleRate;
    }
}

WavDecoder::~WavDecoder()
{
    drwav_uninit(&_wav);
}

auto WavDecoder::info() const -> AudioInfo
{
    return _info;
}

auto WavDecoder::read_data(i16* data, i32& frameCount) -> bool
{
    u64 wantRead { MUSIC_BUFFER_SIZE / _info.Channels / sizeof(i16) };
    u64 haveRead { drwav_read_pcm_frames_s16(&_wav, wantRead, data) }; //assumes short frames
    frameCount = static_cast<i32>(haveRead);
    return wantRead == haveRead;
}

}