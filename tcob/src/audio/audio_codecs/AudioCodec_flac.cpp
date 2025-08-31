// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_flac.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)

    #include <cstddef>
    #include <optional>
    #include <span>

    #include "tcob/audio/Buffer.hpp"
    #include "tcob/core/io/Stream.hpp"

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

extern "C" {
static auto read_flac(void* userdata, void* buffer, size_t bytesToRead) -> usize
{
    auto* stream {static_cast<io::istream*>(userdata)};
    return static_cast<size_t>(stream->read_to<unsigned char>({static_cast<unsigned char*>(buffer), bytesToRead}));
}

static auto seek_flac(void* userdata, int offset, drflac_seek_origin origin) -> drflac_bool32
{
    auto*        stream {static_cast<io::istream*>(userdata)};
    io::seek_dir dir {};
    switch (origin) {
    case DRFLAC_SEEK_SET: dir = io::seek_dir::Begin; break;
    case DRFLAC_SEEK_CUR: dir = io::seek_dir::Current; break;
    case DRFLAC_SEEK_END: dir = io::seek_dir::End; break;
    }
    return stream->seek(offset, dir);
}

static auto tell_flac(void* userdata, drflac_int64* pCursor) -> drflac_bool32
{
    io::istream* stream {static_cast<io::istream*>(userdata)};
    *pCursor = static_cast<drflac_int64>(stream->tell());
    return true;
}
}

////////////////////////////////////////////////////////////

flac_decoder::~flac_decoder()
{
    drflac_close(_flac);
}

void flac_decoder::seek_from_start(milliseconds pos)
{
    f64 const offset {pos.count() / 1000 * _info.Specs.SampleRate / _info.Specs.Channels};
    drflac_seek_to_pcm_frame(_flac, static_cast<u64>(offset));
}

auto flac_decoder::open() -> std::optional<buffer::information>
{
    _flac = drflac_open(&read_flac, &seek_flac, &tell_flac, &stream(), nullptr);
    if (_flac) {
        _info.Specs.Channels   = _flac->channels;
        _info.Specs.SampleRate = static_cast<i32>(_flac->sampleRate);
        _info.FrameCount       = static_cast<i64>(_flac->totalPCMFrameCount);
        return _info;
    }

    return std::nullopt;
}

auto flac_decoder::decode(std::span<f32> outputSamples) -> isize
{
    u64 const wantRead {outputSamples.size() / static_cast<u32>(_info.Specs.Channels)};
    return static_cast<isize>(drflac_read_pcm_frames_f32(_flac, wantRead, outputSamples.data()) * _info.Specs.Channels);
}

}

#endif
