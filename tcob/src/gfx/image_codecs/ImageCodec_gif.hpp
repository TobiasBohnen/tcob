// Copyright (c) 2025 Tobias Bohnen
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

        void read(io::istream& reader);
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
    auto advance(milliseconds ts) -> animated_image_decoder::status override;
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
    usize               _blockSize {0};

    i32             _dispose {0};
    std::vector<u8> _pixelCache {};
    i32             _transIndex {0};       // transparent color index
    bool            _transparency {false}; // use transparent color

    std::streamsize _contentOffset {0};
    bool            _firstFrame {true};

    image        _currentFrame;
    milliseconds _currentTimeStamp {0};

    gif::header _header {};
};

}
