// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

namespace tga {
    enum class rle_packet_type {
        Raw        = 0,
        Compressed = 1
    };

    enum class attribute_type : u8 {
        NoAlpha            = 0,
        UndefinedIgnore    = 1,
        UndefinedRetain    = 2,
        Alpha              = 3,
        PreMultipliedAlpha = 4
    };

    enum class color_map_type : u8 {
        NoColorMapIncluded = 0,
        ColorMapIncluded   = 1
    };

    enum class first_pixel_destination {
        BottomLeft  = 0,
        BottomRight = 1,
        TopLeft     = 2,
        TopRight    = 3
    };

    enum class format {
        Original,
        New
    };

    enum class image_type : u8 {
        NoImageDataIncluded            = 0,
        UncompressedColorMappedImage   = 1,
        UncompressedTrueColorImage     = 2,
        UncompressedBlackAndWhiteImage = 3,
        RLEColorMappedImage            = 9,
        RLETrueColorImage              = 10,
        RLEBlackAndWhiteImage          = 11
    };

    struct footer {
        static inline i32 const Offset {26};
        static inline i32 const SignatureOffset {8};

        u32    DeveloperDirectoryOffset;
        u32    ExtensionAreaOffset;
        format Format;

        void read(istream& reader);
    };

    struct image_descriptor {
        i32                     AttributeBits;
        first_pixel_destination FirstPixelDestination;

        void read(istream& reader);
    };

    struct image_specifications {
        i32              BytesPerPixel;
        u16              Height;
        image_descriptor ImageDescriptor;
        u8               PixelDepth;
        u16              Width;
        u16              XOrigin;
        u16              YOrigin;

        void read(istream& reader);
    };

    struct color_map_specifications {
        u8  ColorMapEntrySize;
        u16 ColorMapLength;
        i32 ColorMapTotalSize;
        u16 FirstEntryIndex;

        void read(istream& reader);
    };

    struct header {
        color_map_specifications ColorMapSpecs;
        bool                     ColorMapIncluded;
        u8                       IDLength;
        image_specifications     ImageSpecs;
        image_type               ImageType;

        void read(istream& reader);
    };

    auto read_data(istream& reader, header const& head) -> std::vector<u8>;
    auto read_color_map(istream& reader, i32 colorMapLength, i32 colorMapEntrySize) -> std::vector<color>;
}

////////////////////////////////////////////////////////////

class tga_decoder final : public image_decoder {
public:
    auto decode(istream& in) -> std::optional<image> override;
    auto decode_header(istream& in) -> std::optional<image::info> override;

private:
    tga::footer _footer {};
    tga::header _header {};
};

////////////////////////////////////////////////////////////

class tga_encoder final : public image_encoder {
public:
    auto encode(image const& img, ostream& out) const -> bool override;

private:
    void write_header(image::info const& image, ostream& out) const;
    void write_image_data(image const& img, ostream& out) const;
    void write_extension_area(ostream& out) const;
    void write_footer(ostream& out, std::streamsize extOffset) const;
};

}
