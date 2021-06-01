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

auto WavDecoder::seek(f32 pos) -> bool
{
    f32 offset { pos / 1000 * _info.Frequency / _info.Channels };
    return drwav_seek_to_pcm_frame(&_wav, static_cast<u64>(offset));
}

auto WavDecoder::read_data(i16* data, i32& frameCount) -> bool
{
    u64 wantRead { MUSIC_BUFFER_SIZE / _info.Channels / sizeof(i16) };
    frameCount = static_cast<i32>(drwav_read_pcm_frames_s16(&_wav, wantRead, data)); //assumes short frames
    return frameCount > 0;
}

////////////////////////////////////////////////////////////

FlacDecoder::FlacDecoder(const std::string& filename)
    : AudioDecoder { filename }
{
    _flac = drflac_open(&read, &seek_flac, stream(), nullptr);
    if (_flac) {
        _info.Channels = _flac->channels;
        _info.Frequency = _flac->sampleRate;
    }
}

FlacDecoder::~FlacDecoder()
{
    drflac_close(_flac);
}

auto FlacDecoder::info() const -> AudioInfo
{
    return _info;
}

auto FlacDecoder::seek(f32 pos) -> bool
{
    f32 offset { pos / 1000 * _info.Frequency / _info.Channels };
    return drflac_seek_to_pcm_frame(_flac, static_cast<u64>(offset));
}

auto FlacDecoder::read_data(i16* data, i32& frameCount) -> bool
{
    u64 wantRead { MUSIC_BUFFER_SIZE / _info.Channels / sizeof(i16) };
    frameCount = static_cast<i32>(drflac_read_pcm_frames_s16(_flac, wantRead, data)); //assumes short frames
    return frameCount > 0;
}

////////////////////////////////////////////////////////////

Mp3Decoder::Mp3Decoder(const std::string& filename)
    : AudioDecoder { filename }
{
    if (drmp3_init(&_mp3, &read, &seek_mp3, stream(), nullptr)) {
        _info.Channels = _mp3.channels;
        _info.Frequency = _mp3.sampleRate;
    }
}

Mp3Decoder::~Mp3Decoder()
{
    drmp3_uninit(&_mp3);
}

auto Mp3Decoder::info() const -> AudioInfo
{
    return _info;
}

auto Mp3Decoder::seek(f32 pos) -> bool
{
    f32 offset { pos / 1000 * _info.Frequency / _info.Channels };
    return drmp3_seek_to_pcm_frame(&_mp3, static_cast<u64>(offset));
}

auto Mp3Decoder::read_data(i16* data, i32& frameCount) -> bool
{
    u64 wantRead { MUSIC_BUFFER_SIZE / _info.Channels / sizeof(i16) };
    frameCount = static_cast<i32>(drmp3_read_pcm_frames_s16(&_mp3, wantRead, data)); //assumes short frames
    return frameCount > 0;
}

}