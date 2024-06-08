// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_mp3.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

extern "C" {
auto static read_mp3(void* userdata, void* buffer, usize bytesToRead) -> usize
{
    auto* stream {static_cast<istream*>(userdata)};
    return static_cast<usize>(stream->read_to<byte>({static_cast<byte*>(buffer), bytesToRead}));
}

auto static seek_mp3(void* userdata, i32 offset, drmp3_seek_origin origin) -> drmp3_bool32
{
    auto*      stream {static_cast<istream*>(userdata)};
    auto const dir {origin == drmp3_seek_origin_current ? io::seek_dir::Current : io::seek_dir::Begin};
    return stream->seek(offset, dir);
}
}

////////////////////////////////////////////////////////////

mp3_decoder::~mp3_decoder()
{
    drmp3_uninit(&_mp3);
}

void mp3_decoder::seek_from_start(milliseconds pos)
{
    f64 const offset {pos.count() / 1000 * _info.SampleRate / _info.Channels};
    drmp3_seek_to_pcm_frame(&_mp3, static_cast<u64>(offset));
}

auto mp3_decoder::open() -> std::optional<buffer::info>
{
    if (drmp3_init(&_mp3, &read_mp3, &seek_mp3, &get_stream(), nullptr)) {
        _info.Channels   = _mp3.channels;
        _info.SampleRate = _mp3.sampleRate;
        _info.FrameCount = drmp3_get_pcm_frame_count(&_mp3);
        return _info;
    }

    return std::nullopt;
}

auto mp3_decoder::decode(std::span<f32> outputSamples) -> i32
{
    u64 const wantRead {outputSamples.size() / _info.Channels};
    return static_cast<i32>(drmp3_read_pcm_frames_f32(&_mp3, wantRead, outputSamples.data()));
}
}

#endif
