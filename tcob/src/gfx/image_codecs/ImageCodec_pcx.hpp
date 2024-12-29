// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

namespace pcx {
    constexpr i32 BPP {3};

    enum class encoding : u8 {
        None = 0,
        Rle  = 1
    };

    enum class palette_type : u8 {
        Color     = 1,
        Grayscale = 2
    };

    enum class version : u8 {
        V2_5             = 0,
        V2_8_Palette     = 2,
        V2_8_NoPalette   = 3,
        WindowsNoPalette = 4,
        V3_0             = 5
    };

    struct header {
        u8           BitsPerPixel;
        i16          BytesPerLine;
        u8           ColorPlanesCount;
        encoding     Encoding;
        i16          HortDPI;
        i16          HScrSize;
        u8           Manufacturer;
        palette_type PaletteType;
        u8           Reserved;
        version      Version;
        u16          VertDPI;
        u16          VScrSize;
        u16          XMax;
        u16          XMin;
        u16          YMax;
        u16          YMin;

        auto Height() const -> i32
        {
            return static_cast<i32>(YMax - YMin + 1);
        }
        auto Width() const -> i32
        {
            return static_cast<i32>(XMax - XMin + 1);
        }

        void read(io::istream& reader);
        void static Write(image::information const& info, io::ostream& writer);
    };

    auto read_image_data(io::istream& reader, header const& h) -> std::vector<u8>;
    auto read_color_palette(io::istream& reader, i32 size) -> std::vector<color>;
}

////////////////////////////////////////////////////////////

class pcx_decoder final : public image_decoder {
public:
    auto decode(io::istream& in) -> std::optional<image> override;
    auto decode_info(io::istream& in) -> std::optional<image::information> override;

private:
    auto read_palette(io::istream& in) const -> std::vector<color>;

    pcx::header _header;
};

////////////////////////////////////////////////////////////

class pcx_encoder final : public image_encoder {
public:
    auto encode(image const& img, io::ostream& out) const -> bool override;
};

}
