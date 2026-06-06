// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <ios>
#include <optional>
#include <span>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

namespace gif {
    constexpr i32 BPP {4};

    auto read_color_table(int ncolors, io::istream& reader) -> std::vector<color>;

    struct header {
        i32                BackgroundIndex;
        std::vector<color> GlobalColorTable;
        bool               GlobalColorTableFlag;
        i32                GlobalColorTableSize;
        u16                Width;
        u16                Height;
        string             Id;
        i32                PixelAspect;

        auto read [[nodiscard]] (io::istream& reader) -> bool;
    };
}

////////////////////////////////////////////////////////////

class gif_decoder final : public image_decoder, public animated_image_decoder {
public:
    // image_decoder
    auto decode(io::istream& in) -> std::optional<image> override;
    auto decode_info(io::istream& in) -> std::optional<image::information> override;

    // animated_image_decoder
    auto open() -> std::optional<image::information> override;
    auto current_frame() const -> std::span<u8 const> override;
    auto advance_to(milliseconds ts) -> animated_image_decoder::status override;
    void reset() override;

protected:
    auto read_contents(io::istream& reader) -> animated_image_decoder::status;

    auto decode_frame_data(io::istream& reader, u16 iw, u16 ih) -> std::vector<u8>;
    auto read_block(io::istream& reader) -> i32;
    void read_graphic_control_ext(io::istream& reader);
    void read_frame(io::istream& reader);

    void clear_pixel_cache();

private:
    std::array<u8, 256> _block {}; // current data block
    u8                  _blockSize {0};

    i32             _dispose {0};
    std::vector<u8> _pixelCache {};
    i32             _transIndex {0};          // transparent color index
    bool            _hasTransparency {false}; // use transparent color

    std::streamoff _contentOffset {0};
    bool           _firstFrame {true};

    image        _currentFrame;
    milliseconds _currentTimeStamp {0};

    gif::header _header {};
};

////////////////////////////////////////////////////////////

class gif_encoder final : public image_encoder, public animated_image_encoder {
public:
    // image_encoder
    auto encode(image const& image, io::ostream& out) -> bool override;

    // animated_image_encoder
    void start() override;
    auto add_frame(image_frame const& frame) -> bool override;
    auto finish() -> bool override;

private:
    void start(io::ostream& out);
    auto add_frame(image_frame const& frame, io::ostream& out) -> bool;
    auto finish(io::ostream& out) -> bool;

    void write_graphic_ctrl_ext(io::ostream& out) const;
    void write_image_desc(io::ostream& out) const;
    void write_lsd(io::ostream& out) const;
    void write_palette(io::ostream& out, std::span<color const> pal) const;
    void write_netscape_loop(io::ostream& out);
    void write_pixels(io::ostream& out, std::vector<u32> const& buffer) const;

    i16  _delay {0};
    i32  _dispose {-1};
    bool _firstFrame {true};

    u8    _transIndex {0};
    color _transparent {colors::Transparent};

    bool _sizeSet {false};
    i16  _height {0};
    i16  _width {0};
};

}
