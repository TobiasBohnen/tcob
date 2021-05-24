// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <future>

#include <tcob/core/data/Size.hpp>

namespace tcob {
struct ImageInfo {
    SizeU SizeInPixels { SizeU::Zero };
    u32 SizeInBytes { 0 };
    u32 Stride { 0 };
    u32 Channels { 0 };
};

class Image final {
public:
    Image() = default;
    Image(u32 width, u32 height, u32 channels, const u8* data);
    ~Image() = default;

    auto save(const std::string& filename, bool flip = true) const -> bool;
    void save_async(const std::string& filename, bool flip = true) const;

    auto buffer() const -> const u8*;
    auto buffer() -> u8*;

    auto info() const -> ImageInfo;

    static auto CreateFromBuffer(const SizeU& size, u32 channels, const u8* data) -> Image;
    static auto Load(const std::string& filename) -> Image;
    static auto LoadAsync(const std::string& filename) -> std::future<Image>;
    static auto Info(const std::string& filename) -> ImageInfo;

private:
    std::vector<u8> _imageBuffer;
    ImageInfo _info;
};
}