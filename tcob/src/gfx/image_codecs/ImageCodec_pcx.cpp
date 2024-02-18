// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_pcx.hpp"

namespace tcob::gfx::detail {

constexpr i32  HeaderPaletteLength {48};
constexpr byte ManufacturerMagicNumber {0x0a};
constexpr byte PaletteMagicNumber {12};
constexpr i32  PaletteOffset {769};
constexpr i32  HeaderLength {128};

auto pcx::read_image_data(istream& reader, header const& h) -> std::vector<u8>
{
    reader.seek(HeaderLength, io::seek_dir::Begin);

    std::vector<u8> retValue;
    retValue.resize(h.BytesPerLine * h.ColorPlanesCount * h.Height());

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

auto pcx::read_color_palette(istream& reader, i32 size) -> std::vector<color>
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

void pcx::header::read(istream& reader)
{
    Manufacturer = reader.read<u8>();

    Version      = static_cast<version>(reader.read<u8>());
    Encoding     = static_cast<encoding>(reader.read<u8>());
    BitsPerPixel = reader.read<u8>();
    XMin         = reader.read<u16>(std::endian::little);
    YMin         = reader.read<u16>(std::endian::little);
    XMax         = reader.read<u16>(std::endian::little);
    YMax         = reader.read<u16>(std::endian::little);
    HortDPI      = reader.read<u16>(std::endian::little);
    VertDPI      = reader.read<u16>(std::endian::little);

    // skip palette
    reader.seek(HeaderPaletteLength, io::seek_dir::Current);

    Reserved         = reader.read<u8>();
    ColorPlanesCount = reader.read<u8>();
    BytesPerLine     = reader.read<u16>(std::endian::little);
    PaletteType      = static_cast<palette_type>(reader.read<u16>(std::endian::little));
    HScrSize         = reader.read<u16>(std::endian::little);
    VScrSize         = reader.read<u16>(std::endian::little);
}

void pcx::header::Write(image::info const& info, ostream& writer)
{
    byte bpp {8}, cpc {3};

    writer.write(ManufacturerMagicNumber);
    writer.write(static_cast<byte>(5));
    writer.write(static_cast<byte>(1));
    writer.write(bpp);
    writer.write(static_cast<i16>(0));
    writer.write(static_cast<i16>(0));
    writer.write(static_cast<i16>(info.Size.Width - 1));
    writer.write(static_cast<i16>(info.Size.Height - 1));

    writer.write(static_cast<i16>(72));
    writer.write(static_cast<i16>(72));

    writer.seek(HeaderPaletteLength, io::seek_dir::Current);

    writer.write(static_cast<byte>(0));
    writer.write(cpc);

    writer.write(static_cast<i16>(info.Size.Width));
    writer.write(static_cast<byte>(1));
    writer.write(static_cast<i16>(0));
    writer.write(static_cast<i16>(0));
}

////////////////////////////////////////////////////////////

auto pcx_decoder::decode(istream& in) -> std::optional<image>
{
    if (decode_header(in)) {
        auto const palette {read_palette(in)};
        auto const imageData {pcx::read_image_data(in, _header)};

        i32 const       width {_header.Width()};
        i32 const       height {_header.Height()};
        std::vector<u8> imgData;
        imgData.resize(width * pcx::BPP * height);

        if (!palette.empty()) {
            // indexed
            if (_header.BitsPerPixel == 8 && _header.ColorPlanesCount == 1) {
                i32 index {0}, srcIndex {0};
                for (i32 y {0}; y < height; y++) {
                    for (i32 x {0}; x < width; x++) {
                        color const c {palette[imageData[srcIndex++]]};
                        imgData[index++] = c.R;
                        imgData[index++] = c.G;
                        imgData[index++] = c.B;
                    }
                }
            }
        } else if (_header.BitsPerPixel == 8 && _header.ColorPlanesCount == 3) {
            // RGB
            i16 const bpl {_header.BytesPerLine};
            i32       index {0};
            for (i32 y {0}; y < height; ++y) {
                for (i32 x {0}; x < width; ++x) {
                    for (i32 i {0}; i < 3; ++i) {
                        imgData[index++] = imageData[x + ((y * bpl) * 3) + (bpl * i)];
                    }
                }
            }
        } else {
            return std::nullopt;
        }

        return image::Create({width, height}, image::format::RGB, imgData);
    }

    return std::nullopt;
}

auto pcx_decoder::decode_header(istream& in) -> std::optional<image::info>
{
    _header.read(in);
    if (_header.Manufacturer == ManufacturerMagicNumber) {
        return image::info {{_header.Width(), _header.Height()}, image::format::RGB};
    }

    return std::nullopt;
}

auto pcx_decoder::read_palette(istream& in) const -> std::vector<color>
{
    if (_header.BitsPerPixel == 8 && _header.ColorPlanesCount == 1) {
        in.seek(in.size_in_bytes() - PaletteOffset, io::seek_dir::Begin);
        if (in.read<u8>() == PaletteMagicNumber) {
            return pcx::read_color_palette(in, PaletteOffset - 1);
        }
    }

    return {};
}

////////////////////////////////////////////////////////////

auto static Compress(std::span<u8> buf, i32 lineWidth) -> std::vector<u8>
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

auto pcx_encoder::encode(image const& img, ostream& out) const -> bool
{
    auto const& info {img.get_info()};

    auto const pos {out.tell()};
    pcx::header::Write(info, out);
    out.seek(pos + HeaderLength, io::seek_dir::Begin);

    std::vector<u8> buffer {};
    buffer.resize(info.Size.Width * info.Size.Height * pcx::BPP);

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
