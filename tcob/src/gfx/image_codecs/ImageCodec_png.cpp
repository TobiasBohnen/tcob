// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_png.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstring>
#include <iterator>
#include <optional>
#include <span>
#include <vector>

#include <miniz/miniz.h>

#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/io/Filter.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/ImageFilters.hpp"

namespace tcob::gfx::detail {

png::IHDR_chunk::IHDR_chunk(std::span<u8 const> data)
    : BitDepth {data[8]}
    , ColorType {static_cast<color_type>(data[9])}
    , CompressionMethod {data[10]}
    , FilterMethod {data[11]}
    , Height {static_cast<i32>(data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7])}
    , InterlaceMethod {data[12]}
    , NonInterlaced {InterlaceMethod == 0}
    , Width {static_cast<i32>(data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3])}
{
}

png::PLTE_chunk::PLTE_chunk(std::span<u8 const> data)
{
    usize const size {std::min<usize>(data.size() / 3, 256)};
    Entries.resize(size);

    for (u32 i {0}; i < size; ++i) {
        auto& color {Entries[i]};
        color.R = data[(i * 3) + 0];
        color.G = data[(i * 3) + 1];
        color.B = data[(i * 3) + 2];
        color.A = 255;
    }
}

png::tRNS_chunk::tRNS_chunk(std::span<u8 const> data, color_type colorType, std::optional<PLTE_chunk>& plte)
{
    switch (colorType) {
    case color_type::Grayscale:
        if (data.size() >= 2) {
            Indicies.resize(1);
            Indicies[0] = data[1];
        }
        break;

    case color_type::TrueColor:
        if (data.size() >= 6) {
            Indicies.resize(3);
            Indicies[0] = data[1];
            Indicies[1] = data[3];
            Indicies[2] = data[5];
        }
        break;

    case color_type::Indexed:
        if (plte) {
            for (usize i {0}; i < data.size(); ++i) {
                plte->Entries[i].A = data[i];
            }
        }
        break;
    default: break;
    }
}

auto png::tRNS_chunk::is_gray_transparent(u8 val) -> bool
{
    return !Indicies.empty()
        && val == Indicies[0];
}

auto png::tRNS_chunk::is_rgb_transparent(u8 r, u8 g, u8 b) -> bool
{
    return Indicies.size() >= 3
        && r == Indicies[0]
        && g == Indicies[1]
        && b == Indicies[2];
}

png::pHYs_chunk::pHYs_chunk(std::span<u8 const> data)
{
    if (data.size() != 9) { return; }

    i32 const ppuX {data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]};
    i32 const ppuY {data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7]};
    Value = static_cast<f32>(ppuX) / static_cast<f32>(ppuY);
}

constexpr std::array<byte, 8> SIGNATURE {0x89, 0x50, 0x4e, 0x47, 0x0d, 0xa, 0x1a, 0x0a};

////////////////////////////////////////////////////////////

auto png_decoder::decode(io::istream& in) -> std::optional<image>
{
    if (decode_info(in)) {
        if (_ihdr.Width > png::MAX_SIZE || _ihdr.Height > png::MAX_SIZE) { return std::nullopt; }

        std::vector<byte>              idat {};
        std::optional<png::pHYs_chunk> phys {};

        for (;;) {
            if (in.is_eof()) { return std::nullopt; }

            auto const chunk {read_chunk(in)};
            // IDAT
            if (chunk.Type == png::chunk_type::IDAT) {
                idat.insert(idat.end(), chunk.Data.begin(), chunk.Data.end());
            }
            // PLTE
            else if (chunk.Type == png::chunk_type::PLTE) {
                if (chunk.Length % 3 != 0) { return std::nullopt; }
                _plte = {chunk.Data};
            }
            // TRNS
            else if (chunk.Type == png::chunk_type::tRNS) {
                _trns = {chunk.Data, _ihdr.ColorType, _plte};
            }
            // PHYS
            else if (chunk.Type == png::chunk_type::pHYs) {
                phys = {chunk.Data};
            }
            // IEND
            else if (chunk.Type == png::chunk_type::IEND) {
                break;
            }
        }

        auto const idatInflated {io::zlib_filter {}.from(idat)};
        if (read_image(idatInflated)) {
            size_i const size {_ihdr.Width, _ihdr.Height};
            auto         retValue {image::Create(size, image::format::RGBA, _data)};

            if (phys && phys->Value != 1.0f) {
                resize_nearest_neighbor filter;
                filter.NewSize = phys->Value > 1.0f
                    ? size_i {size.Width, static_cast<i32>(static_cast<f32>(size.Height) * phys->Value)}
                    : size_i {static_cast<i32>(static_cast<f32>(size.Width) / phys->Value), size.Height};
                if (filter.NewSize != size) { return filter(retValue); }
            }

            return retValue;
        }
    }

    return std::nullopt;
}

auto png_decoder::decode_info(io::istream& in) -> std::optional<image::information>
{
    if (check_sig(in) && read_header(in)) {
        return image::information {{_ihdr.Width, _ihdr.Height}, image::format::RGBA};
    }

    return std::nullopt;
}

auto png_decoder::read_header(io::istream& in) -> bool
{
    auto const chunk {read_chunk(in)}; // The IHDR chunk must appear FIRST.
    if (chunk.Type == png::chunk_type::IHDR && chunk.Data.size() == 13) {
        _ihdr = {chunk.Data};
        return true;
    }
    return false;
}

auto png_decoder::read_chunk(io::istream& in) const -> png::chunk
{
    png::chunk retValue;
    retValue.Length = in.read<u32, std::endian::big>();
    retValue.Type   = static_cast<png::chunk_type>(in.read<u32, std::endian::big>());

    if (retValue.Length > 0) {
        retValue.Data.resize(retValue.Length);
        in.read_to<u8>(retValue.Data);
    }

    retValue.Crc = in.read<u32, std::endian::big>();

    return retValue;
}

auto png_decoder::check_sig(io::istream& in) -> bool
{
    std::array<byte, 8> buf {};
    in.read_to<byte>(buf);
    return buf == SIGNATURE;
}

auto static paeth(u8 a, u8 b, u8 c) -> u8
{ // https://github.com/nothings/stb/blob/f4a71b13373436a2866c5d68f8f80ac6f0bc1ffe/stb_image.h#L4656C1-L4667C1
    i32 const thresh {(c * 3) - (a + b)};
    u8 const  lo {a < b ? a : b};
    u8 const  hi {a < b ? b : a};
    u8 const  t0 {(hi <= thresh) ? lo : c};
    u8 const  t1 {(thresh <= lo) ? hi : t0};
    return t1;
}

void png_decoder::filter_pixel()
{
    if (_filter == 0) { return; }

    i32 const x {_pixel.X / std::max(1, (8 / _ihdr.BitDepth))};
    i32 const xLength {x * _pixelSize};
    assert(xLength + _pixelSize <= std::ssize(_curLine) && xLength - _pixelSize <= std::ssize(_curLine));

    switch (_filter) {
    case 1: {
        if (x <= 0) { return; }
        for (u8 i {0}; i < _pixelSize; ++i) {
            *(_curLineIt + i) += _curLine[xLength + i - _pixelSize];
        }
    } break;
    case 2: {
        if (_pixel.Y <= 0) { return; }
        for (u8 i {0}; i < _pixelSize; ++i) {
            *(_curLineIt + i) += _prvLine[xLength + i];
        }
    } break;
    case 3: {
        for (u8 i {0}; i < _pixelSize; ++i) {
            i32 const a {
                (x > 0 ? _curLine[xLength + i - _pixelSize] : 0)
                + (_pixel.Y > 0 ? _prvLine[xLength + i] : 0)};
            *(_curLineIt + i) += static_cast<u8>(a / 2);
        }
    } break;
    case 4: {
        for (u8 i {0}; i < _pixelSize; ++i) {
            u8 const a {(x > 0 ? _curLine[xLength + i - _pixelSize] : u8 {0})};
            u8 const b {(_pixel.Y > 0 ? _prvLine[xLength + i] : u8 {0})};
            u8 const c {((x > 0 && _pixel.Y > 0) ? _prvLine[xLength + i - _pixelSize] : u8 {0})};
            *(_curLineIt + i) += paeth(a, b, c);
        }
    } break;
    }
}

void png_decoder::filter_line()
{
    if (_filter == 0) { return; }

    switch (_filter) {
    case 1: {
        for (usize i {_pixelSize}; i < _curLine.size(); ++i) {
            _curLine[i] += _curLine[i - _pixelSize];
        }
    } break;
    case 2: {
        if (_pixel.Y <= 0) { return; }
        for (usize i {0}; i < _curLine.size(); ++i) {
            _curLine[i] += _prvLine[i];
        }
    } break;
    case 3: {
        for (usize i {0}; i < _curLine.size(); ++i) {
            i32 const a {(i >= _pixelSize ? _curLine[i - _pixelSize] : 0) + (_pixel.Y > 0 ? _prvLine[i] : 0)};
            *(_curLineIt + i) += static_cast<u8>(a / 2);
        }
    } break;
    case 4: {
        for (usize i {0}; i < _curLine.size(); ++i) {
            u8 const a {(i >= _pixelSize ? _curLine[i - _pixelSize] : u8 {0})};
            u8 const b {(_pixel.Y > 0 ? _prvLine[i] : u8 {0})};
            u8 const c {((i >= _pixelSize && _pixel.Y > 0) ? _prvLine[i - _pixelSize] : u8 {0})};
            *(_curLineIt + i) += paeth(a, b, c);
        }
    } break;
    }
}

void png_decoder::next_line_interlaced(i32 hei)
{
    next_line_non_interlaced();
    if (_pixel.Y >= hei) {
        _pixel.Y = 0;
        ++_interlacePass;
    }
}

void png_decoder::next_line_non_interlaced()
{
    ++_pixel.Y;
    _pixel.X = -1;
    _curLine.swap(_prvLine);
}

auto png_decoder::get_interlace_dimensions() const -> rect_i
{
    switch (_interlacePass) {
    case 1: return {(0 + (_pixel.X * 8)), (0 + (_pixel.Y * 8)), (_ihdr.Width + 7) / 8, (_ihdr.Height + 7) / 8};
    case 2: return {(4 + (_pixel.X * 8)), (0 + (_pixel.Y * 8)), (_ihdr.Width + 3) / 8, (_ihdr.Height + 7) / 8};
    case 3: return {(0 + (_pixel.X * 4)), (4 + (_pixel.Y * 8)), (_ihdr.Width + 3) / 4, (_ihdr.Height + 3) / 8};
    case 4: return {(2 + (_pixel.X * 4)), (0 + (_pixel.Y * 4)), (_ihdr.Width + 1) / 4, (_ihdr.Height + 3) / 4};
    case 5: return {(0 + (_pixel.X * 2)), (2 + (_pixel.Y * 4)), (_ihdr.Width + 1) / 2, (_ihdr.Height + 1) / 4};
    case 6: return {(1 + (_pixel.X * 2)), (0 + (_pixel.Y * 2)), (_ihdr.Width + 0) / 2, (_ihdr.Height + 1) / 2};
    case 7: return {(0 + (_pixel.X * 1)), (1 + (_pixel.Y * 2)), (_ihdr.Width + 0) / 1, (_ihdr.Height + 0) / 2};
    }

    return {};
}

void png_decoder::prepare()
{
    auto const depth {_ihdr.BitDepth};

    i32 lineSize {0};
    switch (_ihdr.ColorType) {
    case png::color_type::Grayscale: // Grayscale
        switch (depth) {
        case 1:
            _pixelSize = 1;
            lineSize   = (_ihdr.Width + 7) / 8;
            break;
        case 2:
            _pixelSize = 1;
            lineSize   = (_ihdr.Width + 3) / 4;
            break;
        case 4:
            _pixelSize = 1;
            lineSize   = (_ihdr.Width + 1) / 2;
            break;
        case 8:
            _pixelSize = 1;
            lineSize   = _ihdr.Width;
            break;
        case 16:
            _pixelSize = 2;
            lineSize   = _ihdr.Width * 2;
            break;
        default: return;
        }

        break;
    case png::color_type::TrueColor: // Truecolor
        switch (depth) {
        case 8:  _pixelSize = 3; break;
        case 16: _pixelSize = 6; break;
        default: return;
        }

        lineSize = _ihdr.Width * _pixelSize;
        break;
    case png::color_type::Indexed: // Indexed-color
        switch (depth) {
        case 1:  lineSize = (_ihdr.Width + 7) / 8; break;
        case 2:  lineSize = (_ihdr.Width + 3) / 4; break;
        case 4:  lineSize = (_ihdr.Width + 1) / 2; break;
        case 8:  lineSize = _ihdr.Width; break;
        default: return;
        }

        _pixelSize = 1;
        break;
    case png::color_type::GrayscaleAlpha: // Grayscale with alpha
        switch (depth) {
        case 8:  _pixelSize = 2; break;
        case 16: _pixelSize = 4; break;
        default: return;
        }

        lineSize = _ihdr.Width * _pixelSize;
        break;
    case png::color_type::TrueColorAlpha: // Truecolor with alpha
        switch (depth) {
        case 8:  _pixelSize = 4; break;
        case 16: _pixelSize = 8; break;
        default: return;
        }

        lineSize = _ihdr.Width * _pixelSize;
        break;
    }

    _prvLine.resize(static_cast<usize>(lineSize));
    _curLine.resize(static_cast<usize>(lineSize));
    _data.resize(static_cast<usize>(_ihdr.Width * png::BPP * _ihdr.Height));
    _dataIt = _data.begin();
    prepare_delegate();
}

void png_decoder::prepare_delegate()
{
    switch (_ihdr.ColorType) {
    case png::color_type::Grayscale:
        switch (_ihdr.BitDepth) {
        case 1:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_G1
                : &png_decoder::interlaced_G1;
            break;
        case 2:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_G2
                : &png_decoder::interlaced_G2;
            break;
        case 4:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_G4
                : &png_decoder::interlaced_G4;
            break;
        case 8:
        case 16:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_G8_16
                : &png_decoder::interlaced_G8_16;
            break;
        }
        break;
    case png::color_type::TrueColor:
        switch (_ihdr.BitDepth) {
        case 8:
        case 16:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_TC8_16
                : &png_decoder::interlaced_TC8_16;
            break;
        }
        break;
    case png::color_type::Indexed:
        switch (_ihdr.BitDepth) {
        case 1:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_I1
                : &png_decoder::interlaced_I1;
            break;
        case 2:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_I2
                : &png_decoder::interlaced_I2;
            break;
        case 4:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_I4
                : &png_decoder::interlaced_I4;
            break;
        case 8:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_I8
                : &png_decoder::interlaced_I8;
            break;
        }
        break;
    case png::color_type::GrayscaleAlpha:
        switch (_ihdr.BitDepth) {
        case 8:
        case 16:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_GA8_16
                : &png_decoder::interlaced_GA8_16;
            break;
        }
        break;
    case png::color_type::TrueColorAlpha:
        switch (_ihdr.BitDepth) {
        case 8:
        case 16:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::non_interlaced_TCA8_16
                : &png_decoder::interlaced_TCA8_16;
            break;
        }
        break;
    }
}

auto png_decoder::read_image(std::span<byte const> idat) -> bool
{
    prepare();
    if (_pixelSize == 0) { return false; }

    auto const idatSize {std::ssize(idat)};
    if (_ihdr.NonInterlaced) { // size check for non-interlaced
        if (_ihdr.Height * (1 + std::ssize(_curLine)) != idatSize) { return false; }
    }

    for (i32 bufferIndex {0}; bufferIndex < idatSize; bufferIndex += _pixelSize) {
        if (_pixel.Y >= _ihdr.Height) { return false; }

        if (!_ihdr.NonInterlaced) {
            if (_ihdr.Width < 5 || _ihdr.Height < 5) {
                rect_i rect {get_interlace_dimensions()};
                while (rect.width() <= 0 || rect.height() <= 0) {
                    ++_interlacePass;
                    rect = get_interlace_dimensions();
                }
            }
        }

        auto const idatIt {idat.begin() + bufferIndex};
        if (_pixel.X == -1) { // First byte is filter type for the line.
            _filter    = *idatIt;
            _curLineIt = _curLine.begin();
            _pixel.X   = 0;

            if (_ihdr.NonInterlaced) { // copy and filter whole line if not interlaced
                std::copy(idatIt + 1, idatIt + 1 + std::ssize(_curLine), _curLineIt);
                filter_line();
            }

            bufferIndex = bufferIndex - _pixelSize + 1;
        } else {
            if (!_ihdr.NonInterlaced) { // copy and filter one by one if interlaced
                std::copy(idatIt, idatIt + _pixelSize, _curLineIt);
                filter_pixel();
            }

            (this->*_getImageData)();
            _curLineIt += _pixelSize;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////

auto png_encoder::encode(image const& image, io::ostream& out) const -> bool
{
    out.write(SIGNATURE);
    write_header(image, out);
    write_image(image, out);
    write_end(out);
    return true;
}

void png_encoder::write_header(image const& image, io::ostream& out) const
{
    auto const& info {image.info()};

    std::array<u8, 17> header {};

    u32 const type {std::byteswap(static_cast<u32>(png::chunk_type::IHDR))}; // TODO: endianess
    memcpy(header.data(), &type, 4);
    u32 const width {std::byteswap(static_cast<u32>(info.Size.Width))};      // TODO: endianess
    memcpy(header.data() + 4, &width, 4);
    u32 const height {std::byteswap(static_cast<u32>(info.Size.Height))};    // TODO: endianess
    memcpy(header.data() + 8, &height, 4);

    u8 bitDepth {0};
    u8 colorType {0};

    switch (info.Format) {
    case image::format::RGB:
        colorType = static_cast<u8>(png::color_type::TrueColor);
        bitDepth  = 8;
        break;
    case image::format::RGBA:
        colorType = static_cast<u8>(png::color_type::TrueColorAlpha);
        bitDepth  = 8;
        break;
    }

    header[12] = bitDepth;
    header[13] = colorType;
    header[14] = 0;
    header[15] = 0;
    header[16] = 0;

    write_chunk(out, header);
}

auto static data(image const& image) -> std::vector<u8>
{
    auto const  buffer {image.data()};
    auto const& info {image.info()};

    std::vector<u8> retValue(static_cast<usize>(info.size_in_bytes() + info.Size.Height));

    auto const format {info.Format};

    u32 index {0};
    u32 srcIndex {0};
    for (i32 y {0}; y < info.Size.Height; y++) {
        retValue[index++] = 0;

        for (i32 x {0}; x < info.Size.Width; x++) {
            switch (format) {
            case image::format::RGB: {
                retValue[index++] = buffer[srcIndex];
                retValue[index++] = buffer[srcIndex + 1];
                retValue[index++] = buffer[srcIndex + 2];
                srcIndex += 3;
            } break;

            case image::format::RGBA: {
                retValue[index++] = buffer[srcIndex];
                retValue[index++] = buffer[srcIndex + 1];
                retValue[index++] = buffer[srcIndex + 2];
                retValue[index++] = buffer[srcIndex + 3];
                srcIndex += 4;
            } break;
            }
        }
    }

    return retValue;
}

void png_encoder::write_image(image const& image, io::ostream& out) const
{
    // compress
    auto buf {io::zlib_filter {}.to(data(image))};
    if (buf.empty()) { return; }

    // write in 8192 byte chunks
    isize offset {0};
    usize total {buf.size()};

    constexpr usize                idatLength {8192};
    std::array<u8, idatLength + 4> idat {};

    u32 const type {std::byteswap(static_cast<u32>(png::chunk_type::IDAT))}; // TODO: endianess
    while (total > 0) {
        memcpy(idat.data(), &type, 4);
        usize const length {std::min(idatLength, total)};
        memcpy(idat.data() + 4, buf.data() + offset, length);
        write_chunk(out, idat, static_cast<u32>(length + 4));
        offset += length;
        total -= length;
    }
}

void png_encoder::write_end(io::ostream& out) const
{
    std::array<u8, 4> iend {};
    u32 const         type {std::byteswap(static_cast<u32>(png::chunk_type::IEND))}; // TODO: endianess
    memcpy(iend.data(), &type, 4);
    write_chunk(out, iend);
}

void png_encoder::write_chunk(io::ostream& out, std::span<u8 const> buf) const
{
    write_chunk(out, buf, static_cast<u32>(buf.size()));
}

void png_encoder::write_chunk(io::ostream& out, std::span<u8 const> buf, u32 length) const
{
    out.write<u32, std::endian::big>(length - 4);
    out.write<u8 const>({buf.data(), length});
    u32 const crc {static_cast<u32>(mz_crc32(MZ_CRC32_INIT, buf.data(), length))};
    out.write<u32, std::endian::big>(crc);
}

}
