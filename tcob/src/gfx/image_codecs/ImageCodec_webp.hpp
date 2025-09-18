// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)

    #include <optional>
    #include <span>
    #include <vector>

    #include <webp/decode.h>
    #include <webp/demux.h>
    #include <webp/encode.h>
    #include <webp/mux.h>
    #include <webp/mux_types.h>

    #include "tcob/core/Size.hpp"
    #include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

class webp_decoder final : public image_decoder {
public:
    auto decode(io::istream& in) -> std::optional<image> override;
    auto decode_info(io::istream& in) -> std::optional<image::information> override;

private:
    std::vector<u8> _buffer;
};

////////////////////////////////////////////////////////////

class webp_encoder final : public image_encoder {
public:
    auto encode(image const& image, io::ostream& out) const -> bool override;
};

////////////////////////////////////////////////////////////

class webp_anim_decoder final : public animated_image_decoder {
public:
    webp_anim_decoder();
    ~webp_anim_decoder() override;

    auto current_frame() const -> std::span<u8 const> override;
    auto advance(milliseconds ts) -> animated_image_decoder::status override;
    void reset() override;

protected:
    auto open() -> std::optional<image::information> override;

private:
    size_i _size {size_i::Zero};
    i32    _currentTimeStamp {0};

    WebPAnimDecoder* _decoder {nullptr};
    WebPData*        _data {nullptr};
    u8*              _buffer {nullptr};
};

////////////////////////////////////////////////////////////

class webp_anim_encoder final : public animated_image_encoder {
public:
    webp_anim_encoder() = default;
    ~webp_anim_encoder() override;

    auto encode(std::span<image_frame const> frames, io::ostream& out) -> bool override;

private:
    WebPAnimEncoder* _encoder {nullptr};
    size_i           _imgSize {size_i::Zero};
};
}

#endif
