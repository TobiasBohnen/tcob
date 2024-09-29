// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_flac.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

extern "C" {
auto static read_flac(void* userdata, void* buffer, usize bytesToRead) -> usize
{
    auto* stream {static_cast<istream*>(userdata)};
    return static_cast<usize>(stream->read_to<byte>({static_cast<byte*>(buffer), bytesToRead}));
}

auto static seek_flac(void* userdata, i32 offset, drflac_seek_origin origin) -> drflac_bool32
{
    auto*      stream {static_cast<istream*>(userdata)};
    auto const dir {origin == drflac_seek_origin_current ? io::seek_dir::Current : io::seek_dir::Begin};
    return stream->seek(offset, dir);
}
}

////////////////////////////////////////////////////////////

flac_decoder::~flac_decoder()
{
    drflac_close(_flac);
}

void flac_decoder::seek_from_start(milliseconds pos)
{
    f64 const offset {pos.count() / 1000 * _info.SampleRate / _info.Channels};
    drflac_seek_to_pcm_frame(_flac, static_cast<u64>(offset));
}

auto flac_decoder::open() -> std::optional<buffer::info>
{
    _flac = drflac_open(&read_flac, &seek_flac, &get_stream(), nullptr);
    if (_flac) {
        _info.Channels   = _flac->channels;
        _info.SampleRate = static_cast<i32>(_flac->sampleRate);
        _info.FrameCount = static_cast<i64>(_flac->totalPCMFrameCount);
        return _info;
    }

    return std::nullopt;
}

auto flac_decoder::decode(std::span<f32> outputSamples) -> i32
{
    u64 const wantRead {outputSamples.size() / static_cast<u32>(_info.Channels)};
    return static_cast<i32>(drflac_read_pcm_frames_f32(_flac, wantRead, outputSamples.data()));
}

}

#endif
