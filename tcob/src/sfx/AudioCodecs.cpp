// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodecs.hpp"

#include <AL/al.h>

#include "AudioIO.hpp"
#include <tcob/core/io/FileStream.hpp>

extern "C" {
int filehack_fgetc(filehack* f)
{
    auto istream { reinterpret_cast<tcob::InputFileStream*>(f) };
    if (istream->eof()) {
        return EOF;
    } else {
        tcob::ubyte retValue { 0 };
        istream->read(reinterpret_cast<tcob::byte*>(&retValue), 1);
        return retValue;
    }
}

int filehack_fread(void* dst, int s, int c, filehack* f)
{
    auto istream { reinterpret_cast<tcob::InputFileStream*>(f) };
    return static_cast<int>(istream->read(reinterpret_cast<tcob::byte*>(dst), s * c)) / s;
}

int filehack_fseek(filehack* f, int idx, int base)
{
    auto istream { reinterpret_cast<tcob::InputFileStream*>(f) };
    static_cast<int>(istream->seek(idx, static_cast<std::ios_base::seekdir>(base)));
    return 0;
}

int filehack_ftell(filehack* f)
{
    auto istream { reinterpret_cast<tcob::InputFileStream*>(f) };
    return static_cast<int>(istream->tell());
}

int filehack_fclose(filehack*)
{
    return 0;
}
}

////////////////////////////////////////////////////////////

namespace tcob::detail {

detail::AudioDecoder::AudioDecoder(const std::string& filename)
    : _stream { std::make_unique<InputFileStream>(filename) }
{
}

auto detail::AudioDecoder::buffer_data(al::Buffer* buffer) -> bool
{
    std::vector<f32> data;
    data.reserve(MUSIC_BUFFER_SIZE);

    i32 sampleCount { read_data(data.data(), MUSIC_BUFFER_SIZE) };
    if (sampleCount > 0) {
        auto audioInfo { info() };
        buffer->buffer_data(data.data(), sampleCount, audioInfo.Channels, audioInfo.Frequency);
        return true;
    }
    return false;
}

auto detail::AudioDecoder::stream() const -> InputFileStream*
{
    return _stream.get();
}

////////////////////////////////////////////////////////////

WavDecoder::WavDecoder(const std::string& filename)
    : AudioDecoder { filename }
{
    if (drwav_init(&_wav, &read, &seek_wav, stream(), nullptr)) {
        _info.Channels = _wav.channels;
        _info.Frequency = _wav.sampleRate;
        _info.SampleCount = _wav.totalPCMFrameCount;
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

auto WavDecoder::seek(f32 duration) -> bool
{
    f32 offset { duration / 1000 * _info.Frequency / _info.Channels };
    return drwav_seek_to_pcm_frame(&_wav, static_cast<u64>(offset));
}

auto WavDecoder::read_data(f32* data, isize size) -> i32
{
    u64 wantRead { size / _info.Channels };
    return static_cast<i32>(drwav_read_pcm_frames_f32(&_wav, wantRead, data));
}

////////////////////////////////////////////////////////////

FlacDecoder::FlacDecoder(const std::string& filename)
    : AudioDecoder { filename }
{
    _flac = drflac_open(&read, &seek_flac, stream(), nullptr);
    if (_flac) {
        _info.Channels = _flac->channels;
        _info.Frequency = _flac->sampleRate;
        _info.SampleCount = _flac->totalPCMFrameCount;
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

auto FlacDecoder::seek(f32 duration) -> bool
{
    f32 offset { duration / 1000 * _info.Frequency / _info.Channels };
    return drflac_seek_to_pcm_frame(_flac, static_cast<u64>(offset));
}

auto FlacDecoder::read_data(f32* data, isize size) -> i32
{
    u64 wantRead { size / _info.Channels };
    return static_cast<i32>(drflac_read_pcm_frames_f32(_flac, wantRead, data));
}

////////////////////////////////////////////////////////////

Mp3Decoder::Mp3Decoder(const std::string& filename)
    : AudioDecoder { filename }
{
    if (drmp3_init(&_mp3, &read, &seek_mp3, stream(), nullptr)) {
        _info.Channels = _mp3.channels;
        _info.Frequency = _mp3.sampleRate;
        _info.SampleCount = drmp3_get_pcm_frame_count(&_mp3);
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

auto Mp3Decoder::seek(f32 duration) -> bool
{
    f32 offset { duration / 1000 * _info.Frequency / _info.Channels };
    return drmp3_seek_to_pcm_frame(&_mp3, static_cast<u64>(offset));
}

auto Mp3Decoder::read_data(f32* data, isize size) -> i32
{
    u64 wantRead { size / _info.Channels };
    return static_cast<i32>(drmp3_read_pcm_frames_f32(&_mp3, wantRead, data));
}

////////////////////////////////////////////////////////////

VorbisDecoder::VorbisDecoder(const std::string& filename)
    : AudioDecoder { filename }
{
    std::vector<byte> buffer {};
    auto istream { stream() };

    i32 error { 0 };
    _vorbis = stb_vorbis_open_file(reinterpret_cast<filehack*>(istream), false, &error, nullptr);

    auto info { stb_vorbis_get_info(_vorbis) };
    _info.Channels = info.channels;
    _info.Frequency = info.sample_rate;
    _info.SampleCount = stb_vorbis_stream_length_in_samples(_vorbis);
}

VorbisDecoder::~VorbisDecoder()
{
    stb_vorbis_close(_vorbis);
}

auto VorbisDecoder::info() const -> AudioInfo
{
    return _info;
}

auto VorbisDecoder::seek(f32 duration) -> bool
{
    if (duration == 0) {
        stb_vorbis_seek_start(_vorbis);
        return true;
    } else {
        f32 offset { duration / 1000 * _info.Frequency / _info.Channels };
        return stb_vorbis_seek_frame(_vorbis, static_cast<i32>(offset));
    }
}

auto VorbisDecoder::read_data(f32* data, isize size) -> i32
{
    return static_cast<i32>(stb_vorbis_get_samples_float_interleaved(_vorbis, _info.Channels, data, static_cast<i32>(size)));
}
}