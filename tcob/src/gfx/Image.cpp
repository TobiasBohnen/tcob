// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Image.hpp"

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Semaphore.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::gfx {

image::image(size_i size, format f)
    : _info {size, f}
{
}

image::image(size_i size, format f, std::span<u8 const> data)
    : image {size, f}
{
    assert(std::ssize(data) == size.Width * size.Height * image::info::GetBPP(f));
    _buffer = {data.begin(), data.end()};
}

auto image::get_data() const -> std::span<u8 const>
{
    return _buffer;
}

auto image::get_data() -> std::span<u8>
{
    return _buffer;
}

auto image::get_data(rect_i const& bounds) const -> std::vector<u8>
{
    i32 const bpp {_info.bytes_per_pixel()};
    i32 const srcStride {_info.stride()};
    i32 const dstStride {bounds.Width * bpp};

    std::vector<u8> retValue;
    retValue.resize(bounds.Height * dstStride);

    // TODO: bounds check

    for (i32 y {bounds.Y}; y < bounds.bottom(); ++y) {
        u8 const* src {&_buffer[y * srcStride + bounds.X * bpp]};
        i32 const dst {(y - bounds.Y) * dstStride};
        std::copy(src, src + dstStride, retValue.begin() + dst);
    }

    return retValue;
}

auto image::get_info() const -> info const&
{
    return _info;
}

void image::flip_horizontally()
{
    i32 const stride {_info.stride()};
    i32 const bpp {_info.bytes_per_pixel()};

    auto const begin {_buffer.begin()};
    for (i32 y {0}; y < _info.Size.Height; ++y) {
        for (i32 x {0}; x < stride / 2; x += bpp) {
            i32 const start {(y * stride) + x};
            i32 const end {start + bpp};
            i32 const targetstart {((y + 1) * stride) - x - bpp};
            std::swap_ranges(
                begin + start, begin + end,
                begin + targetstart);
        }
    }
}

void image::flip_vertically()
{
    i32 const stride {_info.stride()};

    auto const begin {_buffer.begin()};
    for (i32 y {0}; y < _info.Size.Height / 2; ++y) {
        i32 const start {y * stride};
        i32 const end {start + stride};
        i32 const targetstart {(_info.Size.Height - y - 1) * stride};
        std::swap_ranges(
            begin + start, begin + end,
            begin + targetstart);
    }
}

void image::set_pixel(point_i pos, color c)
{
    assert(_info.Size.contains(pos));
    isize const idx {(pos.X + (pos.Y * _info.Size.Width)) * _info.bytes_per_pixel()};
    _buffer[idx + 0] = c.R;
    _buffer[idx + 1] = c.G;
    _buffer[idx + 2] = c.B;
    if (_info.Format == image::format::RGBA) {
        _buffer[idx + 3] = c.A;
    }
}

auto image::get_pixel(point_i pos) const -> color
{
    assert(_info.Size.contains(pos));
    isize const idx {(pos.X + (pos.Y * _info.Size.Width)) * _info.bytes_per_pixel()};
    u8 const    r {_buffer[idx + 0]};
    u8 const    g {_buffer[idx + 1]};
    u8 const    b {_buffer[idx + 2]};
    u8 const    a {_info.Format == image::format::RGBA ? _buffer[idx + 3] : static_cast<u8>(255)};
    return {r, g, b, a};
}

auto image::Create(size_i size, format f, std::span<u8 const> data) -> image
{
    return {size, f, data};
}

auto image::CreateEmpty(size_i size, format f) -> image
{
    image retValue {size, f};
    retValue._buffer.resize(size.Width * size.Height * image::info::GetBPP(f));
    return retValue;
}

auto image::Load(path const& file) noexcept -> std::optional<image>
{
    image retValue;
    if (retValue.load(file) == load_status::Ok) { return retValue; }

    return std::nullopt;
}

auto image::Load(istream& in, string const& ext) noexcept -> std::optional<image>
{
    image retValue;
    if (retValue.load(in, ext) == load_status::Ok) { return retValue; }

    return std::nullopt;
}

auto image::load(path const& file) noexcept -> load_status
{
    if (auto fs {io::ifstream::Open(file)}) {
        return load(*fs, io::get_extension(file));
    }

    return load_status::FileNotFound;
}

auto image::load(istream& in, string const& ext) noexcept -> load_status
{
    if (auto decoder {locate_service<image_decoder::factory>().create_from_sig_or_ext(in, ext)}) {
        if (auto img {decoder->decode(in)}) {
            std::swap(_buffer, img->_buffer);
            std::swap(_info, img->_info);
            return _info.Size != size_i::Zero ? load_status::Ok : load_status::Error;
        }
    }

    return load_status::Error;
}

auto image::load_async(path const& file) noexcept -> std::future<load_status>
{
    return std::async(std::launch::async, [&, file] {
        auto& sema {locate_service<semaphore>()};
        sema.acquire();
        auto retValue {load(file)};
        sema.release();
        return retValue;
    });
}

auto image::LoadInfo(path const& file) noexcept -> std::optional<info>
{
    if (auto fs {io::ifstream::Open(file)}) {
        if (auto decoder {locate_service<image_decoder::factory>().create_from_sig_or_ext(*fs, io::get_extension(file))}) {
            return decoder->decode_header(*fs);
        }
    }

    return std::nullopt;
}

auto image::save(path const& file) const -> bool
{
    io::ofstream of {file};
    return save(of, io::get_extension(file));
}

auto image::save(ostream& out, string const& ext) const -> bool
{
    if (_info.Format != image::format::RGBA
        && _info.Format != image::format::RGB) { // TODO:
        return false;
    }

    if (auto enc {locate_service<image_encoder::factory>().create(ext)}) {
        return enc->encode(*this, out);
    }

    return false;
}

auto image::save_async(path const& file) const -> std::future<bool>
{
    std::promise<bool> pro;
    auto               retValue {pro.get_future()};
    std::thread([*this, file](std::promise<bool> p) { p.set_value(save(file)); }, std::move(pro)).detach();
    return retValue;
}

////////////////////////////////////////////////////////////

auto image::info::size_in_bytes() const -> isize
{
    return Size.Height * Size.Width * static_cast<isize>(bytes_per_pixel());
}

auto image::info::bytes_per_pixel() const -> i32
{
    return GetBPP(Format);
}

auto image::info::stride() const -> i32
{
    return Size.Width * bytes_per_pixel(); // no padding bytes
}

auto image::info::GetBPP(format f) -> i32
{
    switch (f) {
    case image::format::RGB:
        return 3;
    case image::format::RGBA:
        return 4;
    }

    return 0;
}

////////////////////////////////////////////////////////////

auto animated_image_decoder::open(std::shared_ptr<istream> in) -> std::optional<image::info>
{
    _stream = std::move(in);
    return open();
}

auto animated_image_decoder::get_stream() -> istream&
{
    return *_stream;
}

}
