// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_LIBXMP)

    #include <optional>

    #include <xmp.h>

    #include "tcob/audio/Buffer.hpp"

namespace tcob::audio::detail {
////////////////////////////////////////////////////////////

class xmp_decoder final : public decoder {
public:
    xmp_decoder();
    ~xmp_decoder() override;

    void seek_from_start(milliseconds pos) override;

protected:
    auto open() -> std::optional<buffer::info> override;
    auto decode(std::span<f32> outputSamples) -> i32 override;

private:
    buffer::info _info {};
    xmp_context  _context;
};

}

#endif
