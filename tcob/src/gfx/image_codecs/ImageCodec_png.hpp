// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <span>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

namespace png {
    constexpr i32 BPP {4};
    constexpr i32 MAX_SIZE {0x4000};

    enum class blend_op : u8 {
        Source = 0,
        Over   = 1,
    };

    enum class color_type : u8 {
        Grayscale      = 0,
        TrueColor      = 2,
        Indexed        = 3,
        GrayscaleAlpha = 4,
        TrueColorAlpha = 6,
    };
    ;

    enum class dispose_op : u8 {
        None       = 0,
        Background = 1,
        Previous   = 2
    };

    auto static constexpr GetChunkType(string_view s) -> u32
    {
        return static_cast<u32>(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
    }

    enum class chunk_type : u32 {
        acTL = GetChunkType("acTL"),
        fcTL = GetChunkType("fcTL"),
        fdAT = GetChunkType("fdAT"),
        gAMA = GetChunkType("gAMA"),
        IDAT = GetChunkType("IDAT"), // used
        IEND = GetChunkType("IEND"), // used
        IHDR = GetChunkType("IHDR"), // used
        iTXt = GetChunkType("iTXt"),
        pHYs = GetChunkType("pHYs"), // used
        PLTE = GetChunkType("PLTE"), // used
        tEXt = GetChunkType("tEXt"),
        tRNS = GetChunkType("tRNS"), // used
        zTXt = GetChunkType("zTXt")
    };

    struct chunk {
        u32             Length {0};
        chunk_type      Type {0};
        u32             Crc {0};
        std::vector<u8> Data;
    };

    struct IHDR_chunk {
        IHDR_chunk() = default;
        IHDR_chunk(std::span<u8 const> data);

        u8         BitDepth {0};
        color_type ColorType {};
        u8         CompressionMethod {0};
        u8         FilterMethod {0};
        i32        Height {0};
        u8         InterlaceMethod {0};
        bool       NonInterlaced {true};
        i32        Width {0};
    };

    struct PLTE_chunk {
        PLTE_chunk(std::span<u8 const> data);

        std::vector<color> Entries;
    };

    struct tRNS_chunk {
        tRNS_chunk(std::span<u8 const> data, color_type colorType, std::optional<PLTE_chunk>& plte);

        auto is_gray_transparent(u8 val) -> bool;

        auto is_rgb_transparent(u8 r, u8 g, u8 b) -> bool;

        std::vector<u8> Indicies;
    };

    struct pHYs_chunk {
        pHYs_chunk(std::span<u8 const> data);

        f32 Value {1.f};
    };

}

////////////////////////////////////////////////////////////

class png_decoder final : public image_decoder {
    using get_image_data = void (png_decoder::*)();

public:
    auto decode(istream& in) -> std::optional<image> override;
    auto decode_info(istream& in) -> std::optional<image::info> override;

private:
    auto read_header(istream& in) -> bool;
    auto read_chunk(istream& in) const -> png::chunk;
    auto check_sig(istream& in) -> bool;

    void prepare();
    void prepare_delegate();
    auto read_image(std::span<ubyte const> idat) -> bool;

    void filter_pixel();
    void filter_line();

    void next_line_interlaced(i32 hei);
    void next_line_non_interlaced();

    auto get_interlace_dimensions() const -> rect_i;

    void interlaced_G1();
    void interlaced_G2();
    void interlaced_G4();
    void interlaced_G8_16();
    void interlaced_GA8_16();
    void interlaced_I1();
    void interlaced_I2();
    void interlaced_I4();
    void interlaced_I8();
    void interlaced_TC8_16();
    void interlaced_TCA8_16();

    void non_interlaced_G1();
    void non_interlaced_G2();
    void non_interlaced_G4();
    void non_interlaced_G8_16();
    void non_interlaced_GA8_16();
    void non_interlaced_I1();
    void non_interlaced_I2();
    void non_interlaced_I4();
    void non_interlaced_I8();
    void non_interlaced_TC8_16();
    void non_interlaced_TCA8_16();

    png::IHDR_chunk                _ihdr;
    std::optional<png::PLTE_chunk> _plte;
    std::optional<png::tRNS_chunk> _trns;

    point_i _pixel {-1, 0};
    u8      _filter {0};
    u8      _pixelSize {0};
    u32     _interlacePass {1};

    std::vector<u8> _prvLine;

    std::vector<u8>           _curLine;
    std::vector<u8>::iterator _curLineIt;

    std::vector<u8>           _data;
    std::vector<u8>::iterator _dataIt;

    get_image_data _getImageData;
};

////////////////////////////////////////////////////////////

class png_encoder final : public image_encoder {
public:
    auto encode(image const& image, ostream& out) const -> bool override;

private:
    void write_header(image const& image, ostream& out) const;
    void write_image(image const& image, ostream& out) const;
    void write_end(ostream& out) const;

    void write_chunk(ostream& out, std::span<u8 const> buf) const;
    void write_chunk(ostream& out, std::span<u8 const> buf, u32 length) const;
};

}
