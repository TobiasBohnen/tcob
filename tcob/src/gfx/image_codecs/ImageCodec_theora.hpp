// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_FILETYPES_GFX_THEORA)

    #include <theoraplay.h>

    #include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

class theora_decoder final : public animated_image_decoder {
public:
    theora_decoder();
    ~theora_decoder() override;

    auto current_frame() const -> u8 const* override;
    auto advance(milliseconds ts) -> animated_image_decoder::status override;
    void reset() override;

protected:
    auto open() -> std::optional<image::information> override;

private:
    size_i _size {size_i::Zero};
    i32    _currentTimeStamp {0};

    THEORAPLAY_Io                _io {};
    THEORAPLAY_Decoder*          _decoder {nullptr};
    THEORAPLAY_VideoFrame const* _currentFrame {nullptr};
};

}

#endif
