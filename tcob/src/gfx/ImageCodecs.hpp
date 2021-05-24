// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <spng/spng.h>
#include <webp/decode.h>
#include <webp/encode.h>

#include <tcob/gfx/Image.hpp>

namespace tcob {
class PngDecoder final {
public:
    PngDecoder(const std::vector<u8>& filedata);
    ~PngDecoder();

    auto info(ImageInfo& info) -> bool;
    auto decode() -> Image;
    auto is_valid() -> bool;

private:
    const std::vector<u8>& _buffer;
    spng_ctx* _context;

    Image _image {};
    bool _alreadyDecoded { false };
};

////////////////////////////////////////////////////////////

class WebpDecoder final {
public:
    WebpDecoder(const std::vector<u8>& filedata);
    ~WebpDecoder();

    auto info(ImageInfo& info) -> bool;
    auto decode() -> Image;
    auto is_valid() -> bool;

private:
    const std::vector<u8>& _buffer;
};

////////////////////////////////////////////////////////////

class WebpEncoder final {
public:
    WebpEncoder(const Image& image);

    void encode(const std::string& filename, bool flip);

private:
    const Image& _image;
};

}