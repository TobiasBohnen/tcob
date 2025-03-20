// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_opus.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_OPUS)

    #include <algorithm>
    #include <iterator>
    #include <optional>
    #include <span>

    #include <opus_types.h>
    #include <opusenc.h>
    #include <opusfile.h>

    #include "tcob/audio/Buffer.hpp"
    #include "tcob/core/io/Stream.hpp"

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

extern "C" {
auto static read_opus(void* _stream, unsigned char* _ptr, int _nbytes) -> int
{
    auto* stream {static_cast<io::istream*>(_stream)};
    return static_cast<int>(stream->read_to<byte>({reinterpret_cast<byte*>(_ptr), static_cast<usize>(_nbytes)}));
}

auto static seek_opus(void* _stream, opus_int64 _offset, int _whence) -> int
{
    auto*      stream {static_cast<io::istream*>(_stream)};
    auto const dir {static_cast<io::seek_dir>(_whence)};
    stream->seek(_offset, dir);
    return 0;
}

auto static tell_opus(void* _stream) -> opus_int64
{
    io::istream* stream {static_cast<io::istream*>(_stream)};
    return static_cast<long>(stream->tell());
}

////////////////////////////////////////////////////////////

static OpusFileCallbacks opusCallbacks {
    .read  = &read_opus,
    .seek  = &seek_opus,
    .tell  = &tell_opus,
    .close = nullptr};
}

opus_decoder::opus_decoder() = default;

opus_decoder::~opus_decoder()
{
    if (_file) {
        op_free(_file);
        _file = nullptr;
    }
}

void opus_decoder::seek_from_start(milliseconds pos)
{
    f64 const offset {pos.count() / 1000 * _info.SampleRate / _info.Channels};
    op_pcm_seek(_file, static_cast<i64>(offset));
}

auto opus_decoder::open() -> std::optional<buffer::information>
{
    i32 err {0};
    _file = op_open_callbacks(&stream(), &opusCallbacks, nullptr, 0, &err);
    if (!_file) {
        return std::nullopt;
    }

    _info.Channels   = op_channel_count(_file, -1);
    _info.SampleRate = 48000;
    _info.FrameCount = op_pcm_total(_file, -1);
    return _info;
}

auto opus_decoder::decode(std::span<f32> outputSamples) -> i32
{
    i32 readOffset {0};

    for (;;) {
        if (std::ssize(outputSamples) - readOffset == 0) {
            break;
        }

        auto const readBuffer {outputSamples.subspan(static_cast<u32>(readOffset))};
        auto const read {op_read_float(_file, readBuffer.data(), static_cast<i32>(readBuffer.size()), nullptr)};
        readOffset += read * _info.Channels;
    }

    return readOffset;
}

////////////////////////////////////////////////////////////

extern "C" {
auto static write_opus(void* user_data, unsigned char const* ptr, opus_int32 len) -> int
{
    auto* stream {static_cast<io::ostream*>(user_data)};
    return static_cast<int>(stream->write<unsigned char>({ptr, static_cast<usize>(len)})) == len ? 0 : 1;
}

auto static close_opus(void* /* user_data */) -> int
{
    return 0;
}

static OpusEncCallbacks opusEncCallbacks {
    .write = &write_opus,
    .close = &close_opus};
}

////////////////////////////////////////////////////////////

auto opus_encoder::encode(std::span<f32 const> samples, buffer::information const& info, io::ostream& out) const -> bool
{
    OggOpusComments* comments {ope_comments_create()};

    i32   err {0};
    auto* encoder {ope_encoder_create_callbacks(&opusEncCallbacks, &out, comments, info.SampleRate, info.Channels, 0, &err)};
    if (!encoder) {
        ope_comments_destroy(comments);
        return false;
    }

    i32 readOffset {0};
    for (;;) {
        i32 const read {std::min(1024, static_cast<i32>(samples.size()) - readOffset)};
        if (read <= 0) {
            break;
        }

        auto const readBuffer {samples.subspan(static_cast<u32>(readOffset), static_cast<u32>(read))};
        readOffset += read;
        ope_encoder_write_float(encoder, readBuffer.data(), static_cast<i32>(readBuffer.size() / static_cast<u32>(info.Channels)));
    }

    ope_encoder_drain(encoder);
    ope_encoder_destroy(encoder);
    ope_comments_destroy(comments);

    return true;
}
}

#endif
