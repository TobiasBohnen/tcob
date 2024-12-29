// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

namespace bmp {
    constexpr i32 BPP {4};

    class bitmap_file_header {
    public:
        u32 BitsOffset {0};
        u16 Signature {0};
        u32 Size {0};

        void read(io::istream& reader);
    };

    enum class compression : u8 {
        Rgb       = 0,
        Rle8      = 1,
        Rle4      = 2,
        Bitfields = 3
    };

    class bitmap_info_header {
    public:
        u32                AlphaMask {0};
        u16                BitCount {0};
        u32                BlueMask {0};
        u32                ClrImportant {0};
        u32                ClrUsed {0};
        compression        Compression {};
        u32                CsType {0};
        std::array<u32, 9> Endpoints {};
        u32                GammaBlue {0};
        u32                GammaGreen {0};
        u32                GammaRed {0};
        u32                GreenMask {0};
        u32                HeaderSize {0};
        i32                Height {0};
        i32                PelsPerMeterX {0};
        i32                PelsPerMeterY {0};
        u16                Planes {0};
        u32                RedMask {0};
        u32                SizeImage {0};
        i32                Width {0};

        void read(io::istream& reader);

        void read40(io::istream& reader);
    };
}

////////////////////////////////////////////////////////////

class bmp_decoder final : public image_decoder {
public:
    auto decode(io::istream& in) -> std::optional<image> override;
    auto decode_info(io::istream& in) -> std::optional<image::information> override;

private:
    auto get_palette(io::istream& in) const -> std::vector<color>;

    auto get_rgb_data(io::istream& in, size_i size, u16 bitCount, std::span<color const> palette) const -> std::vector<u8>;

    bmp::bitmap_file_header _header;
    bmp::bitmap_info_header _infoHeader;

    image::information _info {};
};

////////////////////////////////////////////////////////////

class bmp_encoder final : public image_encoder {
public:
    auto encode(image const& img, io::ostream& out) const -> bool override;
};

}
