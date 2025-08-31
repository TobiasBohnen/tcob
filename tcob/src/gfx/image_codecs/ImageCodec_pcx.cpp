// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_pcx.hpp"

#include <bit>
#include <cassert>
#include <iterator>
#include <optional>
#include <span>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"

/*
supported formats:
 Bit Depth 	Planes 	Number of Colors
 8 	        1 	    indexed 8-bit
 8 	        3 	    RGB
 1 	        1 	    monochrome
 1 	        4 	    indexed 4-bit
*/

namespace tcob::gfx::detail {

constexpr i32  HeaderPaletteLength {48};
constexpr i32  HeaderPaletteOffset {16};
constexpr char ManufacturerMagicNumber {0x0a};
constexpr char PaletteMagicNumber {12};
constexpr i32  PaletteOffset {769};
constexpr i32  HeaderLength {128};

auto pcx::read_image_data(io::istream& reader, header const& h) -> std::vector<u8>
{
    reader.seek(HeaderLength, io::seek_dir::Begin);

    std::vector<u8> retValue(h.BytesPerLine * h.ColorPlanesCount * h.Height());

    i32 total {0};

    for (i32 i {0}; i < h.Height(); i++) {
        i32 index {0};
        do {
            u8 const b {reader.read<u8>()};
            i32      runcount {0};
            u8       runvalue {0};

            if (h.Encoding == encoding::Rle && b > 0xc0) {
                runcount = b - 0xc0;
                runvalue = reader.read<u8>();
            } else {
                runcount = 1;
                runvalue = b;
            }

            for (; runcount != 0 && index < std::ssize(retValue); runcount--, index++, total++) {
                retValue[total] = runvalue;
            }
        } while (index < h.BytesPerLine * h.ColorPlanesCount);
    }

    return retValue;
}

auto pcx::read_color_palette(io::istream& reader, i32 size) -> std::vector<color>
{
    std::vector<color> retValue;
    for (i32 i {0}; i < size; ++i) {
        u8 const r {reader.read<u8>()};
        u8 const g {reader.read<u8>()};
        u8 const b {reader.read<u8>()};
        retValue.emplace_back(r, g, b, 255);
    }
    return retValue;
}

void pcx::header::read(io::istream& reader)
{
    Manufacturer = reader.read<u8>();

    Version      = static_cast<version>(reader.read<u8>());
    Encoding     = static_cast<encoding>(reader.read<u8>());
    BitsPerPixel = reader.read<u8>();
    XMin         = reader.read<u16, std::endian::little>();
    YMin         = reader.read<u16, std::endian::little>();
    XMax         = reader.read<u16, std::endian::little>();
    YMax         = reader.read<u16, std::endian::little>();
    HortDPI      = reader.read<u16, std::endian::little>();
    VertDPI      = reader.read<u16, std::endian::little>();

    // skip palette
    reader.seek(HeaderPaletteLength, io::seek_dir::Current);

    Reserved         = reader.read<u8>();
    ColorPlanesCount = reader.read<u8>();
    BytesPerLine     = reader.read<u16, std::endian::little>();
    PaletteType      = static_cast<palette_type>(reader.read<u16, std::endian::little>());
    HScrSize         = reader.read<u16, std::endian::little>();
    VScrSize         = reader.read<u16, std::endian::little>();
}

void pcx::header::Write(image::information const& info, io::ostream& writer)
{
    byte bpp {8}, cpc {3};

    writer.write(ManufacturerMagicNumber);
    writer.write<byte>(5);
    writer.write<byte>(1);
    writer.write(bpp);
    writer.write<i16, std::endian::little>(0);
    writer.write<i16, std::endian::little>(0);
    writer.write<i16, std::endian::little>(static_cast<i16>(info.Size.Width - 1));
    writer.write<i16, std::endian::little>(static_cast<i16>(info.Size.Height - 1));

    writer.write<i16, std::endian::little>(72);
    writer.write<i16, std::endian::little>(72);

    writer.seek(HeaderPaletteLength, io::seek_dir::Current);

    writer.write<byte>(0);
    writer.write(cpc);

    writer.write<i16, std::endian::little>(static_cast<i16>(info.Size.Width));
    writer.write<byte>(1);
    writer.write<i16, std::endian::little>(0);
    writer.write<i16, std::endian::little>(0);
}

////////////////////////////////////////////////////////////

auto pcx_decoder::decode(io::istream& in) -> std::optional<image>
{
    if (decode_info(in)) {
        auto const palette {read_palette(in)};
        auto const data {pcx::read_image_data(in, _header)};

        i32 const width {_header.Width()};
        i32 const height {_header.Height()};
        i16 const bpl {_header.BytesPerLine};

        image retValue {image::CreateEmpty({width, height}, image::format::RGB)};
        auto* imgData {retValue.ptr()};
        i32   index {0};

        if (!palette.empty()) {
            // indexed
            if (_header.BitsPerPixel == 8 && _header.ColorPlanesCount == 1) {
                i32 srcIndex {0};
                for (i32 y {0}; y < height; ++y) {
                    for (i32 x {0}; x < width; ++x) {
                        usize const idx {data[srcIndex++]};
                        if (idx >= palette.size()) { return std::nullopt; }
                        color const c {palette[idx]};
                        imgData[index++] = c.R;
                        imgData[index++] = c.G;
                        imgData[index++] = c.B;
                    }
                }
            } else if (_header.BitsPerPixel == 1 && _header.ColorPlanesCount == 4) {
                for (i32 y {0}; y < height; ++y) {
                    for (i32 x {0}; x < width;) {
                        auto const getColor {[&] -> std::optional<color> {
                            if (x >= width) { return std::nullopt; }

                            u32 c1 {0};
                            for (i32 i {0}; i < 4; i++) {
                                i32 const off {(x / 8) + ((y * bpl * 4) + (bpl * i))};
                                assert(off < std::ssize(data));
                                i32 const b {data[off]};
                                u32 const l {helper::extract_bits(b, 7 - (x % 8), 1)};
                                c1 += l << i;
                            }

                            x++;
                            return palette[c1];
                        }};

                        if (auto col1 {getColor()}) {
                            imgData[index++] = col1->R;
                            imgData[index++] = col1->G;
                            imgData[index++] = col1->B;
                        }

                        if (auto col2 {getColor()}) {
                            imgData[index++] = col2->R;
                            imgData[index++] = col2->G;
                            imgData[index++] = col2->B;
                        }
                    }
                }
            }
        } else if (_header.BitsPerPixel == 8 && _header.ColorPlanesCount == 3) {
            // RGB
            for (i32 y {0}; y < height; ++y) {
                for (i32 x {0}; x < width; ++x) {
                    for (i32 i {0}; i < 3; ++i) {
                        imgData[index++] = data[x + ((y * bpl) * 3) + (bpl * i)];
                    }
                }
            }
        } else if (_header.BitsPerPixel == 1 && _header.ColorPlanesCount == 1) {
            // monochrome
            for (i32 i {0}; i < std::ssize(data); ++i) {
                u8 b {data[i]};
                for (i32 j {0}; j < 8; ++j) {
                    u8 col {helper::extract_bits(b, 7 - j, 1) == 0 ? u8 {0} : u8 {255}};
                    imgData[index++] = col;
                    imgData[index++] = col;
                    imgData[index++] = col;
                }
            }

        } else {
            return std::nullopt;
        }

        return retValue;
    }

    return std::nullopt;
}

auto pcx_decoder::decode_info(io::istream& in) -> std::optional<image::information>
{
    _header.read(in);
    if (_header.Manufacturer == ManufacturerMagicNumber) {
        return image::information {.Size = {_header.Width(), _header.Height()}, .Format = image::format::RGB};
    }

    return std::nullopt;
}

auto pcx_decoder::read_palette(io::istream& in) const -> std::vector<color>
{
    if (_header.BitsPerPixel == 1 && _header.ColorPlanesCount == 4) {
        in.seek(HeaderPaletteOffset, io::seek_dir::Begin);
        return pcx::read_color_palette(in, HeaderPaletteLength / 3);
    }
    if (_header.BitsPerPixel == 8 && _header.ColorPlanesCount == 1) {
        in.seek(in.size_in_bytes() - PaletteOffset, io::seek_dir::Begin);
        if (in.read<u8>() == PaletteMagicNumber) {
            return pcx::read_color_palette(in, PaletteOffset - 1);
        }
    }

    return {};
}

////////////////////////////////////////////////////////////

static auto Compress(std::span<u8> buf, i32 lineWidth) -> std::vector<u8>
{
    std::vector<u8> retValue {};
    retValue.reserve(buf.size());

    for (i32 y {0}; y < std::ssize(buf) / lineWidth; ++y) {
        std::span<u8> line {buf.subspan(lineWidth * y, lineWidth)};

        u8  runcount {1};
        u8  runvalue {line[0]};
        i32 total {0};
        for (i32 x {1}; x < lineWidth; x++) {
            u8 nextValue {line[x]};
            if (nextValue == runvalue) {
                runcount++;
            }
            if (runvalue != nextValue || runcount == 0x3f || x == lineWidth - 1) {
                total += runcount;
                if (runvalue < 0xc0 && runcount == 1) {
                    retValue.push_back(runvalue);
                } else {
                    runcount += 0xc0;
                    retValue.push_back(runcount);
                    retValue.push_back(runvalue);
                }

                if (runcount != 0xff) {
                    runvalue = nextValue;
                    runcount = 1;
                } else if (x < lineWidth) {
                    runvalue = line[x++];
                    runcount = 1;
                }
            }
        }

        if (total < lineWidth) {
            retValue.push_back(static_cast<u8>(lineWidth - total + 0xc0));
            retValue.push_back(runvalue);
        }
    }

    return retValue;
}

auto pcx_encoder::encode(image const& img, io::ostream& out) const -> bool
{
    auto const& info {img.info()};

    auto const pos {out.tell()};
    pcx::header::Write(info, out);
    out.seek(pos + HeaderLength, io::seek_dir::Begin);

    std::vector<u8> buffer(info.Size.Width * info.Size.Height * pcx::BPP);

    for (i32 x {0}; x < info.Size.Width; ++x) {
        usize i {0};
        for (i32 y {0}; y < info.Size.Height; ++y) {
            auto pix {img.get_pixel({x, y})};
            if (pix == colors::Transparent) {
                pix = colors::White;
            }

            buffer[x + (i++ * info.Size.Width)] = pix.R;
            buffer[x + (i++ * info.Size.Width)] = pix.G;
            buffer[x + (i++ * info.Size.Width)] = pix.B;
        }
    }

    out.write<u8>(Compress(buffer, info.Size.Width));

    return true;
}

}
