// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_gif.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/ImageQuant.hpp"

namespace tcob::gfx::detail {

auto gif::read_color_table(i32 ncolors, io::istream& reader) -> std::vector<color>
{
    std::vector<u8> c(ncolors * 3);
    reader.read_to<u8>(c);

    std::vector<color> retValue;
    retValue.reserve(ncolors);
    for (i32 i {0}, j {0}; i < ncolors; i++) {
        u8 const r {static_cast<u8>(static_cast<u32>(c[j++]) & 0xff)};
        u8 const g {static_cast<u8>(static_cast<u32>(c[j++]) & 0xff)};
        u8 const b {static_cast<u8>(static_cast<u32>(c[j++]) & 0xff)};
        retValue.emplace_back(r, g, b, 255);
    }

    return retValue;
}

void gif::header::read(io::istream& reader)
{
    Id = "";
    for (i32 i {0}; i < 6; i++) {
        Id += reader.read<char>();
    }

    //.ReadLogicalScreenDescriptor(s);
    // logical screen size
    Width  = reader.read<u16, std::endian::little>();
    Height = reader.read<u16, std::endian::little>();

    // packed fields
    i32 packed {reader.read<u8>()};
    GlobalColorTableFlag = (packed & 0x80) != 0; // 1   : global color table flag
    // 2-4 : color resolution
    // 5   : gct sort flag
    GlobalColorTableSize = 2 << (packed & 7); // 6-8 : gct size

    BackgroundIndex = reader.read<u8>();      // background color index
    PixelAspect     = reader.read<u8>();      // pixel aspect ratio

    if (GlobalColorTableFlag) {
        GlobalColorTable = read_color_table(GlobalColorTableSize, reader);
    }
}

////////////////////////////////////////////////////////////

auto gif_decoder::decode(io::istream& in) -> std::optional<image>
{
    if (decode_info(in)) {
        read_contents(in);
        return _currentFrame;
    }

    return std::nullopt;
}

auto gif_decoder::decode_info(io::istream& in) -> std::optional<image::information>
{
    _header.read(in);
    _pixelCache.resize(_header.Height * _header.Width * gif::BPP);
    if (_header.Id.starts_with("GIF")) {
        return image::information {.Size = {_header.Width, _header.Height}, .Format = image::format::RGBA};
    }

    return std::nullopt;
}

////////////////////////////////////////////////////////////

auto gif_decoder::open() -> std::optional<image::information>
{
    auto& in {stream()};
    _header.read(in);
    _pixelCache.resize(_header.Height * _header.Width * gif::BPP);
    if (_header.Id.starts_with("GIF")) {
        read_contents(in);
        return image::information {.Size = {_header.Width, _header.Height}, .Format = image::format::RGBA};
    }

    return std::nullopt;
}

auto gif_decoder::current_frame() const -> std::span<u8 const>
{
    return _currentFrame.data();
}

auto gif_decoder::advance(milliseconds ts) -> animated_image_decoder::status
{
    if (!_header.Id.starts_with("GIF")) {
        return animated_image_decoder::status::DecodeFailure;
    }

    if (ts <= _currentTimeStamp) {
        return animated_image_decoder::status::OldFrame;
    }

    auto& in {stream()};
    while (read_contents(in) != animated_image_decoder::status::NoMoreFrames) {
        if (ts <= _currentTimeStamp) {
            return animated_image_decoder::status::NewFrame;
        }
    }

    return animated_image_decoder::status::NoMoreFrames;
}

void gif_decoder::reset()
{
    _firstFrame      = true;
    _hasTransparency = false;

    _currentTimeStamp = milliseconds::zero();
    stream().seek(_contentOffset, io::seek_dir::Begin);
}

////////////////////////////////////////////////////////////

auto gif_decoder::read_contents(io::istream& reader) -> animated_image_decoder::status
{
    auto retValue {animated_image_decoder::status::NoMoreFrames};

    if (_firstFrame) {
        _contentOffset = reader.tell() - 1;
    }

    // read GIF file content blocks
    bool done {false};

    while (!(done)) {
        switch (reader.read<u8>()) {

        case 0x21:     // extension
            switch (reader.read<u8>()) {
            case 0xf9: // graphics control extension
                read_graphic_control_ext(reader);
                break;

            default: // uninteresting extension
                do {
                    read_block(reader);
                } while (_blockSize > 0);
                break;
            }
            break;

        case 0x2C:           // image separator
            read_frame(reader);
            done     = true; // stop after one image
            retValue = animated_image_decoder::status::NewFrame;
            break;

        case 0x3b: // terminator
            done = true;
            break;

        case 0x00: // bad byte, but keep going and see what happens
            break;
        }

        if (reader.is_eof()) {
            done = true; // unexpected eof; missing terminator
        }
    }

    return retValue;
}

auto gif_decoder::decode_frame_data(io::istream& reader, u16 iw, u16 ih) -> std::vector<u8>
{
    static constexpr i32             MaxStackSize {4096};
    std::array<u8, MaxStackSize + 1> pixelStack {};
    std::array<i16, MaxStackSize>    prefix {};
    std::array<u8, MaxStackSize>     suffix {};

    //  Initialize GIF data stream decoder.
    i32 const dataSize {reader.read<u8>()};
    i32 const clear {1 << dataSize};
    i32 const endOfInformation {clear + 1};
    i32       available {clear + 2};
    i32       codeSize {dataSize + 1};
    i32       codeMask {(1 << codeSize) - 1};

    i32 nullCode {-1}, inCode {0}, code {0};
    i32 oldCode {nullCode};

    for (; code < clear; code++) {
        prefix[code] = 0;
        suffix[code] = static_cast<u8>(code);
    }

    //  Decode GIF pixel stream.
    i32 datum {0}, bits {0}, count {0}, first {0}, top {0}, index {0}, bi {0};

    std::vector<u8> retValue(iw * ih);

    i32 const npix {iw * ih};
    for (i32 i {0}; i < npix;) {
        if (top == 0) {
            if (bits < codeSize) {
                //  Load bytes until there are enough bits for a code.
                if (count == 0) {
                    // Read a new data block.
                    count = read_block(reader);
                    if (count <= 0) { break; }
                    bi = 0;
                }
                datum += (static_cast<i32>(_block[bi]) & 0xff) << bits;
                bits += 8;
                bi++;
                count--;
                continue;
            }

            //  Get the next code.
            code = datum & codeMask;
            datum >>= codeSize;
            bits -= codeSize;

            //  interpret the code
            if ((code > available) || (code == endOfInformation)) {
                break;
            }
            if (code == clear) {
                //  Reset decoder.
                codeSize  = dataSize + 1;
                codeMask  = (1 << codeSize) - 1;
                available = clear + 2;
                oldCode   = nullCode;
                continue;
            }
            if (oldCode == nullCode) {
                pixelStack[top++] = suffix[code];
                oldCode           = code;
                first             = code;
                continue;
            }
            inCode = code;
            if (code == available) {
                pixelStack[top++] = static_cast<u8>(first);
                code              = oldCode;
            }
            while (code > clear) {
                pixelStack[top++] = suffix[code];
                code              = prefix[code];
            }
            first = static_cast<i32>(suffix[code]) & 0xff;

            //  Add a new string to the string table,
            if (available >= MaxStackSize) { break; }
            pixelStack[top++] = static_cast<u8>(first);
            prefix[available] = static_cast<i16>(oldCode);
            suffix[available] = static_cast<u8>(first);
            available++;
            if (((available & codeMask) == 0) && (available < MaxStackSize)) {
                codeSize++;
                codeMask += available;
            }
            oldCode = inCode;
        }

        top--;
        retValue[index++] = pixelStack[top];

        i++;
    }

    for (i32 i {index}; i < npix; i++) {
        retValue[i] = 0; // clear missing pixels
    }

    return retValue;
}

auto gif_decoder::read_block(io::istream& reader) -> i32
{
    _blockSize = reader.read<u8>();
    usize n {0};
    if (_blockSize > 0) {
        usize count {0};
        while (n < _blockSize) {
            count = reader.read_to<u8>({_block.data(), (_blockSize - n)});
            if (reader.is_eof()) { break; }
            n += count;
        }
    }
    return static_cast<i32>(n);
}

void gif_decoder::read_graphic_control_ext(io::istream& reader)
{
    reader.read<u8>();                    // block size
    i32 const packed {reader.read<u8>()}; // packed fields
    _dispose = (packed & 0x1c) >> 2;      // disposal method
    if (_dispose == 0) {
        _dispose = 1;                     // elect to keep old image if discretionary
    }

    _hasTransparency = (packed & 1) != 0;

    milliseconds const delay {reader.read<u16, std::endian::little>() * 10};
    _currentTimeStamp += delay;

    _transIndex = reader.read<u8>(); // transparent color index
    reader.read<u8>();               // block terminator
}

void gif_decoder::read_frame(io::istream& reader)
{
    u16 const ix {reader.read<u16, std::endian::little>()}; // (sub)image position & size
    u16 const iy {reader.read<u16, std::endian::little>()};
    u16 const iw {reader.read<u16, std::endian::little>()};
    u16 const ih {reader.read<u16, std::endian::little>()};

    u8 const   packed {reader.read<u8>()};
    bool const lctFlag {(packed & 0x80) != 0}; // 1 - local color table flag

    // i32               interlace = (packed & 0x40) != 0; // 2 - interlace flag
    // 3 - sort flag
    // 4-5 - reserved
    i32 const lctSize {2 << (packed & 7)}; // 6-8 - local color table size

    std::vector<color> act {lctFlag
                                ? gif::read_color_table(lctSize, reader)
                                : _header.GlobalColorTable};

    if (_hasTransparency) {
        act[_transIndex] = colors::Transparent;          // set transparent color if specified
    }

    auto const data {decode_frame_data(reader, iw, ih)}; // decode pixel data

    u8* pixPtr {_pixelCache.data()};

    if (_dispose == 2 && !_firstFrame) { clear_pixel_cache(); }

    if (_firstFrame) {
        for (u8 palIdx : data) {
            assert(palIdx < act.size());
            auto [r, g, b, a] {act[palIdx]};
            *pixPtr++ = r;
            *pixPtr++ = g;
            *pixPtr++ = b;
            *pixPtr++ = a;
        }
        _firstFrame = false;
    } else {
        i32 index {0};
        for (usize y {0}; y < ih; y++) {
            for (usize x {0}; x < iw; x++) {
                u8 palIdx {data[index++]};
                if (palIdx != _transIndex || (palIdx == _header.BackgroundIndex && !_hasTransparency) || _dispose == 2) {
                    assert(palIdx < act.size());
                    auto [r, g, b, a] {act[palIdx]};
                    usize const pixInd {((x + ix) * gif::BPP) + ((y + iy) * (_header.Width * gif::BPP))};
                    pixPtr[pixInd + 0] = r;
                    pixPtr[pixInd + 1] = g;
                    pixPtr[pixInd + 2] = b;
                    pixPtr[pixInd + 3] = a;
                }
            }
        }
    }

    _currentFrame = image::Create({_header.Width, _header.Height}, image::format::RGBA, _pixelCache);
}

void gif_decoder::clear_pixel_cache()
{
    u8* pixPtr {_pixelCache.data()};
    for (usize i {0}; i < _pixelCache.capacity(); i += gif::BPP) {
        *pixPtr++ = 0;
        *pixPtr++ = 0;
        *pixPtr++ = 0;
        *pixPtr++ = 255;
    }
}

////////////////////////////////////////////////////////////

auto gif_encoder::encode(image const& image, io::ostream& out) -> bool
{
    start(out);
    image_frame const frame {.Image = image, .Duration = milliseconds::zero()};
    if (add_frame(frame, out)) { return finish(out); }
    return false;
}

void gif_encoder::start()
{
    start(stream());
}

auto gif_encoder::add_frame(image_frame const& frame) -> bool
{
    return add_frame(frame, stream());
}

auto gif_encoder::finish() -> bool
{
    return finish(stream());
}

void gif_encoder::start(io::ostream& out)
{
    _transIndex = 0;
    _firstFrame = true;
    out.write("GIF89a"); // header
}

auto gif_encoder::add_frame(image_frame const& frame, io::ostream& out) -> bool
{
    if (!_sizeSet) {
        _width  = static_cast<i16>(frame.Image.info().Size.Width);
        _height = static_cast<i16>(frame.Image.info().Size.Height);
    }

    auto const palette {octree_quantizer::GetPalette(frame.Image, 256)};
    auto const newFrame {floyd_steinberg_dither {palette}.to_indexed(frame.Image)};

    if (_firstFrame) {
        write_lsd(out);              // logical screen descriptior
        write_palette(out, palette); // global color table
        write_netscape_loop(out);
    }

    _delay = static_cast<i16>(frame.Duration.count() / 10);

    write_graphic_ctrl_ext(out);     // write graphic control extension
    write_image_desc(out);           // image descriptor
    if (!_firstFrame) {
        write_palette(out, palette); // local color table
    }
    write_pixels(out, newFrame);     // encode and write pixel data
    _firstFrame = false;
    return true;
}

auto gif_encoder::finish(io::ostream& out) -> bool
{
    out.write<u8>(0x3b); // gif trailer
    return true;
}

void gif_encoder::write_graphic_ctrl_ext(io::ostream& out) const
{
    out.write<u8>(0x21); // extension introducer
    out.write<u8>(0xf9); // GCE label
    out.write<u8>(4);    // data block size
    i32 transp {0}, disp {0};
    if (_transparent == colors::Transparent) {
        transp = 0;
        disp   = 0;          // dispose = no action
    } else {
        transp = 1;
        disp   = 2;          // force clear if using transparent color
    }
    if (_dispose >= 0) {
        disp = _dispose & 7; // user override
    }
    disp <<= 2;

    // packed fields
    out.write<u8>(static_cast<u8>(0 | disp | 0 | transp));

    out.write<i16>(_delay);     // delay x 1/100 sec
    out.write<u8>(_transIndex); // transparent color index
    out.write<u8>(0);           // block terminator
}

void gif_encoder::write_image_desc(io::ostream& out) const
{
    out.write<u8>(0x2c);    // image separator
    out.write<i16>(0);      // image position x,y = 0,0
    out.write<i16>(0);
    out.write<i16>(_width); // image size
    out.write<i16>(_height);
    // packed fields
    if (_firstFrame) {
        // no LCT  - GCT is used for first (or only) frame
        out.write<u8>(0);
    } else {
        // specify normal LCT
        out.write<u8>(static_cast<u8>(0x80 | // 1 local color table  1=yes
                                      0 |    // 2 interlace - 0=no
                                      0 |    // 3 sorted - 0=no
                                      0 |    // 4-5 reserved
                                      7));   // 6-8 size of color table
    }
}

void gif_encoder::write_lsd(io::ostream& out) const
{
    // logical screen size
    out.write<i16>(_width);
    out.write<i16>(_height);
    // packed fields
    out.write<u8>(static_cast<u8>(0x80 | // 1   : global color table flag = 1 (gct used)
                                  0x70 | // 2-4 : color resolution = 7
                                  0x00 | // 5   : gct sort flag = 0
                                  7));   // 6-8 : gct size

    out.write<u8>(0);                    // background color index
    out.write<u8>(0);                    // pixel aspect ratio - assume 1:1
}

void gif_encoder::write_palette(io::ostream& out, std::span<color const> pal) const
{
    for (auto c : pal) {
        out.write<u8>(c.R);
        out.write<u8>(c.G);
        out.write<u8>(c.B);
    }
    i32 const n {static_cast<i32>((3 * 256) - (3 * pal.size()))};
    for (i32 i = 0; i < n; i++) {
        out.write<u8>(0);
    }
}

void gif_encoder::write_netscape_loop(io::ostream& out)
{
    out.write<u8>(0x21);      // Extension Introducer
    out.write<u8>(0xFF);      // Application Label
    out.write<u8>(0x0B);      // Block Size
    out.write("NETSCAPE2.0"); // App Identifier
    out.write<u8>(0x03);      // Sub-block data size
    out.write<u8>(0x01);      // Sub-block ID
    out.write<u8>(0x00);      // Loop Count LSB (0 = infinite)
    out.write<u8>(0x00);      // Loop Count MSB
    out.write<u8>(0x00);      // Block Terminator
}

class lzw_encoder {
    class bit_encoder {
    public:
        std::vector<u8> OutList;
        i32             InBit;

        bit_encoder(i32 init)
            : InBit {init}
        {
            OutList.reserve(1024);
        }

        void add(i32 inCode)
        {
            _currentVal |= (static_cast<u32>(inCode) << _currentBit);
            _currentBit += InBit;

            while (_currentBit >= 8) {
                OutList.push_back(static_cast<u8>(_currentVal & 0xFF));
                _currentVal >>= 8;
                _currentBit -= 8;
            }
        }

        void end()
        {
            if (_currentBit > 0) {
                OutList.push_back(static_cast<u8>(_currentVal & 0xFF));
            }
            _currentVal = 0;
            _currentBit = 0;
        }

    private:
        i32 _currentBit {0};
        u32 _currentVal {0};
    };

public:
    lzw_encoder(std::span<u32 const> pixel, u8 cd)
        : _colorDepth {std::max<u8>(2, cd)}
        , _indexedPixel {pixel}
    {
    }

    void encode(io::ostream& out)
    {
        i32 const clearCode {(1 << _colorDepth)};
        i32 const endCode {clearCode + 1};

        i32 availableCode {endCode + 1};
        i32 currentCodeSize {_colorDepth + 1};

        std::unordered_map<u32, i32> codeTable;
        codeTable.reserve(MaxStackSize);

        bit_encoder bitEncoder {currentCodeSize};
        out.write<u8>(_colorDepth);
        bitEncoder.add(clearCode);

        i32 prefix {-1};

        for (u32 suffix : _indexedPixel) {
            if (prefix == -1) {
                prefix = suffix;
                continue;
            }

            u32 const key {(static_cast<u32>(prefix) << 8) | suffix};

            if (auto it {codeTable.find(key)}; it != codeTable.end()) {
                prefix = it->second;
            } else {
                bitEncoder.add(prefix);

                if (availableCode < MaxStackSize) {
                    codeTable[key] = availableCode++;
                }

                if (availableCode > (1 << currentCodeSize) && currentCodeSize < 12) {
                    currentCodeSize++;
                    bitEncoder.InBit = currentCodeSize;
                }

                if (availableCode >= MaxStackSize) {
                    bitEncoder.add(clearCode);
                    codeTable.clear();
                    currentCodeSize  = _colorDepth + 1;
                    bitEncoder.InBit = currentCodeSize;
                    availableCode    = endCode + 1;
                }

                prefix = suffix;
            }

            flush_blocks(out, bitEncoder.OutList, false);
        }

        if (prefix != -1) { bitEncoder.add(prefix); }
        bitEncoder.add(endCode);
        bitEncoder.end();
        flush_blocks(out, bitEncoder.OutList, true);

        out.write<u8>(0x00);
    }

private:
    void flush_blocks(io::ostream& stream, std::vector<u8>& list, bool finish)
    {
        while (list.size() >= 255 || (finish && !list.empty())) {
            usize const chunkSize {std::min<usize>(list.size(), 255)};
            if (chunkSize == 0) { break; }

            stream.write<u8>(static_cast<u8>(chunkSize));
            stream.write<u8>(std::span {list.data(), chunkSize});

            list.erase(list.begin(), list.begin() + chunkSize);
        }
    }

    static constexpr i32 MaxStackSize {4096};
    u8                   _colorDepth;
    std::span<u32 const> _indexedPixel;
};

void gif_encoder::write_pixels(io::ostream& out, std::vector<u32> const& buffer) const
{
    lzw_encoder enc {buffer, 8};
    enc.encode(out);
}

}
