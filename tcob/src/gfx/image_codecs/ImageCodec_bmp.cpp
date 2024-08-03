// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_bmp.hpp"

namespace tcob::gfx::detail {

namespace bmp {

    void bitmap_file_header::read(istream& reader)
    {
        Signature = reader.read<u16>();
        Size      = reader.read<u32>();
        reader.read<u32>();
        BitsOffset = reader.read<u32>();
    }

    void bitmap_info_header::read(istream& reader)
    {
        read40(reader);
        RedMask   = reader.read<u32>();
        GreenMask = reader.read<u32>();
        BlueMask  = reader.read<u32>();
        AlphaMask = reader.read<u32>();
        CsType    = reader.read<u32>();
        reader.read_to<u32>(Endpoints);
        GammaRed   = reader.read<u32>();
        GammaGreen = reader.read<u32>();
        GammaBlue  = reader.read<u32>();
    }

    void bitmap_info_header::read40(istream& reader)
    {
        HeaderSize    = reader.read<u32>();
        Width         = reader.read<i32>();
        Height        = reader.read<i32>();
        Planes        = reader.read<u16>();
        BitCount      = reader.read<u16>();
        Compression   = static_cast<compression>(reader.read<u32>());
        SizeImage     = reader.read<u32>();
        PelsPerMeterX = reader.read<i32>();
        PelsPerMeterY = reader.read<i32>();
        ClrUsed       = reader.read<u32>();
        ClrImportant  = reader.read<u32>();
    }
}

constexpr i16 SIGNATURE {0x4d42};

////////////////////////////////////////////////////////////

auto bmp_decoder::decode(istream& in) -> std::optional<image>
{
    auto const offset {in.tell()};

    if (auto info {decode_info(in)}) {
        in.seek(offset + _infoHeader.HeaderSize + 14, io::seek_dir::Begin);

        auto const palette {get_palette(in)};

        size_i const size {
            _infoHeader.Width < 0 ? -_infoHeader.Width : _infoHeader.Width,
            _infoHeader.Height < 0 ? -_infoHeader.Height : _infoHeader.Height};
        u16 const bitCount {_infoHeader.BitCount};

        in.seek(offset + _header.BitsOffset, io::seek_dir::Begin);

        std::vector<u8> data;
        if (_infoHeader.Compression == bmp::compression::Rgb) {
            data = get_rgb_data(in, size, bitCount, palette);
        } else if (_infoHeader.Compression == bmp::compression::Rle8) {
            return std::nullopt; // TODO
        } else if (_infoHeader.Compression == bmp::compression::Rle4) {
            return std::nullopt; // TODO
        } else if (_infoHeader.Compression == bmp::compression::Bitfields) {
            return std::nullopt; // TODO
        } else {
            return std::nullopt;
        }

        if (!data.empty()) {
            return image::Create(size, image::format::RGBA, data);
        }
    }

    return std::nullopt;
}

auto bmp_decoder::decode_info(istream& in) -> std::optional<image::info>
{
    _header.read(in);
    if (_header.Signature == SIGNATURE) {
        _infoHeader.read(in);
        _info = image::info {{_infoHeader.Width, _infoHeader.Height}, image::format::RGBA};
        return _info;
    }
    return std::nullopt;
}

auto bmp_decoder::get_palette(istream& in) const -> std::vector<color>
{
    u32 colorTableSize {0};
    if (_infoHeader.BitCount == 1) {
        colorTableSize = 2;
    } else if (_infoHeader.BitCount == 4) {
        colorTableSize = 16;
    } else if (_infoHeader.BitCount == 8) {
        colorTableSize = 256;
    }

    if (colorTableSize > 0) {
        std::vector<color> palette {};
        palette.reserve(colorTableSize);

        for (u32 i {0}; i < colorTableSize; i++) {
            u8 const b {in.read<u8>()};
            u8 const g {in.read<u8>()};
            u8 const r {in.read<u8>()};
            u8 const a {in.read<u8>()};
            palette.emplace_back(r, g, b, a);
        }

        return palette;
    }

    return {};
}

void static CheckAlpha(std::vector<u8>& data)
{
    bool check {false};
    for (u32 i {3}; i < data.size(); i += 4) {
        if (data[i] != 0) {
            check = true;
            break;
        }
    }

    if (!check) {
        for (u32 i {3}; i < data.size(); i += 4) {
            data[i] = 255;
        }
    }
}

auto bmp_decoder::get_rgb_data(istream& in, size_i size, u16 bitCount, std::vector<color> palette) const -> std::vector<u8>
{
    std::vector<u8> retValue;
    retValue.resize(_info.size_in_bytes());

    auto const [width, height] {size};
    i32 const srcStride {static_cast<i32>(std::ceil(width * bitCount / 32.f) * 4)};
    i32 const dstStride {_info.stride()};

    if (bitCount == 1) {
        return {};
    }

    if (bitCount == 16) {
        return {};
    }

    if (bitCount == 4) {
        i32 index {dstStride * (height - 1)};

        for (i32 y {0}; y < height; y++) {
            for (i32 x {0}; x < width;) {
                u8 const idx {in.read<u8>()};

                u8 const idx1 {static_cast<u8>(idx >> 4)};
                if (idx1 < palette.size()) {
                    color const c {palette[idx1]};
                    retValue[index++] = c.R;
                    retValue[index++] = c.G;
                    retValue[index++] = c.B;
                    retValue[index++] = c.A == 0 ? 255 : c.A;
                }

                x++;

                if (x < width) {
                    u8 const idx2 {static_cast<u8>(idx & 0x0F)};
                    if (idx2 < palette.size()) {
                        color const c {palette[idx2]};
                        retValue[index++] = c.R;
                        retValue[index++] = c.G;
                        retValue[index++] = c.B;
                        retValue[index++] = c.A == 0 ? 255 : c.A;
                    }

                    x++;
                }
            }

            index -= dstStride * 2;
        }
    } else if (bitCount == 8) {
        i32 pad {srcStride - width};
        i32 index {dstStride * (height - 1)};

        for (i32 y {0}; y < height; y++) {
            for (i32 x {0}; x < width; x++) {
                u8 const idx {in.read<u8>()};
                if (idx < palette.size()) {
                    color const c {palette[idx]};
                    retValue[index++] = c.R;
                    retValue[index++] = c.G;
                    retValue[index++] = c.B;
                    retValue[index++] = c.A == 0 ? 255 : c.A;
                }
            }

            index -= dstStride * 2;
            in.read_n<u8>(pad);
        }
    } else if (bitCount == 24) {
        i32       index {dstStride * (height - 1)};
        i32 const pad {srcStride - width * 3};

        for (i32 y {0}; y < height; y++) {
            for (i32 x {0}; x < width; x++) {
                u8 const b {in.read<u8>()};
                u8 const g {in.read<u8>()};
                u8 const r {in.read<u8>()};

                retValue[index++] = r;
                retValue[index++] = g;
                retValue[index++] = b;
                retValue[index++] = 255;
            }

            index -= dstStride * 2;
            in.read_n<u8>(pad);
        }
    } else if (bitCount == 32) {
        i32 index {dstStride * (height - 1)};
        for (i32 y {0}; y < height; y++) {
            for (i32 x {0}; x < width; x++) {
                u8 const b {in.read<u8>()};
                u8 const g {in.read<u8>()};
                u8 const r {in.read<u8>()};
                u8 const a {in.read<u8>()};

                retValue[index++] = r;
                retValue[index++] = g;
                retValue[index++] = b;
                retValue[index++] = a;
            }

            index -= srcStride * 2;
        }

        CheckAlpha(retValue);
    }

    return retValue;
}

////////////////////////////////////////////////////////////

void static WriteFileHeader(std::streamsize imageOffset, std::streamsize fileSize, ostream& writer)
{
    writer.write(SIGNATURE);
    writer.write<u32>(static_cast<u32>(fileSize));
    writer.write(0);
    writer.write<u32>(static_cast<u32>(imageOffset));
}

void static WriteImageData(image const& img, ostream& writer)
{
    auto const& info {img.get_info()};
    auto const  data {img.get_data()};
    i32 const   stride {info.stride()};
    i32 const   bpp {info.bytes_per_pixel()};

    std::vector<u8> buffer {};
    buffer.resize(info.Size.Height * info.Size.Width * bmp::BPP);

    usize dstIndex {0};
    for (i32 y {info.Size.Height - 1}; y >= 0; y--) {
        i32 srcIndex {stride * y};
        for (i32 x {0}; x < info.Size.Width; x++) {
            buffer[dstIndex++] = data[srcIndex + 2];
            buffer[dstIndex++] = data[srcIndex + 1];
            buffer[dstIndex++] = data[srcIndex + 0];
            buffer[dstIndex++] = info.Format == image::format::RGBA ? data[srcIndex + 3] : 255;
            srcIndex += bpp;
        }
    }

    writer.write<u8>(buffer);
}

void static WriteInfoHeader(image::info const& info, ostream& writer)
{
    writer.write(40);
    writer.write<i32>(info.Size.Width);
    writer.write<i32>(info.Size.Height);
    writer.write<i16>(1);
    writer.write<i16>(32);
    writer.write<i32>(static_cast<i32>(bmp::compression::Rgb));
    writer.write<u32>(static_cast<u32>(info.size_in_bytes()));
    writer.write(0);
    writer.write(0);
    writer.write(0);
    writer.write(0);
}

auto bmp_encoder::encode(image const& img, ostream& out) const -> bool
{
    auto const& info {img.get_info()};

    auto startPos {out.tell()};
    out.seek(14, io::seek_dir::Current);

    WriteInfoHeader(info, out);

    auto imageOffset {out.tell()};
    WriteImageData(img, out);

    auto endPos {out.tell()};
    out.seek(startPos, io::seek_dir::Begin);
    WriteFileHeader(imageOffset, (endPos - startPos), out);
    out.seek(endPos, io::seek_dir::Begin);

    return true;
}
}
