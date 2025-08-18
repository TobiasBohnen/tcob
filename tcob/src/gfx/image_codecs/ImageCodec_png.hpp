// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ios>
#include <optional>
#include <span>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
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

    auto static consteval GetChunkType(char const* s) -> u32
    {
        return static_cast<u32>(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
    }

    enum class chunk_type : u32 {
        acTL = GetChunkType("acTL"), // used
        fcTL = GetChunkType("fcTL"), // used
        fdAT = GetChunkType("fdAT"), // used
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

        i32        Width {0};
        i32        Height {0};
        u8         BitDepth {0};
        color_type ColorType {};
        u8         CompressionMethod {0};
        u8         FilterMethod {0};
        u8         InterlaceMethod {0};
        bool       NonInterlaced {true};
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

        f32 Value {1.0f};
    };

    struct acTL_chunk {
        acTL_chunk(std::span<u8 const> data);

        u32 NumFrames {0};
        u32 NumPlays {0};
    };

    struct fcTL_chunk {
        fcTL_chunk(std::span<u8 const> data);

        u32        SequenceNumber {0};
        i32        Width {0};
        i32        Height {0};
        u32        XOffset {0};
        u32        YOffset {0};
        u16        DelayNum {0};
        u16        DelayDen {0};
        dispose_op DisposeOp {0};
        blend_op   BlendOp {0};

        milliseconds Duration {};
    };

}

////////////////////////////////////////////////////////////

class png_decoder : public image_decoder {
    using get_image_data = void (png_decoder::*)(i32, i32);

public:
    auto decode(io::istream& in) -> std::optional<image> override;
    auto decode_info(io::istream& in) -> std::optional<image::information> override;

protected:
    auto read_header(io::istream& in) -> bool;
    auto read_chunk(io::istream& in) const -> png::chunk;
    auto check_sig(io::istream& in) -> bool;

    auto read_image(std::span<byte const> idat, i32 width, i32 height) -> bool;

    auto ihdr() const -> png::IHDR_chunk const&;
    void handle_plte(png::chunk const& chunk);
    void handle_trns(png::chunk const& chunk);
    auto data() const -> std::vector<u8> const&;

private:
    void prepare(i32 width, i32 height);
    void prepare_delegate();

    void filter_pixel();
    void filter_line();

    void next_line_interlaced(i32 hei);
    void next_line_non_interlaced();

    auto get_interlace_dimensions(i32 width, i32 height) const -> rect_i;

    void interlaced_G1(i32 width, i32 height);
    void interlaced_G2(i32 width, i32 height);
    void interlaced_G4(i32 width, i32 height);
    void interlaced_G8_16(i32 width, i32 height);
    void interlaced_GA8_16(i32 width, i32 height);
    void interlaced_I1(i32 width, i32 height);
    void interlaced_I2(i32 width, i32 height);
    void interlaced_I4(i32 width, i32 height);
    void interlaced_I8(i32 width, i32 height);
    void interlaced_TC8_16(i32 width, i32 height);
    void interlaced_TCA8_16(i32 width, i32 height);

    void non_interlaced_G1(i32 width, i32 height);
    void non_interlaced_G2(i32 width, i32 height);
    void non_interlaced_G4(i32 width, i32 height);
    void non_interlaced_G8_16(i32 width, i32 height);
    void non_interlaced_GA8_16(i32 width, i32 height);
    void non_interlaced_I1(i32 width, i32 height);
    void non_interlaced_I2(i32 width, i32 height);
    void non_interlaced_I4(i32 width, i32 height);
    void non_interlaced_I8(i32 width, i32 height);
    void non_interlaced_TC8_16(i32 width, i32 height);
    void non_interlaced_TCA8_16(i32 width, i32 height);

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

class png_encoder : public image_encoder {
public:
    auto encode(image const& image, io::ostream& out) const -> bool override;

    void write_ihdr(image::information const& info, io::ostream& out) const;
    void write_idat(image const& image, io::ostream& out) const;
    void write_iend(io::ostream& out) const;

    void write_chunk(io::ostream& out, std::span<u8 const> buf) const;
    void write_chunk(io::ostream& out, std::span<u8 const> buf, u32 length) const;
};

////////////////////////////////////////////////////////////

class png_anim_decoder final : public png_decoder, public animated_image_decoder {
public:
    auto open() -> std::optional<image::information> override;
    auto current_frame() const -> std::span<u8 const> override;
    auto advance(milliseconds ts) -> animated_image_decoder::status override;
    void reset() override;

private:
    auto get_next_frame(io::istream& in) -> animated_image_decoder::status;

    std::optional<png::fcTL_chunk> _previousFctl {};
    std::optional<image>           _previousFrame;
    image                          _currentFrame;
    milliseconds                   _currentTimeStamp {0};
    std::streamsize                _contentOffset {0};
};

////////////////////////////////////////////////////////////

class png_anim_encoder final : public animated_image_encoder {
public:
    auto encode(std::span<image_frame const> frames, io::ostream& out) -> bool override;

private:
    void write_actl(std::span<image_frame const> frames, io::ostream& out) const;
    void write_fctl(u32 idx, rect_i const& rect, image_frame const& frame, io::ostream& out) const;
    void write_fdat(u32 idx, image const& frame, io::ostream& out) const;

    png_encoder _enc;
};

}
