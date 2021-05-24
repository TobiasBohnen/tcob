// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/Image.hpp>

#include "ImageCodecs.hpp"

#include <tcob/core/io/FileStream.hpp>

namespace tcob {

Image::Image(u32 width, u32 height, u32 channels, const u8* data)
    : _imageBuffer { std::vector<u8> { data, data + (width * height * channels) } }
    , _info {
        .SizeInPixels = { width, height },
        .SizeInBytes = height * width * channels,
        .Stride = width * channels,
        .Channels = channels
    }
{
}

auto Image::save(const std::string& filename, bool flip) const -> bool
{
    if (_info.Channels == 4) {
        WebpEncoder { *this }.encode(filename, flip);
        return true;
    }
    return false;
}

void Image::save_async(const std::string& filename, bool flip) const
{
    std::thread([*this, filename, flip]() { save(filename, flip); }).detach();
}

auto Image::buffer() const -> const u8*
{
    return _imageBuffer.data();
}

auto Image::buffer() -> u8*
{
    return _imageBuffer.data();
}

auto Image::info() const -> ImageInfo
{
    return _info;
}

auto Image::CreateFromBuffer(const SizeU& size, u32 channels, const u8* data) -> Image
{
    return { size.Width, size.Height, channels, data };
}

auto Image::Load(const std::string& filename) -> Image
{
    if (FileSystem::is_file(filename)) {
        InputFileStreamU stream { filename };
        std::string ext { FileSystem::extension(filename) };

        auto buffer { stream.read_all() };

        if (ext == ".webp") {
            WebpDecoder webp { buffer };
            if (webp.is_valid()) { //try webp
                return webp.decode();
            }
        } else if (ext == ".png") { //try png
            PngDecoder png { buffer };
            if (png.is_valid()) {
                return png.decode();
            }
        }
    }

    //TODO: log error
    return {};
}

auto Image::LoadAsync(const std::string& filename) -> std::future<Image>
{
    return std::async(std::launch::async,
        [filename] {
            return Image::Load(filename);
        });
}

auto Image::Info(const std::string& filename) -> ImageInfo
{
    if (FileSystem::is_file(filename)) {
        ImageInfo retValue {};

        InputFileStreamU stream { filename };
        auto buffer { stream.read_all() };

        if (WebpDecoder { buffer }.info(retValue)) {
            return retValue;
        } else if (PngDecoder { buffer }.info(retValue)) {
            return retValue;
        }

        return retValue;
    }

    //TODO: log error
    return {};
}
}