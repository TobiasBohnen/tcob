// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_qoi.hpp"

#include <array>
#include <bit>
#include <optional>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {

static constexpr std::array<byte, 8> padding {0, 0, 0, 0, 0, 0, 0, 1};
static constexpr std::array<u8, 4>   magic {'q', 'o', 'i', 'f'};

static constexpr byte QOI_OP_RGB {0b11111110};
static constexpr byte QOI_OP_RGBA {0b11111111};
static constexpr byte QOI_OP_RUN {0b11000000};
static constexpr byte QOI_OP_INDEX {0b00000000};
static constexpr byte QOI_OP_DIFF {0b01000000};
static constexpr byte QOI_OP_LUMA {0b10000000};

auto static to_index(color c) -> usize
{
    return ((c.R * 3) + (c.G * 5) + (c.B * 7) + (c.A * 11)) % 64;
}

auto qoi_decoder::decode(io::istream& in) -> std::optional<image>
{
    if (auto info {decode_info(in)}) {
        i32 const  width {info->Size.Width};
        i32 const  height {info->Size.Height};
        auto const format {info->Format};
        i32 const  bpp {info->bytes_per_pixel()};

        color                 prevPixel {0, 0, 0, 255};
        std::array<color, 64> indexCache {};

        image retValue {image::CreateEmpty({width, height}, format)};
        auto* imgData {retValue.ptr()};

        for (isize i {0}; i < info->size_in_bytes(); i += bpp) {
            if (in.is_eof()) { return std::nullopt; }

            byte const tag8 {in.read<byte>()};
            if (tag8 == QOI_OP_RGB) {
                prevPixel.R = in.read<u8>();
                prevPixel.G = in.read<u8>();
                prevPixel.B = in.read<u8>();
            } else if (tag8 == QOI_OP_RGBA) {
                prevPixel.R = in.read<u8>();
                prevPixel.G = in.read<u8>();
                prevPixel.B = in.read<u8>();
                prevPixel.A = in.read<u8>();
            } else {
                byte const tag2 {static_cast<byte>(helper::extract_bits(tag8, 6, 2))};
                if (tag2 == QOI_OP_INDEX) {
                    byte const idx {static_cast<byte>(helper::extract_bits(tag8, 0, 6))};
                    prevPixel = indexCache[idx];
                } else if (tag2 == QOI_OP_DIFF >> 6) {
                    i32 const dr {static_cast<i32>(helper::extract_bits(tag8, 4, 2)) - 2};
                    i32 const dg {static_cast<i32>(helper::extract_bits(tag8, 2, 2)) - 2};
                    i32 const db {static_cast<i32>(helper::extract_bits(tag8, 0, 2)) - 2};
                    prevPixel.R += dr;
                    prevPixel.G += dg;
                    prevPixel.B += db;
                } else if (tag2 == QOI_OP_LUMA >> 6) {
                    byte const tag8_2 {in.read<byte>()};
                    i32 const  dr {static_cast<i32>(helper::extract_bits(tag8_2, 4, 4)) - 8};
                    i32 const  dg {static_cast<i32>(helper::extract_bits(tag8, 0, 6)) - 32};
                    i32 const  db {static_cast<i32>(helper::extract_bits(tag8_2, 0, 4)) - 8};
                    prevPixel.R += dg + dr;
                    prevPixel.G += dg;
                    prevPixel.B += dg + db;
                } else if (tag2 == QOI_OP_RUN >> 6) {
                    byte const rl {static_cast<byte>(helper::extract_bits(tag8, 0, 6))};
                    for (u32 j {0}; j < rl; ++j) {
                        imgData[i + 0] = prevPixel.R;
                        imgData[i + 1] = prevPixel.G;
                        imgData[i + 2] = prevPixel.B;
                        if (format == image::format::RGBA) { imgData[i + 3] = prevPixel.A; }
                        i += info->bytes_per_pixel();
                    }
                } else {
                    return std::nullopt;
                }
            }

            imgData[i + 0] = prevPixel.R;
            imgData[i + 1] = prevPixel.G;
            imgData[i + 2] = prevPixel.B;
            if (format == image::format::RGBA) { imgData[i + 3] = prevPixel.A; }

            indexCache[to_index(prevPixel)] = prevPixel;
        }

        if (in.read_n<byte, 8>() != padding) { return std::nullopt; }
        return retValue;
    }

    return std::nullopt;
}

auto qoi_decoder::decode_info(io::istream& in) -> std::optional<image::information>
{
    if (in.read_n<u8, 4>() != magic) { return std::nullopt; }

    i32 const w {static_cast<i32>(in.read<u32, std::endian::big>())};
    i32 const h {static_cast<i32>(in.read<u32, std::endian::big>())};
    u8 const  bpp {in.read<u8>()};
    in.read<u8>(); // colorspace
    return image::information {.Size = {w, h}, .Format = bpp == 3 ? image::format::RGB : image::format::RGBA};
}

////////////////////////////////////////////////////////////

auto qoi_encoder::encode(image const& image, io::ostream& out) const -> bool
{
    auto const& info {image.info()};
    i32 const   bpp {info.bytes_per_pixel()};
    auto const  size {info.size_in_bytes()};

    color                 prevPixel {0, 0, 0, 255};
    std::array<color, 64> indexCache {};
    i32                   run {0};

    // header
    out.write(magic);
    out.write<u32, std::endian::big>(info.Size.Width);
    out.write<u32, std::endian::big>(info.Size.Height);
    out.write<u8>(bpp);
    out.write<u8>(1);

    // content
    auto const imgData {image.data()};
    for (isize i {0}; i < size; i += bpp) {
        color currPixel {imgData[i + 0], imgData[i + 1], imgData[i + 2], 255};
        if (bpp == 4) {
            currPixel.A = imgData[i + 3];
        }

        if (currPixel == prevPixel) {
            ++run;
            if (run == 62 || i + bpp >= size) {
                out.write<byte>(QOI_OP_RUN | (run - 1));
                run = 0;
            }
        } else {
            if (run > 0) {
                out.write<byte>(QOI_OP_RUN | (run - 1));
                run = 0;
            }

            auto const idx {to_index(currPixel)};
            if (indexCache[idx] == currPixel) {
                out.write<byte>(QOI_OP_INDEX | idx);
            } else {
                if (currPixel.A == prevPixel.A) {
                    i32 const dr {static_cast<i32>(currPixel.R) - static_cast<i32>(prevPixel.R)};
                    i32 const dg {static_cast<i32>(currPixel.G) - static_cast<i32>(prevPixel.G)};
                    i32 const db {static_cast<i32>(currPixel.B) - static_cast<i32>(prevPixel.B)};

                    if (dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1) {
                        out.write<byte>(QOI_OP_DIFF | ((dr + 2) << 4) | ((dg + 2) << 2) | (db + 2));
                    } else if (dg >= -32 && dg <= 31 && (dr - dg) >= -8 && (dr - dg) <= 7 && (db - dg) >= -8 && (db - dg) <= 7) {
                        out.write<byte>(QOI_OP_LUMA | (dg + 32));
                        out.write<byte>(((dr - dg + 8) << 4) | (db - dg + 8));
                    } else {
                        out.write<byte>(QOI_OP_RGB);
                        out.write<byte>(currPixel.R);
                        out.write<byte>(currPixel.G);
                        out.write<byte>(currPixel.B);
                    }
                } else {
                    out.write<byte>(QOI_OP_RGBA);
                    out.write<byte>(currPixel.R);
                    out.write<byte>(currPixel.G);
                    out.write<byte>(currPixel.B);
                    out.write<byte>(currPixel.A);
                }
            }

            indexCache[idx] = currPixel;
            prevPixel       = currPixel;
        }
    }

    // padding
    out.write(padding);

    return true;
}

}
