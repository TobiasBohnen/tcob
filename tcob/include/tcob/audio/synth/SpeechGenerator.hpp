// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_AUDIO_SPEECH)

    #include "tcob/audio/Buffer.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API speech_generator final {
public:
    speech_generator() = default;

    auto create_buffer [[nodiscard]] (string const& text) -> buffer;

private:
};
}

#endif
