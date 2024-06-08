// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <optional>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

namespace gif {
    constexpr i32 BPP {4};

    auto read_color_table(int ncolors, istream& reader) -> std::vector<color>;

    struct header {
        i32                BackgroundIndex;
        std::vector<color> GlobalColorTable;
        bool               GlobalColorTableFlag;
        i32                GlobalColorTableSize;
        u32                Height;
        string             Id;
        i32                PixelAspect;
        u32                Width;

        void read(istream& reader);
    };
}

////////////////////////////////////////////////////////////

class gif_decoder_base {
protected:
    auto read_contents(istream& reader, gif::header const& header) -> animated_image_decoder::status;

    auto decode_frame_data(istream& reader, u16 iw, u16 ih) -> std::vector<u8>;
    auto read_block(istream& reader) -> i32;
    void read_graphic_control_ext(istream& reader);
    void read_frame(istream& reader, gif::header const& header);
    void skip(istream& reader);

    void clear_pixel_cache();
    void seek_to_first_frame(istream& reader) const;

    image        _currentFrame;
    milliseconds _currentTimeStamp {0};

private:
    std::array<u8, 256> _block {};      // current data block
    i32                 _blockSize {0}; // block size

    i32             _dispose {0};
    std::vector<u8> _pixelCache {};
    i32             _transIndex {0};       // transparent color index
    bool            _transparency {false}; // use transparent color

    std::streamsize _firstFrameOffset {0};
    bool            _firstFrame {true};
};

////////////////////////////////////////////////////////////

class gif_decoder final : public gif_decoder_base, public image_decoder {
public:
    auto decode(istream& in) -> std::optional<image> override;
    auto decode_header(istream& in) -> std::optional<image::info> override;

private:
    gif::header _header {};
};

////////////////////////////////////////////////////////////

class gif_anim_decoder final : public gif_decoder_base, public animated_image_decoder {
public:
    auto get_current_frame() const -> u8 const* override;
    auto seek_from_current(milliseconds ts) -> animated_image_decoder::status override;
    void reset() override;

protected:
    auto open() -> std::optional<image::info> override;

private:
    gif::header _header {};
};

}
