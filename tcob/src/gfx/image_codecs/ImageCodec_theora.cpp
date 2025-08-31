// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_theora.hpp"
#if defined(TCOB_ENABLE_FILETYPES_GFX_THEORA)

    #include <optional>
    #include <span>
    #include <thread>

    #include <theoraplay.h>

    #include "tcob/core/io/Stream.hpp"
    #include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {

extern "C" {
static auto read(THEORAPLAY_Io* io, void* buf, long buflen) -> long
{
    auto* stream {static_cast<io::istream*>(io->userdata)};
    return static_cast<long>(stream->read_to<byte>({static_cast<byte*>(buf), static_cast<usize>(buflen)}));
}

static auto streamlen(THEORAPLAY_Io* io) -> long
{
    auto* stream {static_cast<io::istream*>(io->userdata)};
    return static_cast<long>(stream->size_in_bytes());
}

static auto seek(THEORAPLAY_Io* io, long absolute_offset) -> int
{
    auto* stream {static_cast<io::istream*>(io->userdata)};
    return stream->seek(absolute_offset, io::seek_dir::Begin) ? 0 : -1;
}

static void close(THEORAPLAY_Io*)
{
    // stream is closed in theora_anim_decoder destructor
}
}

theora_decoder::theora_decoder()
{
    _io.read      = &read;
    _io.streamlen = &streamlen;
    _io.seek      = &seek;
    _io.close     = &close;
}

theora_decoder::~theora_decoder()
{
    if (_currentFrame) {
        THEORAPLAY_freeVideo(_currentFrame);
    }

    if (_decoder) {
        THEORAPLAY_stopDecode(_decoder);
    }
}

constexpr i32 MAX_FRAMES {20};

    #if !defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
constexpr bool MULTI_THREADED {true};
    #else
constexpr bool MULTI_THREADED {false};
    #endif

auto theora_decoder::open() -> std::optional<image::information>
{
    _io.userdata = &stream();
    _decoder     = THEORAPLAY_startDecode(&_io, MAX_FRAMES, THEORAPLAY_VIDFMT_RGBA, nullptr, MULTI_THREADED);
    if (_decoder) {
        // wait til initialized
        while (!THEORAPLAY_isInitialized(_decoder)) {
            std::this_thread::yield();
        }

        // check for video stream
        if (!THEORAPLAY_hasVideoStream(_decoder)) { return std::nullopt; }

        // get first frame for size
        THEORAPLAY_VideoFrame const* video {nullptr};
        while (!video) {
            video = THEORAPLAY_getVideo(_decoder);
            std::this_thread::yield();
        }
        _size = {static_cast<i32>(video->width), static_cast<i32>(video->height)};
        THEORAPLAY_freeVideo(video);
        return image::information {.Size = _size, .Format = image::format::RGBA};
    }

    return std::nullopt;
}

auto theora_decoder::current_frame() const -> std::span<u8 const>
{
    if (!_currentFrame) { return {}; }
    return {_currentFrame->pixels, _currentFrame->height * _currentFrame->width * 4};
}

auto theora_decoder::advance(milliseconds ts) -> animated_image_decoder::status
{
    auto timestamp {static_cast<i32>(ts.count())};
    if (!_decoder) { return animated_image_decoder::status::DecodeFailure; }
    if (!THEORAPLAY_isDecoding(_decoder)) { return animated_image_decoder::status::NoMoreFrames; }
    if (timestamp <= _currentTimeStamp) { return animated_image_decoder::status::OldFrame; }

    while (timestamp > _currentTimeStamp) {
        if (_currentFrame) { THEORAPLAY_freeVideo(_currentFrame); }

        _currentFrame = THEORAPLAY_getVideo(_decoder);
        if (_currentFrame) {
            _currentTimeStamp = _currentFrame->playms;
        } else {
            return animated_image_decoder::status::NoMoreFrames;
        }
    }

    return animated_image_decoder::status::NewFrame;
}

void theora_decoder::reset()
{
    _currentTimeStamp = 0;
    if (_decoder) {
        THEORAPLAY_stopDecode(_decoder);

        stream().seek(0, io::seek_dir::Begin); // FIXME: store position
        _decoder = THEORAPLAY_startDecode(&_io, MAX_FRAMES, THEORAPLAY_VIDFMT_RGBA, nullptr, MULTI_THREADED);
        while (!THEORAPLAY_isInitialized(_decoder)) {
            std::this_thread::yield();
        }
    }
}

}

#endif
