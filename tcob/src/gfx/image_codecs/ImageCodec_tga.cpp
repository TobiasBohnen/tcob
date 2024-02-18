// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_tga.hpp"

#include <algorithm>

namespace tcob::gfx::detail {

constexpr std::array<ubyte, 18> SIGNATURE {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};

auto static is_rle(tga::header const& h) -> bool
{
    return h.ImageType == tga::image_type::RLEBlackAndWhiteImage
        || h.ImageType == tga::image_type::RLEColorMappedImage
        || h.ImageType == tga::image_type::RLETrueColorImage;
}

auto static is_truecolor(tga::header const& h) -> bool
{
    return h.ImageType == tga::image_type::RLETrueColorImage || h.ImageType == tga::image_type::UncompressedTrueColorImage;
}

auto static is_colormapped(tga::header const& h) -> bool
{
    return h.ColorMapIncluded && (h.ImageType == tga::image_type::RLEColorMappedImage || h.ImageType == tga::image_type::UncompressedColorMappedImage);
}

auto static is_blackandwhite(tga::header const& h) -> bool
{
    return h.ImageType == tga::image_type::RLEBlackAndWhiteImage || h.ImageType == tga::image_type::UncompressedBlackAndWhiteImage;
}

auto static check_supported_format(tga::header const& h) -> bool
{
    if (!h.ColorMapIncluded) {
        if (is_truecolor(h)) {
            return h.ImageSpecs.BytesPerPixel == 3 || h.ImageSpecs.BytesPerPixel == 4;
        }
        if (is_blackandwhite(h)) {
            return h.ImageSpecs.BytesPerPixel == 1;
        }
    } else {
        return h.ImageSpecs.BytesPerPixel == 1;
    }

    return false;
}

auto static get_bits(i32 i, i32 offset, i32 count) -> i32
{
    return (i >> offset) & ((1 << count) - 1);
}

auto static get_padding_bytes(i32 bitsPerPixel, i32 stride, i32 width) -> i32
{
    if (bitsPerPixel < 8) {
        return stride - static_cast<i32>(std::ceil(width * (bitsPerPixel / 8.0f)));
    }

    return stride - (width * (bitsPerPixel / 8));
}

auto static get_color(u8 one, u8 two) -> color
{
    i32 r1 {get_bits(one, 2, 5)};
    u8  r {static_cast<u8>(r1 << 3)};

    i32 bit {get_bits(one, 0, 2)};
    i32 g1 {bit << 6};

    bit = get_bits(two, 5, 3);
    i32 g2 {bit << 3};
    u8  g {static_cast<u8>(g1 + g2)};

    i32 b1 {get_bits(two, 0, 5)};
    u8  b {static_cast<u8>(b1 << 3)};

    i32 a1 {get_bits(one, 7, 1)};
    u8  a {static_cast<u8>(a1 * 255)};

    return {r, g, b, a};
}

auto tga::read_color_map(istream& reader, i32 colorMapLength, i32 colorMapEntrySize) -> std::vector<color>
{
    std::vector<color> retValue;
    retValue.reserve(colorMapLength);

    for (i32 i {0}; i < colorMapLength; ++i) {
        switch (colorMapEntrySize) {
        case 15:
        case 16: {
            std::array<u8, 2> color16 {};
            reader.read_to<u8>(color16);
            retValue.push_back(get_color(color16[1], color16[0]));
        } break;
        case 24: {
            u8 b {reader.read<u8>()};
            u8 g {reader.read<u8>()};
            u8 r {reader.read<u8>()};
            retValue.emplace_back(r, g, b, 255);
        } break;
        case 32: {
            u8 b {reader.read<u8>()};
            u8 g {reader.read<u8>()};
            u8 r {reader.read<u8>()};
            u8 a {reader.read<u8>()};
            retValue.emplace_back(r, g, b, a);
        } break;
        }
    }

    return retValue;
}

auto tga::read_data(istream& reader, header const& h) -> std::vector<u8>
{
    i32 const imgRowSize {h.ImageSpecs.Width * h.ImageSpecs.BytesPerPixel};
    i32 const imgSize {imgRowSize * h.ImageSpecs.Height};

    std::vector<std::vector<u8>> rows;
    std::vector<u8>              row;

    if (is_rle(h)) {
        u8              rlePacket {};
        std::vector<u8> rlePixel;

        i64 bytesRead {0};
        i64 rowBytesRead {0};

        while (bytesRead < imgSize) {
            rlePacket = reader.read<u8>();
            auto const rlePacketType {static_cast<rle_packet_type>(rlePacket >> 7)};

            switch (rlePacketType) {
            case rle_packet_type::Compressed: {
                i32 rlePixelCount {rlePacket - 127};
                rlePixel.resize(h.ImageSpecs.BytesPerPixel);
                reader.read_to<u8>(rlePixel);
                for (i32 i {0}; i < rlePixelCount; ++i) {
                    row.insert(row.end(), rlePixel.begin(), rlePixel.end());

                    rowBytesRead += rlePixel.size();
                    bytesRead += rlePixel.size();

                    if (rowBytesRead == imgRowSize) {
                        rows.push_back(row);
                        row.clear();
                        rowBytesRead = 0;
                    }
                }
            } break;

            case rle_packet_type::Raw: {
                i32 bytesToRead {(rlePacket + 1) * h.ImageSpecs.BytesPerPixel};

                for (i32 i {0}; i < bytesToRead; ++i) {
                    row.push_back(reader.read<u8>());

                    bytesRead++;
                    rowBytesRead++;

                    if (rowBytesRead == imgRowSize) {
                        rows.push_back(row);
                        row.clear();
                        rowBytesRead = 0;
                    }
                }
            } break;
            }
        }

    } else {
        for (i32 i {0}; i < h.ImageSpecs.Height; ++i) {
            for (i32 j {0}; j < imgRowSize; ++j) {
                row.push_back(reader.read<u8>());
            }

            rows.push_back(row);
            row.clear();
        }
    }

    bool flipV {false}, flipH {false};

    switch (h.ImageSpecs.ImageDescriptor.FirstPixelDestination) {
    case first_pixel_destination::TopLeft:
        flipV = flipH = false;
        break;

    case first_pixel_destination::TopRight:
        flipV = false;
        flipH = true;
        break;

    case first_pixel_destination::BottomLeft:
        flipV = true;
        flipH = false;
        break;

    case first_pixel_destination::BottomRight:
        flipV = flipH = true;
        break;
    }

    if (flipV) {
        std::reverse(rows.begin(), rows.end());
    }

    std::vector<u8> retValue;
    for (auto& r : rows) {
        if (flipH) {
            std::reverse(r.begin(), r.end());
        }

        retValue.insert(retValue.end(), r.begin(), r.end());
    }

    if (is_truecolor(h)) {
        // BGRA -> RGBA
        for (i32 i {0}; i < std::ssize(retValue); i += h.ImageSpecs.BytesPerPixel) {
            std::swap(retValue[i], retValue[i + 2]);
        }
    } else if (is_colormapped(h)) {
        for (i32 i {0}; i < std::ssize(retValue); ++i) {
            if (retValue[i] >= h.ColorMapSpecs.ColorMapLength) {
                retValue[i] = 0;
            }
        }
    }

    return retValue;
}

void tga::footer::read(istream& reader)
{
    reader.seek(-Offset + SignatureOffset, io::seek_dir::End);

    // try reading the signature
    std::array<ubyte, SIGNATURE.size()> sig {};
    reader.read_to<ubyte>(sig);

    if (sig == SIGNATURE) {
        Format = format::New;

        reader.seek(-(SignatureOffset + static_cast<i32>(SIGNATURE.size())), io::seek_dir::Current);
        ExtensionAreaOffset      = reader.read<u32>(std::endian::little);
        DeveloperDirectoryOffset = reader.read<u32>(std::endian::little);
    } else {
        Format = format::Original;
    }

    reader.seek(0, io::seek_dir::Begin); // FIXME: store position
}

void tga::image_descriptor::read(istream& reader)
{
    u8 imgDesc            = reader.read<u8>();
    AttributeBits         = get_bits(imgDesc, 0, 4);
    FirstPixelDestination = static_cast<first_pixel_destination>(get_bits(imgDesc, 4, 2));
}

void tga::image_specifications::read(istream& reader)
{
    XOrigin       = reader.read<u16>(std::endian::little);
    YOrigin       = reader.read<u16>(std::endian::little);
    Width         = reader.read<u16>(std::endian::little);
    Height        = reader.read<u16>(std::endian::little);
    PixelDepth    = reader.read<u8>();
    BytesPerPixel = PixelDepth / 8;

    ImageDescriptor.read(reader);
}

void tga::color_map_specifications::read(istream& reader)
{
    FirstEntryIndex   = reader.read<u16>(std::endian::little);
    ColorMapLength    = reader.read<u16>(std::endian::little);
    ColorMapEntrySize = reader.read<u8>();

    i32 bytes {0};
    switch (ColorMapEntrySize) {
    case 15:
    case 16:
        bytes = 2;
        break;
    case 24:
        bytes = 3;
        break;
    case 32:
        bytes = 4;
        break;
    }
    ColorMapTotalSize = ColorMapLength * bytes;
}

void tga::header::read(istream& reader)
{
    IDLength         = reader.read<u8>();
    ColorMapIncluded = reader.read<color_map_type>() == tga::color_map_type::ColorMapIncluded;
    ImageType        = reader.read<image_type>();

    // Color Map Specs
    ColorMapSpecs.read(reader);

    // Image Specs
    ImageSpecs.read(reader);
}

////////////////////////////////////////////////////////////

auto tga_decoder::decode(istream& in) -> std::optional<image>
{
    if (decode_header(in)) {
        in.seek(18 + _header.IDLength, io::seek_dir::Begin); // skip ID
        auto const colorMap {tga::read_color_map(in, _header.ColorMapSpecs.ColorMapLength, _header.ColorMapSpecs.ColorMapEntrySize)};

        auto const   imgData {tga::read_data(in, _header)};
        size_i const imgSize {_header.ImageSpecs.Width, _header.ImageSpecs.Height};
        if (std::ssize(imgData) != imgSize.Width * imgSize.Height * _header.ImageSpecs.BytesPerPixel) {
            return {};               // invalid image data
        }

        if (is_truecolor(_header)) { // RGBA
            if (_header.ImageSpecs.BytesPerPixel == 4) {
                return image::Create(imgSize, image::format::RGBA, imgData);
            }
            if (_header.ImageSpecs.BytesPerPixel == 3) {
                return image::Create(imgSize, image::format::RGB, imgData);
            }
            return std::nullopt;
        }

        std::vector<u8> pixels;
        pixels.resize(imgSize.Width * imgSize.Height * 4);
        u8* pixPtr {pixels.data()};

        if (is_colormapped(_header) && !colorMap.empty()) {
            // indexed
            for (u8 ind : imgData) {
                auto [r, g, b, a] {colorMap[ind]};
                *pixPtr++ = r;
                *pixPtr++ = g;
                *pixPtr++ = b;
                *pixPtr++ = a;
            }
        } else if (is_blackandwhite(_header)) {
            // grayscale
            for (u8 ind : imgData) {
                *pixPtr++ = ind;
                *pixPtr++ = ind;
                *pixPtr++ = ind;
                *pixPtr++ = 255;
            }
        }

        return image::Create(imgSize, image::format::RGBA, pixels);
    }

    return std::nullopt;
}

auto tga_decoder::decode_header(istream& in) -> std::optional<image::info>
{
    _footer.read(in);
    _header.read(in);

    i32 bpp {_header.ImageSpecs.BytesPerPixel};
    if (!is_truecolor(_header)) {
        bpp = 4;
    }

    return _footer.Format == tga::format::New && check_supported_format(_header)
        ? std::optional {image::info {{_header.ImageSpecs.Width, _header.ImageSpecs.Height}, bpp == 4 ? image::format::RGBA : image::format::RGB}}
        : std::nullopt;
}

////////////////////////////////////////////////////////////

auto tga_encoder::encode(image const& img, ostream& out) const -> bool
{
    write_header(img.get_info(), out);
    write_image_data(img, out);

    auto extOffset {out.tell()};
    write_extension_area(out);

    write_footer(out, extOffset);

    return true;
}

void tga_encoder::write_header(image::info const& image, ostream& out) const
{
    out.write<u8>(0);
    out.write<u8>(static_cast<u8>(tga::color_map_type::NoColorMapIncluded));
    out.write<u8>(static_cast<u8>(tga::image_type::RLETrueColorImage));
    std::array<u8, 5> colorMapSpec {};
    colorMapSpec.fill(0);
    out.write(colorMapSpec);

    out.write<u16>(0, std::endian::little);
    out.write<u16>(0, std::endian::little);
    out.write<u16>(static_cast<u16>(image.Size.Width), std::endian::little);
    out.write<u16>(static_cast<u16>(image.Size.Height), std::endian::little);
    out.write<u8>(static_cast<u8>(image.bytes_per_pixel() * 8));
    out.write<u8>(40);
}

void tga_encoder::write_image_data(image const& img, ostream& out) const
{
    std::vector<u8> rle;
    std::vector<u8> rawPackage;
    std::vector<u8> rlePackage;

    auto const& imgInfo {img.get_info()};
    i32 const   bytesPerPixel {static_cast<i32>(imgInfo.bytes_per_pixel())};
    i32 const   paddingByteCount {get_padding_bytes(bytesPerPixel * 8, imgInfo.stride(), imgInfo.Size.Width)};
    i32 const   sizeInBytes {static_cast<i32>(imgInfo.size_in_bytes())};

    // RGBA -> BGRA
    image      imgCopy {img};
    auto const data {imgCopy.get_data()};
    for (i32 i {0}; i < sizeInBytes; i += bytesPerPixel) {
        u8 buf {data[i]};
        data[i]     = data[i + 2];
        data[i + 2] = buf;
    }

    i32 index {0};
    for (i32 y {0}; y < imgInfo.Size.Height; ++y) {
        i32 runCount {-1};

        for (i32 x {0}; x < imgInfo.Size.Width; ++x) {
            bool isRle {true};
            for (i32 i {0}; i < bytesPerPixel; ++i) {
                if (index + i + bytesPerPixel < sizeInBytes) {
                    isRle &= (data[index + i] == data[index + i + bytesPerPixel]);
                }
            }

            if (!isRle) {
                if (!rlePackage.empty()) {
                    rle.push_back(static_cast<u8>(128 + runCount));
                    rle.insert(rle.end(), rlePackage.begin(), rlePackage.end());
                    rlePackage.clear();
                    runCount = -1;
                }

                for (i32 i {0}; i < bytesPerPixel; ++i) {
                    rawPackage.push_back(data[index + i]);
                }
            } else {
                if (!rawPackage.empty()) {
                    rle.push_back(static_cast<u8>(runCount));
                    rle.insert(rle.end(), rawPackage.begin(), rawPackage.end());
                    rawPackage.clear();
                    runCount = -1;
                }

                if (rlePackage.empty()) {
                    for (i32 i {0}; i < bytesPerPixel; ++i) {
                        rlePackage.push_back(data[index + i]);
                    }
                }
            }

            runCount++;
            if (runCount == 127 || x == imgInfo.Size.Width - 1) {
                if (!rawPackage.empty()) {
                    rle.push_back(static_cast<u8>(runCount));
                    rle.insert(rle.end(), rawPackage.begin(), rawPackage.end());
                    rawPackage.clear();
                }

                if (!rlePackage.empty()) {
                    rle.push_back(static_cast<u8>(128 + runCount));
                    rle.insert(rle.end(), rlePackage.begin(), rlePackage.end());
                    rlePackage.clear();
                }

                runCount = -1;
            }

            index += bytesPerPixel;
        }

        index += paddingByteCount;
    }

    out.write<u8>(rle);
}

void tga_encoder::write_extension_area(ostream& out) const
{
    out.write<u16>(495);

    std::array<u8, 492> buffer {};
    buffer.fill(0);
    out.write(buffer);

    out.write<u8>(static_cast<u8>(tga::attribute_type::Alpha));
}

void tga_encoder::write_footer(ostream& out, std::streamsize extOffset) const
{
    out.write<u32>(static_cast<u32>(extOffset));
    out.write<ubyte>(SIGNATURE);
}

}
