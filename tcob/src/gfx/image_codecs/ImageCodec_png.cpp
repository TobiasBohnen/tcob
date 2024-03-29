// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_png.hpp"

#include <algorithm>

#include <miniz/miniz.h>

#include "tcob/core/io/Filter.hpp"
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
        color.R = data[i * 3 + 0];
        color.G = data[i * 3 + 1];
        color.B = data[i * 3 + 2];
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
            for (i32 i {0}; i < std::ssize(data); i++) {
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
    if (data.size() != 9) {
        Value = 1;
        return;
    }

    i32 const ppuX {data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]};
    i32 const ppuY {data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7]};
    Value = static_cast<f32>(ppuX) / ppuY;
}

constexpr std::array<ubyte, 8> SIGNATURE {0x89, 0x50, 0x4e, 0x47, 0x0d, 0xa, 0x1a, 0x0a};

////////////////////////////////////////////////////////////

auto png_decoder::decode(istream& in) -> std::optional<image>
{
    if (decode_header(in)) {
        if (_ihdr.Width > png::MAX_SIZE || _ihdr.Height > png::MAX_SIZE) {
            return std::nullopt;
        }

        std::vector<ubyte>             idat {};
        std::optional<png::pHYs_chunk> phys {};

        while (!in.is_eof()) {
            auto const chunk {read_chunk(in)};
            // IDAT
            if (chunk.Type == png::chunk_type::IDAT) {
                idat.insert(idat.end(), chunk.Data.begin(), chunk.Data.end());
            }
            // PLTE
            else if (chunk.Type == png::chunk_type::PLTE) {
                if (chunk.Length % 3 != 0) { return std::nullopt; }
                _plte = std::make_optional<png::PLTE_chunk>(chunk.Data);
            }
            // TRNS
            else if (chunk.Type == png::chunk_type::tRNS) {
                _trns = {chunk.Data, _ihdr.ColorType, _plte};
            }
            // PHYS
            else if (chunk.Type == png::chunk_type::pHYs) {
                phys = std::make_optional<png::pHYs_chunk>(chunk.Data);
            }
            // IEND
            else if (chunk.Type == png::chunk_type::IEND) {
                break;
            }
        }

        auto const idatInflated {io::zlib_filter {}.from(idat)};
        if (idatInflated && read_image(*idatInflated)) {
            size_i const size {_ihdr.Width, _ihdr.Height};
            auto         retValue {image::Create(size, image::format::RGBA, _data)};

            if (phys) {
                resize_nearest_neighbor filter;
                filter.NewSize = phys->Value > 1.0
                    ? size_i {size.Width, static_cast<i32>(size.Height * phys->Value)}
                    : size_i {static_cast<i32>(size.Width / phys->Value), size.Height};
                if (filter.NewSize != size) {
                    return filter(retValue);
                }
            }

            return retValue;
        }
    }

    return std::nullopt;
}

auto png_decoder::decode_header(istream& in) -> std::optional<image::info>
{
    if (check_sig(in) && read_header(in)) {
        return image::info {{_ihdr.Width, _ihdr.Height}, image::format::RGBA};
    }

    return std::nullopt;
}

auto png_decoder::read_header(istream& in) -> bool
{
    auto const chunk {read_chunk(in)}; // The IHDR chunk must appear FIRST.
    if (chunk.Type == png::chunk_type::IHDR && chunk.Data.size() == 13) {
        _ihdr = {chunk.Data};
        return true;
    }
    return false;
}

auto png_decoder::read_chunk(istream& in) const -> png::chunk
{
    png::chunk retValue;
    retValue.Length = in.read<u32>(std::endian::big);
    retValue.Type   = static_cast<png::chunk_type>(in.read<u32>(std::endian::big));

    if (retValue.Length > 0) {
        retValue.Data.resize(retValue.Length);
        in.read_to<u8>(retValue.Data);
    }

    retValue.Crc = in.read<u32>(std::endian::big);

    return retValue;
}

auto png_decoder::check_sig(istream& in) -> bool
{
    std::array<ubyte, 8> buf {};
    in.read_to<ubyte>(buf);
    return buf == SIGNATURE;
}

// https://github.com/nothings/stb/blob/f4a71b13373436a2866c5d68f8f80ac6f0bc1ffe/stb_image.h#L4656C1-L4667C1
auto static paeth(u8 a, u8 b, u8 c) -> u8
{
    i32 const thresh {c * 3 - (a + b)};
    u8 const  lo {a < b ? a : b};
    u8 const  hi {a < b ? b : a};
    u8 const  t0 {(hi <= thresh) ? lo : c};
    u8 const  t1 {(thresh <= lo) ? hi : t0};
    return t1;
}

void png_decoder::filter8(i32 x, i32 y, i32 unitLength)
{
    if (_filter == 0) { return; }

    i32 const xLength {x * unitLength};
    switch (_filter) {
    case 1:
        if (x > 0) {
            for (i32 i {0}; i < unitLength; i++) {
                _curLine[xLength + i] += _curLine[xLength + i - unitLength];
            }
        }
        break;
    case 2:
        if (y > 0) {
            for (i32 i {0}; i < unitLength; i++) {
                _curLine[xLength + i] += _prvLine[xLength + i];
            }
        }
        break;
    case 3:
        for (i32 i {0}; i < unitLength; i++) {
            i32 const a {
                (x > 0 ? _curLine[xLength + i - unitLength] : 0)
                + (y > 0 ? _prvLine[xLength + i] : 0)};
            _curLine[xLength + i] += static_cast<u8>(a / 2);
        }
        break;
    case 4:
        for (i32 i {0}; i < unitLength; i++) {
            u8 const a {(x > 0 ? _curLine[xLength + i - unitLength] : u8 {0})};
            u8 const b {(y > 0 ? _prvLine[xLength + i] : u8 {0})};
            u8 const c {((x > 0 && y > 0) ? _prvLine[xLength - unitLength + i] : u8 {0})};
            _curLine[xLength + i] += paeth(a, b, c);
        }
        break;
    }
}

auto png_decoder::get_interlace_dimensions() const -> rect_i
{
    switch (_interlacePass) {
    case 1:
        return {(_pixel.X * 8),
                (_pixel.Y * 8),
                (_ihdr.Width + 7) / 8,
                (_ihdr.Height + 7) / 8};
    case 2:
        return {(4 + _pixel.X * 8),
                (_pixel.Y * 8),
                (_ihdr.Width + 3) / 8,
                (_ihdr.Height + 7) / 8};
    case 3:
        return {(_pixel.X * 4),
                (4 + _pixel.Y * 8),
                (_ihdr.Width + 3) / 4,
                (_ihdr.Height + 3) / 8};
    case 4:
        return {(2 + _pixel.X * 4),
                (_pixel.Y * 4),
                (_ihdr.Width + 1) / 4,
                (_ihdr.Height + 3) / 4};
    case 5:
        return {(_pixel.X * 2),
                (2 + _pixel.Y * 4),
                (_ihdr.Width + 1) / 2,
                (_ihdr.Height + 1) / 4};
    case 6:
        return {(1 + _pixel.X * 2),
                (_pixel.Y * 2),
                (_ihdr.Width) / 2,
                (_ihdr.Height + 1) / 2};
    case 7:
        return {_pixel.X,
                (1 + _pixel.Y * 2),
                _ihdr.Width,
                _ihdr.Height / 2};
    }

    return {};
}

void png_decoder::next_line_interlaced(i32 hei)
{
    ++_pixel.Y;
    _pixel.X = -1;
    _curLine.swap(_prvLine);
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

auto png_decoder::prepare() -> i32
{
    i32        retValue {0};
    auto const depth {_ihdr.BitDepth};

    // See PNG Specification 11.2 for allowed combinations of color type and bit depth
    switch (_ihdr.ColorType) {
    case png::color_type::Grayscale: // Grayscale
        if (depth != 1 && depth != 2
            && depth != 4 && depth != 8
            && depth != 16) {
            return false;
        }
        break;
    case png::color_type::TrueColor: // Truecolor
        if (depth != 8 && depth != 16) {
            return false;
        }
        break;
    case png::color_type::Indexed: // Indexed-color
        if (depth != 1 && depth != 2
            && depth != 4 && depth != 8) {
            return false;
        }
        break;
    case png::color_type::GrayscaleAlpha: // Grayscale with alpha
        if (depth != 8 && depth != 16) {
            return false;
        }
        break;
    case png::color_type::TrueColorAlpha: // Truecolor with alpha
        if (depth != 8 && depth != 16) {
            return false;
        }
        break;
    }

    _interlacePass = 1;

    i32 lineSize {0};
    switch (_ihdr.ColorType) {
    case png::color_type::Grayscale: // Grayscale
        switch (depth) {
        case 1:
            retValue = 1;
            lineSize = (_ihdr.Width + 7) / 8;
            break;
        case 2:
            retValue = 1;
            lineSize = (_ihdr.Width + 3) / 4;
            break;
        case 4:
            retValue = 1;
            lineSize = (_ihdr.Width + 1) / 2;
            break;
        case 8:
            retValue = 1;
            lineSize = _ihdr.Width;
            break;
        case 16:
            retValue = 2;
            lineSize = _ihdr.Width * 2;
            break;
        }

        break;
    case png::color_type::TrueColor: // Truecolor
        switch (depth) {
        case 8:
            retValue = 3;
            break;
        case 16:
            retValue = 6;
            break;
        }

        lineSize = _ihdr.Width * retValue;
        break;
    case png::color_type::Indexed: // Indexed-color
        switch (depth) {
        case 1:
            lineSize = (_ihdr.Width + 7) / 8;
            break;
        case 2:
            lineSize = (_ihdr.Width + 3) / 4;
            break;
        case 4:
            lineSize = (_ihdr.Width + 1) / 2;
            break;
        case 8:
            lineSize = _ihdr.Width;
            break;
        }

        retValue = 1;
        break;
    case png::color_type::GrayscaleAlpha: // Grayscale with alpha
        switch (depth) {
        case 8:
            retValue = 2;
            break;
        case 16:
            retValue = 4;
            break;
        }

        lineSize = _ihdr.Width * retValue;
        break;
    case png::color_type::TrueColorAlpha: // Truecolor with alpha
        switch (depth) {
        case 8:
            retValue = 4;
            break;
        case 16:
            retValue = 8;
            break;
        }

        lineSize = _ihdr.Width * retValue;
        break;
    }

    _prvLine.resize(lineSize * 2);
    _curLine.resize(lineSize * 2);
    _stride = _ihdr.Width * png::BPP;
    _data.resize(_stride * _ihdr.Height);
    get_image_data_delegate();
    return retValue;
}

void png_decoder::get_image_data_delegate()
{
    switch (_ihdr.ColorType) {
    case png::color_type::Grayscale:
        switch (_ihdr.BitDepth) {
        case 1:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_G1
                : &png_decoder::get_image_data_interlaced_G1;
            break;
        case 2:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_G2
                : &png_decoder::get_image_data_interlaced_G2;
            break;
        case 4:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_G4
                : &png_decoder::get_image_data_interlaced_G4;
            break;
        case 8:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_G8
                : &png_decoder::get_image_data_interlaced_G8;
            break;
        case 16:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_G16
                : &png_decoder::get_image_data_interlaced_G16;
            break;
        }
        break;
    case png::color_type::TrueColor:
        switch (_ihdr.BitDepth) {
        case 8:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_TC8
                : &png_decoder::get_image_data_interlaced_TC8;
            break;
        case 16:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_TC16
                : &png_decoder::get_image_data_interlaced_TC16;
            break;
        }
        break;
    case png::color_type::Indexed:
        switch (_ihdr.BitDepth) {
        case 1:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_I1
                : &png_decoder::get_image_data_interlaced_I1;
            break;
        case 2:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_I2
                : &png_decoder::get_image_data_interlaced_I2;
            break;
        case 4:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_I4
                : &png_decoder::get_image_data_interlaced_I4;
            break;
        case 8:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_I8
                : &png_decoder::get_image_data_interlaced_I8;
            break;
        }
        break;
    case png::color_type::GrayscaleAlpha:
        switch (_ihdr.BitDepth) {
        case 8:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_GA8
                : &png_decoder::get_image_data_interlaced_GA8;
            break;
        case 16:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_GA16
                : &png_decoder::get_image_data_interlaced_GA16;
            break;
        }
        break;
    case png::color_type::TrueColorAlpha:
        switch (_ihdr.BitDepth) {
        case 8:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_TCA8
                : &png_decoder::get_image_data_interlaced_TCA8;
            break;

        case 16:
            _getImageData = _ihdr.NonInterlaced
                ? &png_decoder::get_image_data_non_interlaced_TCA16
                : &png_decoder::get_image_data_interlaced_TCA16;
            break;
        }
        break;
    }
}

auto png_decoder::read_image(std::span<ubyte const> idat) -> bool
{
    i32 const pixelSize {prepare()};
    if (pixelSize == 0) { return false; }

    for (i32 bufferIndex {0}; bufferIndex < std::ssize(idat); bufferIndex += pixelSize) {
        std::span<u8 const> const dat {idat.begin() + bufferIndex, idat.begin() + bufferIndex + pixelSize};

        if (_pixel.Y >= _ihdr.Height) { return false; }

        if (_ihdr.Width < 5 || _ihdr.Height < 5) {
            rect_i rect {get_interlace_dimensions()};
            while (rect.Width <= 0 || rect.Height <= 0) {
                ++_interlacePass;
                rect = get_interlace_dimensions();
            }
        }

        if (_pixel.X == -1) {      // First u8 is filter type for the line.
            _filter      = dat[0]; // See PNG Specification 4.5.4 Filtering, 9 Filtering
            _inLineCount = 0;
            ++_pixel.X;
            bufferIndex = bufferIndex - pixelSize + 1;
        } else {
            (this->*_getImageData)(dat);
        }
    }

    return true;
}

////////////////////////////////////////////////////////////

auto png_encoder::encode(image const& image, ostream& out) const -> bool
{
    out.write(SIGNATURE);
    write_header(image, out);
    write_image(image, out);
    write_end(out);
    return true;
}

void png_encoder::write_header(image const& image, ostream& out) const
{
    auto const& info {image.get_info()};

    std::array<u8, 17> header {};

    u32 const type {helper::byteswap(static_cast<u32>(png::chunk_type::IHDR))};
    memcpy(header.data(), &type, 4);
    u32 const width {helper::byteswap(static_cast<u32>(info.Size.Width))};
    memcpy(header.data() + 4, &width, 4);
    u32 const height {helper::byteswap(static_cast<u32>(info.Size.Height))};
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

auto static get_data(image const& image) -> std::vector<u8>
{
    auto const  buffer {image.get_data()};
    auto const& info {image.get_info()};

    std::vector<u8> retValue;
    retValue.resize(info.size_in_bytes() + info.Size.Height);

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

void png_encoder::write_image(image const& image, ostream& out) const
{
    // compress
    auto buf {io::zlib_filter {}.to(get_data(image))};
    if (!buf.has_value()) { return; }

    // write in 8192 byte chunks
    isize offset {0};
    isize total {std::ssize(*buf)};

    constexpr isize                idatLength {8192};
    std::array<u8, idatLength + 4> idat {};

    u32 const type {helper::byteswap(static_cast<u32>(png::chunk_type::IDAT))};
    while (total > 0) {
        memcpy(idat.data(), &type, 4);
        isize const length {std::min(idatLength, total)};
        memcpy(idat.data() + 4, buf->data() + offset, length);
        write_chunk(out, idat, static_cast<u32>(length + 4));
        offset += length;
        total -= length;
    }
}

void png_encoder::write_end(ostream& out) const
{
    std::array<u8, 4> iend {};
    u32 const         type {helper::byteswap(static_cast<u32>(png::chunk_type::IEND))};
    memcpy(iend.data(), &type, 4);
    write_chunk(out, iend);
}

void png_encoder::write_chunk(ostream& out, std::span<u8 const> buf) const
{
    write_chunk(out, buf, static_cast<u32>(buf.size()));
}

void png_encoder::write_chunk(ostream& out, std::span<u8 const> buf, u32 length) const
{
    out.write(length - 4, std::endian::big);
    out.write<u8 const>({buf.data(), length});
    u32 const crc {static_cast<u32>(mz_crc32(MZ_CRC32_INIT, buf.data(), length))};
    out.write(crc, std::endian::big);
}

}
