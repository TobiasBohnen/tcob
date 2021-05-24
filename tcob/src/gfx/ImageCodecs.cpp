// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodecs.hpp"

#include <tcob/core/io/FileStream.hpp>

namespace tcob {

PngDecoder::PngDecoder(const std::vector<u8>& filedata)
    : _buffer { filedata }
    , _context { spng_ctx_new(0) }
{
    spng_set_crc_action(_context, SPNG_CRC_USE, SPNG_CRC_USE);

    isize limit { 1024 * 1024 * 64 };
    spng_set_chunk_limits(_context, limit, limit);

    spng_set_png_buffer(_context, _buffer.data(), _buffer.size());
}

PngDecoder::~PngDecoder()
{
    spng_ctx_free(_context);
}

auto PngDecoder::info(ImageInfo& info) -> bool
{
    spng_ihdr ihdr;
    if (spng_get_ihdr(_context, &ihdr) == 0) {
        info.SizeInPixels = { ihdr.width, ihdr.height };
        info.SizeInBytes = ihdr.height * ihdr.width * 4;
        info.Stride = ihdr.width * 4;
        info.Channels = 4;
        return true;
    }

    return false;
}

auto PngDecoder::decode() -> Image
{
    if (_alreadyDecoded) {
        return _image;
    }

    spng_ihdr ihdr;
    if (spng_get_ihdr(_context, &ihdr) == 0) {
        isize outSize;
        if (spng_decoded_image_size(_context, SPNG_FMT_RGBA8, &outSize) == 0) {
            std::vector<u8> data {};
            data.reserve(outSize);
            if (spng_decode_image(_context, data.data(), outSize, SPNG_FMT_RGBA8, 0) == 0) {
                _image = { ihdr.width, ihdr.height, 4, data.data() };
                _alreadyDecoded = true;
                return _image;
            }
        }
    }

    return {};
}

auto PngDecoder::is_valid() -> bool
{
    spng_ihdr ihdr;
    return spng_get_ihdr(_context, &ihdr) == 0;
}

////////////////////////////////////////////////////////////

WebpDecoder::WebpDecoder(const std::vector<u8>& filedata)
    : _buffer { filedata }
{
}

WebpDecoder::~WebpDecoder() = default;

auto WebpDecoder::info(ImageInfo& info) -> bool
{
    i32 w {}, h {};
    if (WebPGetInfo(_buffer.data(), _buffer.size(), &w, &h)) { //try webp
        info.SizeInPixels = { static_cast<u32>(w), static_cast<u32>(h) };
        info.SizeInBytes = static_cast<u32>(h) * static_cast<u32>(w) * 4;
        info.Stride = static_cast<u32>(w) * 4;
        info.Channels = 4;
        return true;
    }

    return false;
}

auto WebpDecoder::decode() -> Image
{
    i32 w {}, h {};
    auto data { WebPDecodeRGBA(_buffer.data(), _buffer.size(), &w, &h) };
    if (data) {
        Image img { static_cast<u32>(w), static_cast<u32>(h), 4, data };
        WebPFree(data);
        return img;
    }

    return {};
}

auto WebpDecoder::is_valid() -> bool
{
    return WebPGetInfo(_buffer.data(), _buffer.size(), nullptr, nullptr);
}

////////////////////////////////////////////////////////////

WebpEncoder::WebpEncoder(const Image& image)
    : _image { image }
{
}

void WebpEncoder::encode(const std::string& filename, bool flip)
{
    u8* output;
    isize outputSize { 0 };

    auto info { _image.info() };
    auto imageBuffer { _image.buffer() };

    if (!flip) {
        outputSize = WebPEncodeLosslessRGBA(imageBuffer, info.SizeInPixels.Width, info.SizeInPixels.Height, info.Stride, &output);
    } else {
        std::vector<u8> imageData {};
        imageData.reserve(info.SizeInBytes);

        for (i32 y { static_cast<i32>(info.SizeInPixels.Height - 1) }; y >= 0; --y) {
            auto start { y * info.Stride };
            auto end { (y + 1) * info.Stride };
            imageData.insert(imageData.end(),
                imageBuffer + start,
                imageBuffer + end);
        }

        outputSize = WebPEncodeLosslessRGBA(imageData.data(), info.SizeInPixels.Width, info.SizeInPixels.Height, info.Stride, &output);
    }

    OutputFileStreamU fs { filename };
    fs.write(output, outputSize);

    WebPFree(output);
}

}