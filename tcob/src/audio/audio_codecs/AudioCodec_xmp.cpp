// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_xmp.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_LIBXMP)

    #include <iterator>
    #include <limits>
    #include <optional>
    #include <span>
    #include <vector>

    #include <xmp.h>

    #include "tcob/audio/Buffer.hpp"
    #include "tcob/core/io/Stream.hpp"

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

extern "C" {
auto static read_xmp(void* dest, unsigned long len, unsigned long nmemb, void* priv) -> unsigned long
{
    auto* stream {static_cast<io::istream*>(priv)};
    return static_cast<unsigned long>(stream->read_to<byte>({static_cast<byte*>(dest), len * nmemb}) / len);
}

auto static seek_xmp(void* priv, long offset, int whence) -> int
{
    auto*      stream {static_cast<io::istream*>(priv)};
    auto const dir {static_cast<io::seek_dir>(whence)};
    stream->seek(offset, dir);
    return 0;
}

auto static tell_xmp(void* priv) -> long
{
    io::istream* stream {static_cast<io::istream*>(priv)};
    return static_cast<long>(stream->tell());
}

static xmp_callbacks xmpCallbacks {
    .read_func  = &read_xmp,
    .seek_func  = &seek_xmp,
    .tell_func  = &tell_xmp,
    .close_func = nullptr};
}

////////////////////////////////////////////////////////////

xmp_decoder::xmp_decoder()
    : _context {xmp_create_context()}
{
}

xmp_decoder::~xmp_decoder()
{
    xmp_release_module(_context);
    xmp_end_player(_context);
    xmp_free_context(_context);
}

void xmp_decoder::seek_from_start(milliseconds pos)
{
    xmp_seek_time(_context, static_cast<i32>(pos.count()));
    xmp_play_buffer(_context, nullptr, 0, 0);
}

auto xmp_decoder::open() -> std::optional<buffer::information>
{
    if (xmp_load_module_from_callbacks(_context, &stream(), xmpCallbacks) == 0) {
        xmp_module_info info {};
        xmp_get_module_info(_context, &info);
        _info.Specs.Channels   = 2;
        _info.Specs.SampleRate = 44100;

        xmp_frame_info mi {};
        xmp_get_frame_info(_context, &mi);
        _info.FrameCount = static_cast<i64>((static_cast<f32>(mi.total_time) / 1000.0f) * static_cast<f32>(_info.Specs.SampleRate));

        xmp_start_player(_context, _info.Specs.SampleRate, 0);

        return _info;
    }

    return std::nullopt;
}

auto xmp_decoder::decode(std::span<f32> outputSamples) -> i32
{
    std::vector<i16> buffer(outputSamples.size());
    auto const       res {xmp_play_buffer(_context, buffer.data(), static_cast<i32>(buffer.size() * sizeof(i16)), 1)};
    for (usize i {0}; i < outputSamples.size(); ++i) {
        outputSamples[i] = static_cast<f32>(buffer[i]) / static_cast<f32>(std::numeric_limits<i16>::max());
    }

    return res == 0 ? static_cast<i32>(std::ssize(outputSamples)) : 0;
}

}

#endif
