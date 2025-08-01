// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Image.hpp"

#include <algorithm>
#include <cassert>
#include <expected>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::gfx {

image::image() = default;

image::image(size_i size, format f)
    : _info {.Size = size, .Format = f}
{
}

image::image(size_i size, format f, std::span<u8 const> data)
    : image {size, f}
{
    assert(std::ssize(data) == size.Width * size.Height * image::information::GetBPP(f));
    _buffer = {data.begin(), data.end()};
}

auto image::data(rect_i const& bounds) const -> std::vector<u8>
{
    assert(bounds.width() > 0 && bounds.height() > 0);
    assert(bounds.left() >= 0 && bounds.right() <= _info.Size.Width);

    i32 const bpp {_info.bytes_per_pixel()};
    i32 const srcStride {_info.stride()};
    i32 const dstStride {bounds.width() * bpp};

    std::vector<u8> retValue(static_cast<usize>(bounds.height() * dstStride));

    for (i32 y {bounds.top()}; y < bounds.bottom(); ++y) {
        u8 const* src {&_buffer[(y * srcStride) + (bounds.left() * bpp)]};
        i32 const dst {(y - bounds.top()) * dstStride};
        std::copy(src, src + dstStride, retValue.begin() + dst);
    }

    return retValue;
}

auto image::info() const -> information const&
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
            if (start != targetstart) {
                std::swap_ranges(
                    begin + start, begin + end,
                    begin + targetstart);
            }
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

auto image::get_pixel(point_i pos) const -> color
{
    assert(_info.Size.contains(pos));
    usize const idx {static_cast<usize>((pos.X + (pos.Y * _info.Size.Width)) * _info.bytes_per_pixel())};
    u8 const    r {_buffer[idx + 0]};
    u8 const    g {_buffer[idx + 1]};
    u8 const    b {_buffer[idx + 2]};
    u8 const    a {image::information::HasAlpha(_info.Format) ? _buffer[idx + 3] : static_cast<u8>(255)};
    return {r, g, b, a};
}

void image::set_pixel(point_i pos, color c)
{
    assert(_info.Size.contains(pos));
    usize const idx {static_cast<usize>((pos.X + (pos.Y * _info.Size.Width)) * _info.bytes_per_pixel())};
    _buffer[idx + 0] = c.R;
    _buffer[idx + 1] = c.G;
    _buffer[idx + 2] = c.B;
    if (image::information::HasAlpha(_info.Format)) { _buffer[idx + 3] = c.A; }
}

auto image::count_colors [[nodiscard]] () const -> isize
{
    std::unordered_set<u32> colors;
    for (usize i {0}; i < _buffer.size(); i += _info.bytes_per_pixel()) {
        u8 const r {_buffer[i + 0]};
        u8 const g {_buffer[i + 1]};
        u8 const b {_buffer[i + 2]};
        u8 const a {image::information::HasAlpha(_info.Format) ? _buffer[i + 3] : static_cast<u8>(255)};
        colors.insert(static_cast<u32>(r << 24 | g << 16 | b << 8 | a));
    }

    return std::ssize(colors);
}

auto image::Create(size_i size, format f, std::span<u8 const> data) -> image
{
    return {size, f, data};
}

auto image::CreateEmpty(size_i size, format f) -> image
{
    image retValue {size, f};
    retValue._buffer.resize(static_cast<usize>(size.Width * size.Height * image::information::GetBPP(f)));
    return retValue;
}

auto image::Load(path const& file) noexcept -> std::expected<image, bool>
{
    image      retValue;
    auto const err {retValue.load(file)};
    if (err) { return retValue; }
    return std::unexpected {err};
}

auto image::Load(io::istream& in, string const& ext) noexcept -> std::expected<image, bool>
{
    image      retValue;
    auto const err {retValue.load(in, ext)};
    if (err) { return retValue; }
    return std::unexpected {err};
}

auto image::load(path const& file) noexcept -> bool
{
    io::ifstream fs {file};
    return load(fs, io::get_extension(file));
}

auto image::load(io::istream& in, string const& ext) noexcept -> bool
{
    if (!in) { return false; }

    if (auto decoder {locate_service<image_decoder::factory>().from_magic(in, ext)}) {
        if (auto img {decoder->decode(in)}) {
            std::swap(_buffer, img->_buffer);
            std::swap(_info, img->_info);
            return _info.Size != size_i::Zero;
        }
    }

    return false;
}

auto image::load_async(path const& file) noexcept -> std::future<bool>
{
    return locate_service<task_manager>().run_async<bool>([&, file]() { return load(file); });
}

auto image::LoadInfo(path const& file) noexcept -> std::optional<information>
{
    io::ifstream fs {file};
    if (auto decoder {locate_service<image_decoder::factory>().from_magic(fs, io::get_extension(file))}) {
        return decoder->decode_info(fs);
    }

    return std::nullopt;
}

auto image::save(path const& file) const noexcept -> bool
{
    io::ofstream of {file};
    return save(of, io::get_extension(file));
}

auto image::save(io::ostream& out, string const& ext) const noexcept -> bool
{
    if (auto enc {locate_service<image_encoder::factory>().create(ext)}) {
        return enc->encode(*this, out);
    }

    return false;
}

auto image::save_async(path const& file) const noexcept -> std::future<bool>
{
    std::promise<bool> pro;
    auto               retValue {pro.get_future()};
    std::thread([*this, file](std::promise<bool> p) { p.set_value(save(file)); }, std::move(pro)).detach();
    return retValue;
}

////////////////////////////////////////////////////////////

auto image::information::size_in_bytes() const -> isize
{
    return Size.Height * Size.Width * static_cast<isize>(bytes_per_pixel());
}

auto image::information::bytes_per_pixel() const -> i32
{
    return GetBPP(Format);
}

auto image::information::stride() const -> i32
{
    return Size.Width * bytes_per_pixel(); // no padding bytes
}

auto image::information::GetBPP(format f) -> i32
{
    switch (f) {
    case image::format::RGB:  return 3;
    case image::format::RGBA: return 4;
    }

    return 0;
}

auto image::information::HasAlpha(format f) -> bool
{
    switch (f) {
    case image::format::RGB:  return false;
    case image::format::RGBA: return true;
    }

    return 0;
}

////////////////////////////////////////////////////////////

auto animated_image_decoder::open(std::shared_ptr<io::istream> in) -> std::optional<image::information>
{
    _stream = std::move(in);
    return open();
}

auto animated_image_decoder::stream() -> io::istream&
{
    return *_stream;
}

}
