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
#include <ios>
#include <iterator>
#include <optional>
#include <span>
#include <vector>

#include <miniz/miniz.h>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/io/Filter.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/ImageFilters.hpp"

namespace tcob::gfx::detail {

static auto to_i32(std::span<u8 const> data, usize start) -> i32
{
    return static_cast<i32>(data[start] << 24 | data[start + 1] << 16 | data[start + 2] << 8 | data[start + 3]);
}
static auto to_u32(std::span<u8 const> data, usize start) -> u32
{
    return static_cast<u32>(data[start] << 24 | data[start + 1] << 16 | data[start + 2] << 8 | data[start + 3]);
}
static auto to_u16(std::span<u8 const> data, usize start) -> u16
{
    return static_cast<u16>(data[start] << 8 | data[start + 1]);
}

png::IHDR_chunk::IHDR_chunk(std::span<u8 const> data)
    : Width {to_i32(data, 0)}
    , Height {to_i32(data, 4)}
    , BitDepth {data[8]}
    , ColorType {static_cast<color_type>(data[9])}
    , CompressionMethod {data[10]}
    , FilterMethod {data[11]}
    , InterlaceMethod {data[12]}
    , NonInterlaced {InterlaceMethod == 0}
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

    i32 const ppuX {to_i32(data, 0)};
    i32 const ppuY {to_i32(data, 4)};

    Value = static_cast<f32>(ppuX) / (ppuY != 0 ? static_cast<f32>(ppuY) : 1.0f);
}

png::acTL_chunk::acTL_chunk(std::span<u8 const> data)
    : NumFrames {to_u32(data, 0)}
    , NumPlays {to_u32(data, 4)}
{
}

png::fcTL_chunk::fcTL_chunk(std::span<u8 const> data)
    : SequenceNumber {to_u32(data, 0)}
    , Width {to_i32(data, 4)}
    , Height {to_i32(data, 8)}
    , XOffset {to_u32(data, 12)}
    , YOffset {to_u32(data, 16)}
    , DelayNum {to_u16(data, 20)}
    , DelayDen {to_u16(data, 22)}
    , DisposeOp {static_cast<dispose_op>(data[24])}
    , BlendOp {static_cast<blend_op>(data[25])}
{
    if (DelayNum == 0) {
        Duration = milliseconds {1};
        return;
    }

    Duration = milliseconds {(DelayNum * 1000) / (DelayDen != 0 ? static_cast<u32>(DelayDen) : 100)};
}

constexpr std::array<byte, 8> SIGNATURE {0x89, 0x50, 0x4e, 0x47, 0x0d, 0xa, 0x1a, 0x0a};

////////////////////////////////////////////////////////////

void png_decoder::handle_plte(png::chunk const& chunk) { _plte = {chunk.Data}; }

void png_decoder::handle_trns(png::chunk const& chunk) { _trns = {chunk.Data, _ihdr.ColorType, _plte}; }

auto png_decoder::decode(io::istream& in) -> std::optional<image>
{
    if (!decode_info(in)) { return std::nullopt; }
    if (_ihdr.Width > png::MAX_SIZE || _ihdr.Height > png::MAX_SIZE) { return std::nullopt; }

    std::vector<byte>              idat {};
    std::optional<png::pHYs_chunk> phys {};

    for (;;) {
        if (in.is_eof()) { return std::nullopt; }

        auto const chunk {read_chunk(in)};

        if (chunk.Type == png::chunk_type::IDAT) {        // IDAT
            idat.insert(idat.end(), chunk.Data.begin(), chunk.Data.end());
        } else if (chunk.Type == png::chunk_type::PLTE) { // PLTE
            if (chunk.Length % 3 != 0) { return std::nullopt; }
            handle_plte(chunk);
        } else if (chunk.Type == png::chunk_type::tRNS) { // TRNS
            handle_trns(chunk);
        } else if (chunk.Type == png::chunk_type::pHYs) { // PHYS
            phys = {chunk.Data};
        } else if (chunk.Type == png::chunk_type::IEND) { // IEND
            break;
        }
    }

    if (!read_image(idat, _ihdr.Width, _ihdr.Height)) { return std::nullopt; }
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

auto png_decoder::decode_info(io::istream& in) -> std::optional<image::information>
{
    if (check_sig(in) && read_header(in)) {
        return image::information {.Size = {_ihdr.Width, _ihdr.Height}, .Format = image::format::RGBA};
    }

    return std::nullopt;
}

////////////////////////////////////////////////////////////

auto png_anim_decoder::open() -> std::optional<image::information>
{
    auto const& hdr {ihdr()};

    auto& in {stream()};
    if (check_sig(in) && read_header(in)) {
        if (hdr.Width > png::MAX_SIZE || hdr.Height > png::MAX_SIZE) { return std::nullopt; }

        for (;;) {
            if (in.is_eof()) { return std::nullopt; }

            auto const chunk {read_chunk(in)};

            if (chunk.Type == png::chunk_type::acTL) {                                               // ACTL
                _contentOffset = in.tell();
                _currentFrame  = image::CreateEmpty({hdr.Width, hdr.Height}, image::format::RGBA);
            } else if (chunk.Type == png::chunk_type::IDAT || chunk.Type == png::chunk_type::fdAT) { // IDAT | fdAT
                return _currentFrame.info();
            } else if (chunk.Type == png::chunk_type::PLTE) {                                        // PLTE
                if (chunk.Length % 3 != 0) { return std::nullopt; }
                handle_plte(chunk);
            } else if (chunk.Type == png::chunk_type::tRNS) {                                        // TRNS
                handle_trns(chunk);
            } else if (chunk.Type == png::chunk_type::IEND) {                                        // IEND
                return std::nullopt;
            }
        }
    }

    return std::nullopt;
}

auto png_anim_decoder::current_frame() const -> std::span<u8 const>
{
    return _currentFrame.data();
}

auto png_anim_decoder::get_next_frame(io::istream& in) -> animated_image_decoder::status
{
    std::vector<byte>              idat {};
    std::optional<png::fcTL_chunk> fctl {};

    for (;;) {
        if (in.is_eof()) { return animated_image_decoder::status::NoMoreFrames; }

        std::streamsize const lastPos {in.tell()};
        auto const            chunk {read_chunk(in)};

        if (chunk.Type == png::chunk_type::fcTL) {
            if (fctl) { // next fctl found -> break
                in.seek(lastPos, io::seek_dir::Begin);
                break;
            }
            fctl = {chunk.Data};
            _currentTimeStamp += fctl->Duration;
        } else if (chunk.Type == png::chunk_type::IDAT) {
            idat.insert(idat.end(), chunk.Data.begin(), chunk.Data.end());
        } else if (chunk.Type == png::chunk_type::fdAT) {
            idat.insert(idat.end(), chunk.Data.begin() + 4, chunk.Data.end()); // skip sequence num
        } else if (chunk.Type == png::chunk_type::IEND) {
            break;
        }
    }

    if (idat.empty() || !fctl) {
        return animated_image_decoder::status::NoMoreFrames;
    }

    if (!read_image(idat, fctl->Width, fctl->Height)) {
        return animated_image_decoder::status::DecodeFailure;
    }

    if (_previousFctl) {
        switch (_previousFctl->DisposeOp) {
        case png::dispose_op::None:       break;
        case png::dispose_op::Background: {
            size_i const  size {_previousFctl->Width, _previousFctl->Height};
            point_i const offset {static_cast<i32>(_previousFctl->XOffset), static_cast<i32>(_previousFctl->YOffset)};
            _currentFrame.fill({offset, size}, colors::Transparent);
        } break;
        case png::dispose_op::Previous:
            if (_previousFrame) { _currentFrame.blit(point_i::Zero, *_previousFrame); }
            break;
        }
    }

    _previousFctl = fctl;
    if (fctl->DisposeOp == png::dispose_op::Previous) {
        _previousFrame = _currentFrame;
    } else {
        _previousFrame.reset();
    }

    size_i const  size {fctl->Width, fctl->Height};
    point_i const offset {static_cast<i32>(fctl->XOffset), static_cast<i32>(fctl->YOffset)};
    auto const    frame {image::Create(size, image::format::RGBA, data())};
    switch (fctl->BlendOp) {
    case png::blend_op::Source: _currentFrame.blit(offset, frame); break;
    case png::blend_op::Over:   _currentFrame.blend(offset, frame); break;
    }

    return animated_image_decoder::status::NewFrame;
}

auto png_anim_decoder::advance(milliseconds ts) -> animated_image_decoder::status
{
    if (ts <= _currentTimeStamp) {
        return animated_image_decoder::status::OldFrame;
    }

    auto& in {stream()};
    while (get_next_frame(in) != animated_image_decoder::status::NoMoreFrames) {
        if (ts <= _currentTimeStamp) {
            return animated_image_decoder::status::NewFrame;
        }
    }

    return animated_image_decoder::status::NoMoreFrames; // TODO: check against frame count
}

void png_anim_decoder::reset()
{
    auto const& hdr {ihdr()};
    _currentTimeStamp = milliseconds::zero();
    _currentFrame     = image::CreateEmpty({hdr.Width, hdr.Height}, image::format::RGBA);
    _previousFrame    = {};
    stream().seek(_contentOffset, io::seek_dir::Begin);
}

////////////////////////////////////////////////////////////

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

auto png_decoder::read_image(std::span<byte const> idat, i32 width, i32 height) -> bool
{
    auto const idatInflated {io::zlib_filter {}.from(idat)};

    prepare(width, height);
    if (_pixelSize == 0) { return false; }

    auto const idatSize {std::ssize(idatInflated)};
    if (_ihdr.NonInterlaced) { // size check for non-interlaced
        if (height * (1 + std::ssize(_curLine)) != idatSize) { return false; }
    }

    for (i32 bufferIndex {0}; bufferIndex < idatSize; bufferIndex += _pixelSize) {
        if (_pixel.Y >= height) { return false; }

        if (!_ihdr.NonInterlaced) {
            if (width < 5 || height < 5) {
                rect_i rect {get_interlace_dimensions(width, height)};
                while (rect.width() <= 0 || rect.height() <= 0) {
                    ++_interlacePass;
                    rect = get_interlace_dimensions(width, height);
                }
            }
        }

        auto const idatIt {idatInflated.begin() + bufferIndex};
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

            (this->*_getImageData)(width, height);
            _curLineIt += _pixelSize;
        }
    }

    _pixel = {-1, 0};
    return true;
}

auto png_decoder::ihdr() const -> png::IHDR_chunk const& { return _ihdr; }
auto png_decoder::data() const -> std::vector<u8> const& { return _data; }

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
            _curLine[i] += static_cast<u8>(a / 2);
        }
    } break;
    case 4: {
        for (usize i {0}; i < _curLine.size(); ++i) {
            u8 const a {(i >= _pixelSize ? _curLine[i - _pixelSize] : u8 {0})};
            u8 const b {(_pixel.Y > 0 ? _prvLine[i] : u8 {0})};
            u8 const c {((i >= _pixelSize && _pixel.Y > 0) ? _prvLine[i - _pixelSize] : u8 {0})};
            _curLine[i] += paeth(a, b, c);
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

auto png_decoder::get_interlace_dimensions(i32 width, i32 height) const -> rect_i
{
    switch (_interlacePass) {
    case 1: return {(0 + (_pixel.X * 8)), (0 + (_pixel.Y * 8)), (width + 7) / 8, (height + 7) / 8};
    case 2: return {(4 + (_pixel.X * 8)), (0 + (_pixel.Y * 8)), (width + 3) / 8, (height + 7) / 8};
    case 3: return {(0 + (_pixel.X * 4)), (4 + (_pixel.Y * 8)), (width + 3) / 4, (height + 3) / 8};
    case 4: return {(2 + (_pixel.X * 4)), (0 + (_pixel.Y * 4)), (width + 1) / 4, (height + 3) / 4};
    case 5: return {(0 + (_pixel.X * 2)), (2 + (_pixel.Y * 4)), (width + 1) / 2, (height + 1) / 4};
    case 6: return {(1 + (_pixel.X * 2)), (0 + (_pixel.Y * 2)), (width + 0) / 2, (height + 1) / 2};
    case 7: return {(0 + (_pixel.X * 1)), (1 + (_pixel.Y * 2)), (width + 0) / 1, (height + 0) / 2};
    }

    return {};
}

void png_decoder::prepare(i32 width, i32 height)
{
    auto const depth {_ihdr.BitDepth};

    i32 lineSize {0};
    switch (_ihdr.ColorType) {
    case png::color_type::Grayscale: // Grayscale
        switch (depth) {
        case 1:
            _pixelSize = 1;
            lineSize   = (width + 7) / 8;
            break;
        case 2:
            _pixelSize = 1;
            lineSize   = (width + 3) / 4;
            break;
        case 4:
            _pixelSize = 1;
            lineSize   = (width + 1) / 2;
            break;
        case 8:
            _pixelSize = 1;
            lineSize   = width;
            break;
        case 16:
            _pixelSize = 2;
            lineSize   = width * 2;
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

        lineSize = width * _pixelSize;
        break;
    case png::color_type::Indexed: // Indexed-color
        switch (depth) {
        case 1:  lineSize = (width + 7) / 8; break;
        case 2:  lineSize = (width + 3) / 4; break;
        case 4:  lineSize = (width + 1) / 2; break;
        case 8:  lineSize = width; break;
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

        lineSize = width * _pixelSize;
        break;
    case png::color_type::TrueColorAlpha: // Truecolor with alpha
        switch (depth) {
        case 8:  _pixelSize = 4; break;
        case 16: _pixelSize = 8; break;
        default: return;
        }

        lineSize = width * _pixelSize;
        break;
    }

    _prvLine.resize(static_cast<usize>(lineSize));
    _curLine.resize(static_cast<usize>(lineSize));
    _data.resize(static_cast<usize>(width * png::BPP * height));
    _dataIt = _data.begin();
    prepare_delegate();
}

void png_decoder::prepare_delegate()
{
    switch (_ihdr.ColorType) {
    case png::color_type::Grayscale:
        switch (_ihdr.BitDepth) {
        case 1: _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_G1 : &png_decoder::interlaced_G1; break;
        case 2: _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_G2 : &png_decoder::interlaced_G2; break;
        case 4: _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_G4 : &png_decoder::interlaced_G4; break;
        case 8:
        case 16:
            _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_G8_16 : &png_decoder::interlaced_G8_16;
            break;
        }
        break;
    case png::color_type::TrueColor:
        switch (_ihdr.BitDepth) {
        case 8:
        case 16:
            _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_TC8_16 : &png_decoder::interlaced_TC8_16;
            break;
        }
        break;
    case png::color_type::Indexed:
        switch (_ihdr.BitDepth) {
        case 1: _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_I1 : &png_decoder::interlaced_I1; break;
        case 2: _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_I2 : &png_decoder::interlaced_I2; break;
        case 4: _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_I4 : &png_decoder::interlaced_I4; break;
        case 8: _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_I8 : &png_decoder::interlaced_I8; break;
        }
        break;
    case png::color_type::GrayscaleAlpha:
        switch (_ihdr.BitDepth) {
        case 8:
        case 16:
            _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_GA8_16 : &png_decoder::interlaced_GA8_16;
            break;
        }
        break;
    case png::color_type::TrueColorAlpha:
        switch (_ihdr.BitDepth) {
        case 8:
        case 16:
            _getImageData = _ihdr.NonInterlaced ? &png_decoder::non_interlaced_TCA8_16 : &png_decoder::interlaced_TCA8_16;
            break;
        }
        break;
    }
}

////////////////////////////////////////////////////////////

auto png_encoder::encode(image const& image, io::ostream& out) const -> bool
{
    out.write(SIGNATURE);
    write_ihdr(image.info(), out);
    write_idat(image, out);
    write_iend(out);
    return true;
}

void png_encoder::write_ihdr(image::information const& info, io::ostream& out) const
{
    std::array<u8, 17> header {};

    // TODO: endianess
    u32 const type {std::byteswap(static_cast<u32>(png::chunk_type::IHDR))};
    memcpy(header.data(), &type, 4);
    u32 const width {std::byteswap(static_cast<u32>(info.Size.Width))};
    memcpy(header.data() + 4, &width, 4);
    u32 const height {std::byteswap(static_cast<u32>(info.Size.Height))};
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

void png_encoder::write_idat(image const& image, io::ostream& out) const
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

void png_encoder::write_iend(io::ostream& out) const
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

////////////////////////////////////////////////////////////

auto calculate_diff_rect(image const& a, image const& b) -> std::optional<rect_i>
{
    auto const size {a.info().Size};
    assert(size == b.info().Size);

    i32  left {size.Width};
    i32  top {size.Height};
    i32  right {0};
    i32  bottom {0};
    bool found {false};

    for (i32 y {0}; y < size.Height; ++y) {
        for (i32 x {0}; x < size.Width; ++x) {
            if (a.get_pixel({x, y}) != b.get_pixel({x, y})) {
                left   = std::min(left, x);
                top    = std::min(top, y);
                right  = std::max(right, x + 1);
                bottom = std::max(bottom, y + 1);
                found  = true;
            }
        }
    }

    if (!found) {
        return std::nullopt;
    }

    return rect_i::FromLTRB(left, top, right, bottom);
}

auto png_anim_encoder::encode(std::span<frame const> frames, io::ostream& out) -> bool
{
    if (frames.empty()) { return false; }

    auto const& info {frames[0].Image.info()};
    out.write(SIGNATURE);
    _enc.write_ihdr(info, out);

    std::vector<frame> newFrames;
    newFrames.reserve(frames.size());
    std::vector<rect_i> newFrameRects;
    newFrameRects.reserve(frames.size());

    newFrames.emplace_back(frames[0]);
    newFrameRects.emplace_back(point_i::Zero, info.Size);

    for (u32 i {1}; i < frames.size(); ++i) {
        auto const diff {calculate_diff_rect(frames[i - 1].Image, frames[i].Image)};
        if (!diff || diff->width() == 0 || diff->height() == 0) {
            newFrames.back().TimeStamp += frames[i].TimeStamp;
        } else {
            newFrames.push_back({.Image = frames[i].Image.crop(*diff), .TimeStamp = frames[i].TimeStamp});
            newFrameRects.push_back(*diff);
        }
    }

    write_actl(newFrames, out);

    write_fctl(0, {point_i::Zero, info.Size}, newFrames[0], out);
    _enc.write_idat(newFrames[0].Image, out);

    u32 seq {1};
    for (u32 i {1}; i < newFrames.size(); ++i) {
        write_fctl(seq++, newFrameRects[i], newFrames[i], out);
        write_fdat(seq++, newFrames[i].Image, out);
    }
    _enc.write_iend(out);
    return true;
}

void png_anim_encoder::write_actl(std::span<frame const> frames, io::ostream& out) const
{
    std::array<u8, 12> actl {};

    // TODO: endianess
    u32 const type {std::byteswap(static_cast<u32>(png::chunk_type::acTL))};
    memcpy(actl.data(), &type, 4);
    u32 const numFrames {std::byteswap(static_cast<u32>(frames.size()))};
    memcpy(actl.data() + 4, &numFrames, 4);
    u32 const numPlays {0};
    memcpy(actl.data() + 8, &numPlays, 4);

    _enc.write_chunk(out, actl);
}

void png_anim_encoder::write_fctl(u32 idx, rect_i const& rect, frame const& frame, io::ostream& out) const
{
    std::array<u8, 30> fctl {};

    // TODO: endianess
    u32 const type {std::byteswap(static_cast<u32>(png::chunk_type::fcTL))};
    memcpy(fctl.data(), &type, 4);
    u32 const seq {std::byteswap(idx)};
    memcpy(fctl.data() + 4, &seq, 4);
    u32 const width {static_cast<u32>(std::byteswap(rect.Size.Width))};
    memcpy(fctl.data() + 8, &width, 4);
    u32 const height {static_cast<u32>(std::byteswap(rect.Size.Height))};
    memcpy(fctl.data() + 12, &height, 4);
    u32 const xoff {static_cast<u32>(std::byteswap(rect.left()))};
    memcpy(fctl.data() + 16, &xoff, 4);
    u32 const yoff {static_cast<u32>(std::byteswap(rect.top()))};
    memcpy(fctl.data() + 20, &yoff, 4);
    u16 const delayNum {std::byteswap(static_cast<u16>(frame.TimeStamp.count()))};
    memcpy(fctl.data() + 24, &delayNum, 2);
    u16 const delayDen {std::byteswap(u16 {1000})};
    memcpy(fctl.data() + 26, &delayDen, 2);
    fctl[28] = static_cast<u8>(png::dispose_op::None);
    fctl[29] = static_cast<u8>(png::blend_op::Source);
    _enc.write_chunk(out, fctl);
}

void png_anim_encoder::write_fdat(u32 idx, image const& frame, io::ostream& out) const
{
    // compress
    auto buf {io::zlib_filter {}.to(data(frame))};
    if (buf.empty()) { return; }

    // write in 8192 byte chunks
    isize offset {0};
    usize total {buf.size()};

    constexpr usize                fdatLength {8192};
    std::array<u8, fdatLength + 8> fdat {};

    u32 const type {std::byteswap(static_cast<u32>(png::chunk_type::fdAT))}; // TODO: endianess
    u32 const seq {std::byteswap(idx)};

    while (total > 0) {
        memcpy(fdat.data(), &type, 4);
        memcpy(fdat.data() + 4, &seq, 4);
        usize const length {std::min(fdatLength, total)};
        memcpy(fdat.data() + 8, buf.data() + offset, length);
        _enc.write_chunk(out, fdat, static_cast<u32>(length + 8));
        offset += length;
        total -= length;
    }
}
}
