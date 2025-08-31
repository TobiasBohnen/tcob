// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_mp3.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)

    #include <optional>
    #include <span>

    #include "tcob/audio/Buffer.hpp"
    #include "tcob/core/io/Stream.hpp"

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

extern "C" {
static auto read_mp3(void* userdata, void* buffer, usize bytesToRead) -> usize
{
    auto* stream {static_cast<io::istream*>(userdata)};
    return static_cast<usize>(stream->read_to<byte>({static_cast<byte*>(buffer), bytesToRead}));
}

static auto seek_mp3(void* userdata, i32 offset, drmp3_seek_origin origin) -> drmp3_bool32
{
    auto*        stream {static_cast<io::istream*>(userdata)};
    io::seek_dir dir {};
    switch (origin) {
    case DRMP3_SEEK_SET: dir = io::seek_dir::Begin; break;
    case DRMP3_SEEK_CUR: dir = io::seek_dir::Current; break;
    case DRMP3_SEEK_END: dir = io::seek_dir::End; break;
    }
    return stream->seek(offset, dir);
}

static auto tell_mp3(void* userdata, drmp3_int64* pCursor) -> drmp3_bool32
{
    io::istream* stream {static_cast<io::istream*>(userdata)};
    *pCursor = static_cast<drmp3_int64>(stream->tell());
    return true;
}
}

////////////////////////////////////////////////////////////

mp3_decoder::~mp3_decoder()
{
    drmp3_uninit(&_mp3);
}

void mp3_decoder::seek_from_start(milliseconds pos)
{
    f64 const offset {pos.count() / 1000 * _info.Specs.SampleRate / _info.Specs.Channels};
    drmp3_seek_to_pcm_frame(&_mp3, static_cast<u64>(offset));
}

auto mp3_decoder::open() -> std::optional<buffer::information>
{
    if (drmp3_init(&_mp3, &read_mp3, &seek_mp3, &tell_mp3, nullptr, &stream(), nullptr)) {
        _info.Specs.Channels   = static_cast<i32>(_mp3.channels);
        _info.Specs.SampleRate = static_cast<i32>(_mp3.sampleRate);
        _info.FrameCount       = static_cast<i64>(drmp3_get_pcm_frame_count(&_mp3));
        return _info;
    }

    return std::nullopt;
}

auto mp3_decoder::decode(std::span<f32> outputSamples) -> isize
{
    u64 const wantRead {outputSamples.size() / static_cast<u32>(_info.Specs.Channels)};
    return static_cast<isize>(drmp3_read_pcm_frames_f32(&_mp3, wantRead, outputSamples.data()) * _info.Specs.Channels);
}
}

#endif
