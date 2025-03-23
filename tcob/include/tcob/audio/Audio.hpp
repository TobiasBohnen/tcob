// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

// TODO:
// master volume/pan in audio::system -> signals
// source position and listener -> volume scaling

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API specification {
public:
    i32 Channels {0};
    i32 SampleRate {0};

    auto     is_valid() const -> bool;
    explicit operator bool() const;

    auto operator==(specification const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////
// forward declarations

class buffer;
class decoder;
class source;
class effect_base;

namespace detail {
    class audio_stream;
}

}
