// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_gif.hpp"

namespace tcob::gfx::detail {

auto gif::read_color_table(i32 ncolors, istream& reader) -> std::vector<color>
{
    std::vector<color> retValue {};

    std::vector<u8> c {};
    c.resize(3 * ncolors);
    reader.read_to<u8>(c);

    for (i32 i {0}, j {0}; i < ncolors; i++) {
        u8 const r {static_cast<u8>(static_cast<u32>(c[j++]) & 0xff)};
        u8 const g {static_cast<u8>(static_cast<u32>(c[j++]) & 0xff)};
        u8 const b {static_cast<u8>(static_cast<u32>(c[j++]) & 0xff)};
        retValue.emplace_back(r, g, b, 255);
    }

    return retValue;
}

void gif::header::read(istream& reader)
{
    Id = "";
    for (i32 i {0}; i < 6; i++) {
        Id += reader.read<char>();
    }

    //.ReadLogicalScreenDescriptor(s);
    // logical screen size
    Width  = static_cast<u32>(reader.read<u16>(std::endian::little));
    Height = static_cast<u32>(reader.read<u16>(std::endian::little));

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

auto gif_decoder::decode(istream& in) -> std::optional<image>
{
    if (decode_header(in)) {
        read_contents(in, _header);
        return _currentFrame;
    }

    return std::nullopt;
}

auto gif_decoder::decode_header(istream& in) -> std::optional<image::info>
{
    _header.read(in);
    if (_header.Id.rfind("GIF", 0) == 0) {
        return image::info {{static_cast<i32>(_header.Width), static_cast<i32>(_header.Height)}, image::format::RGBA};
    }

    return std::nullopt;
}

////////////////////////////////////////////////////////////

auto gif_anim_decoder::open() -> std::optional<image::info>
{
    auto& stream {get_stream()};
    _header.read(stream);
    if (_header.Id.rfind("GIF", 0) == 0) {
        read_contents(stream, _header);
        return image::info {{static_cast<i32>(_header.Width), static_cast<i32>(_header.Height)}, image::format::RGBA};
    }

    return std::nullopt;
}

auto gif_anim_decoder::get_current_frame() const -> u8 const*
{
    return _currentFrame.get_data().data();
}

auto gif_anim_decoder::seek_from_current(milliseconds ts) -> animated_image_decoder::status
{
    if (_header.Id.rfind("GIF", 0) != 0) {
        return animated_image_decoder::status::DecodeFailure;
    }

    if (ts <= _currentTimeStamp) {
        return animated_image_decoder::status::OldFrame;
    }

    while (read_contents(get_stream(), _header) != animated_image_decoder::status::NoMoreFrames) {
        if (ts <= _currentTimeStamp) {
            return animated_image_decoder::status::NewFrame;
        }
    }

    return animated_image_decoder::status::NoMoreFrames;
}

void gif_anim_decoder::reset()
{
    _currentTimeStamp = milliseconds {0};
    seek_to_first_frame(get_stream());
    clear_pixel_cache();
}

////////////////////////////////////////////////////////////

auto gif_decoder_base::read_contents(istream& reader, gif::header const& header) -> animated_image_decoder::status
{
    auto retValue {animated_image_decoder::status::NoMoreFrames};

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
                skip(reader);
                break;
            }
            break;

        case 0x2C: // image separator
            if (_firstFrame) {
                _firstFrameOffset = reader.tell() - 1;
                _firstFrame       = false;
            }
            read_frame(reader, header);
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

auto gif_decoder_base::decode_frame_data(istream& reader, u16 iw, u16 ih) -> std::vector<u8>
{
    static constexpr i32             MaxStackSize {4096};
    std::array<u8, MaxStackSize + 1> pixelStack {};
    std::array<i16, MaxStackSize>    prefix {};
    std::array<u8, MaxStackSize>     suffix {};

    //  Initialize GIF data stream decoder.
    i32 dataSize {reader.read<u8>()};
    i32 clear {1 << dataSize};
    i32 endOfInformation {clear + 1};
    i32 available {clear + 2};
    i32 codeSize {dataSize + 1};
    i32 codeMask {(1 << codeSize) - 1};

    i32 nullCode {-1}, inCode {0}, code {0};
    i32 oldCode {nullCode};

    for (; code < clear; code++) {
        prefix[code] = 0;
        suffix[code] = static_cast<u8>(code);
    }

    //  Decode GIF pixel stream.
    i32 datum {0}, bits {0}, count {0}, first {0}, top {0}, index {0}, bi {0};

    std::vector<u8> retValue;
    retValue.resize(iw * ih);

    i32 const npix {iw * ih};
    for (i32 i {0}; i < npix;) {
        if (top == 0) {
            if (bits < codeSize) {
                //  Load bytes until there are enough bits for a code.
                if (count == 0) {
                    // Read a new data block.
                    count = read_block(reader);
                    if (count <= 0) {
                        break;
                    }
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
            if (available >= MaxStackSize) {
                break;
            }
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

auto gif_decoder_base::read_block(istream& reader) -> i32
{
    _blockSize = reader.read<u8>();
    i32 n {0};
    if (_blockSize > 0) {
        i32 count {0};
        while (n < _blockSize) {
            count = static_cast<i32>(reader.read_to<u8>({_block.data(), static_cast<usize>(_blockSize - n)}));
            if (reader.is_eof()) {
                break;
            }
            n += count;
        }
    }
    return n;
}

void gif_decoder_base::read_graphic_control_ext(istream& reader)
{
    reader.read<u8>();                    // block size
    i32 const packed {reader.read<u8>()}; // packed fields
    _dispose = (packed & 0x1c) >> 2;      // disposal method
    if (_dispose == 0) {
        _dispose = 1;                     // elect to keep old image if discretionary
    }

    _transparency = (packed & 1) != 0;

    milliseconds const delay {reader.read<u16>(std::endian::little) * 10};
    _currentTimeStamp += delay;

    _transIndex = reader.read<u8>(); // transparent color index
    reader.read<u8>();               // block terminator
}

void gif_decoder_base::read_frame(istream& reader, gif::header const& header)
{
    u16 const ix {reader.read<u16>(std::endian::little)}; // (sub)image position & size
    u16 const iy {reader.read<u16>(std::endian::little)};
    u16 const iw {reader.read<u16>(std::endian::little)};
    u16 const ih {reader.read<u16>(std::endian::little)};

    u8 const   packed {reader.read<u8>()};
    bool const lctFlag {(packed & 0x80) != 0}; // 1 - local color table flag

    // i32               interlace = (packed & 0x40) != 0; // 2 - interlace flag
    // 3 - sort flag
    // 4-5 - reserved
    i32 const          lctSize {2 << (packed & 7)}; // 6-8 - local color table size
    std::vector<color> act;

    if (lctFlag) {
        act = gif::read_color_table(lctSize, reader);
    } else {
        act = header.GlobalColorTable; // make global table active
    }

    if (_transparency) {
        act[_transIndex] = colors::Transparent;          // set transparent color if specified
    }

    auto const data {decode_frame_data(reader, iw, ih)}; // decode pixel data

    _pixelCache.resize(header.Height * header.Width * gif::BPP);
    u8* pixPtr {_pixelCache.data()};

    if (_dispose == 2 && !_firstFrame) {
        clear_pixel_cache();
    }

    if (_firstFrame) {
        for (u8 palIdx : data) {
            assert(palIdx < act.size());
            auto [r, g, b, a] {act[palIdx]};
            *pixPtr++ = r;
            *pixPtr++ = g;
            *pixPtr++ = b;
            *pixPtr++ = a;
        }
    } else {
        i32 index {0};
        for (i32 y {0}; y < ih; y++) {
            for (i32 x {0}; x < iw; x++) {
                u8 palIdx {data[index++]};
                if (palIdx != _transIndex || (palIdx == header.BackgroundIndex && !_transparency) || _dispose == 2) {
                    assert(palIdx < act.size());
                    auto [r, g, b, a] {act[palIdx]};
                    u32 const pixInd {((x + ix) * gif::BPP) + ((y + iy) * (header.Width * gif::BPP))};
                    pixPtr[pixInd + 0] = r;
                    pixPtr[pixInd + 1] = g;
                    pixPtr[pixInd + 2] = b;
                    pixPtr[pixInd + 3] = a;
                }
            }
        }
    }

    _currentFrame = image::Create({static_cast<i32>(header.Width), static_cast<i32>(header.Height)}, image::format::RGBA, _pixelCache);
}

void gif_decoder_base::skip(istream& reader)
{
    do {
        read_block(reader);
    } while (_blockSize > 0);
}

void gif_decoder_base::clear_pixel_cache()
{
    u8* pixPtr {_pixelCache.data()};
    for (usize i {0}; i < _pixelCache.capacity(); i += gif::BPP) {
        *pixPtr++ = 0;
        *pixPtr++ = 0;
        *pixPtr++ = 0;
        *pixPtr++ = 255;
    }
}

void gif_decoder_base::seek_to_first_frame(istream& reader) const
{
    reader.seek(_firstFrameOffset, io::seek_dir::Begin);
}

}
