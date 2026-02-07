// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Image.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
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
    return get_pixel(static_cast<usize>((pos.X + (pos.Y * _info.Size.Width))));
}

auto image::get_pixel(usize idx) const -> color
{
    idx *= _info.bytes_per_pixel();
    u8 const r {_buffer[idx + 0]};
    u8 const g {_buffer[idx + 1]};
    u8 const b {_buffer[idx + 2]};
    u8 const a {image::information::HasAlpha(_info.Format) ? _buffer[idx + 3] : static_cast<u8>(255)};
    return {r, g, b, a};
}

void image::set_pixel(point_i pos, color c)
{
    assert(_info.Size.contains(pos));
    usize const idx {static_cast<usize>((pos.X + (pos.Y * _info.Size.Width)))};
    set_pixel(idx, c);
}

void image::set_pixel(usize idx, color c)
{
    idx *= _info.bytes_per_pixel();
    _buffer[idx + 0] = c.R;
    _buffer[idx + 1] = c.G;
    _buffer[idx + 2] = c.B;
    if (image::information::HasAlpha(_info.Format)) { _buffer[idx + 3] = c.A; }
}

void image::clear()
{
    std::ranges::fill(_buffer, 0);
}

void image::fill(rect_i const& rect, color c)
{
    i32 const  bpp {_info.bytes_per_pixel()};
    bool const hasAlpha {image::information::HasAlpha(_info.Format)};

    rect_i const clipped {{std::max(0, rect.left()), std::max(0, rect.top())},
                          {std::min(rect.Size.Width, _info.Size.Width - rect.Position.X),
                           std::min(rect.Size.Height, _info.Size.Height - rect.Position.Y)}};

    if (clipped.Size.Width <= 0 || clipped.Size.Height <= 0) { return; }

    // Fill first row. Then copy to other rows.
    usize idx {static_cast<usize>((clipped.top() * _info.Size.Width) + clipped.left()) * bpp};
    for (i32 x {0}; x < clipped.Size.Width; ++x) {
        _buffer[idx + 0] = c.R;
        _buffer[idx + 1] = c.G;
        _buffer[idx + 2] = c.B;
        if (hasAlpha) { _buffer[idx + 3] = c.A; }
        idx += bpp;
    }

    u8*         firstRow {&_buffer[(clipped.top() * _info.Size.Width + clipped.left()) * bpp]};
    isize const rowBytes {static_cast<isize>(clipped.Size.Width) * bpp};

    for (i32 y {clipped.top() + 1}; y < clipped.bottom(); ++y) {
        u8* destRow {&_buffer[(y * _info.Size.Width + clipped.left()) * bpp]};
        std::memcpy(destRow, firstRow, rowBytes);
    }
}

void image::blit(point_i offset, image const& src)
{
    auto const [width, height] {src.info().Size};
    for (i32 y {0}; y < height; ++y) {
        for (i32 x {0}; x < width; ++x) {
            set_pixel({x + offset.X, y + offset.Y}, src.get_pixel({x, y}));
        }
    }
}

void image::blend(point_i offset, image const& src)
{
    auto const [width, height] {src.info().Size};
    for (i32 y {0}; y < height; ++y) {
        for (i32 x {0}; x < width; ++x) {
            color const dstCol {get_pixel({x + offset.X, y + offset.Y})};
            color const srcCol {src.get_pixel({x, y})};

            f32 const sa {srcCol.A / 255.0f};
            f32 const da {dstCol.A / 255.0f};
            f32 const outA {sa + (da * (1 - sa))};

            color const out {
                static_cast<u8>((srcCol.R * sa + dstCol.R * da * (1 - sa)) / outA),
                static_cast<u8>((srcCol.G * sa + dstCol.G * da * (1 - sa)) / outA),
                static_cast<u8>((srcCol.B * sa + dstCol.B * da * (1 - sa)) / outA),
                static_cast<u8>(outA * 255)};

            set_pixel({x + offset.X, y + offset.Y}, out);
        }
    }
}

auto image::crop(rect_i const& bounds) const -> image
{
    if (bounds == rect_i::Zero) { return image::CreateEmpty(size_i::Zero, _info.Format); }
    return image::Create(bounds.Size, _info.Format, data(bounds));
}

auto image::count_colors [[nodiscard]] () const -> isize
{
    std::unordered_set<u32> colors;
    bool const              hasAlpha {image::information::HasAlpha(_info.Format)};
    i32 const               bpp {_info.bytes_per_pixel()};

    for (usize i {0}; i < _buffer.size(); i += bpp) {
        colors.insert((static_cast<u32>(_buffer[i + 0]) << 24)
                      | (static_cast<u32>(_buffer[i + 1]) << 16)
                      | (static_cast<u32>(_buffer[i + 2]) << 8)
                      | static_cast<u32>(hasAlpha ? _buffer[i + 3] : static_cast<u8>(255)));
    }

    return std::ssize(colors);
}

auto image::Create(size_i size, format f, std::span<u8 const> data) -> image
{
    return {size, f, data};
}

auto image::Create(size_i size, format f, std::span<std::byte const> data) -> image
{
    return {size, f, {reinterpret_cast<u8 const*>(data.data()), data.size()}};
}

auto image::CreateEmpty(size_i size, format f) -> image
{
    image retValue {size, f};
    retValue._buffer.resize(static_cast<usize>(size.Width * size.Height * image::information::GetBPP(f)));
    return retValue;
}

auto image::Load(path const& file) noexcept -> std::optional<image>
{
    image      retValue;
    auto const err {retValue.load(file)};
    if (err) { return retValue; }
    return std::nullopt;
}

auto image::Load(io::istream& in, string const& ext) noexcept -> std::optional<image>
{
    image      retValue;
    auto const err {retValue.load(in, ext)};
    if (err) { return retValue; }
    return std::nullopt;
}

auto image::load(path const& file) noexcept -> bool
{
    io::ifstream fs {file};
    return load(fs, io::get_extension(file));
}

auto image::load(io::istream& in, string const& ext) noexcept -> bool
{
    if (!in) { return false; }

    if (auto decoder {locate_service<image_decoder::factory>().create_from_magic(in, ext)}) {
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
    return locate_service<task_manager>().run_async<bool>([this, file] { return load(file); });
}

auto image::LoadInfo(path const& file) noexcept -> std::optional<information>
{
    io::ifstream fs {file};
    if (auto decoder {locate_service<image_decoder::factory>().create_from_magic(fs, io::get_extension(file))}) {
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

////////////////////////////////////////////////////////////

auto save_animation [[nodiscard]] (path const& file, std::span<image_frame const> frames) noexcept -> bool
{
    return save_animation(std::make_shared<io::ofstream>(file), io::get_extension(file), frames);
}

auto save_animation [[nodiscard]] (std::shared_ptr<io::ostream> out, string const& ext, std::span<image_frame const> frames) noexcept -> bool
{
    auto enc {locate_service<gfx::animated_image_encoder::factory>().create(ext)};
    enc->start(std::move(out));
    enc->add_frames(frames);
    return enc->finish();
}

auto save_animation_async [[nodiscard]] (path const& file, std::span<image_frame const> frames) noexcept -> std::future<bool>
{
    return locate_service<task_manager>().run_async<bool>([file, frames] { return save_animation(file, frames); });
}

void animated_image_encoder::start(std::shared_ptr<io::ostream> out)
{
    _stream = std::move(out);
    start();
}

auto animated_image_encoder::add_frames(std::span<image_frame const> frames) -> bool
{
    return std::ranges::all_of(frames, [this](auto const& frame) {
        return add_frame(frame);
    });
}

auto animated_image_encoder::stream() -> io::ostream&
{
    return *_stream;
}

}
