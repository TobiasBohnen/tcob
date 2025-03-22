// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Audio.hpp"

namespace tcob::audio {

auto specification::is_valid() const -> bool
{
    return Channels > 0 && SampleRate > 0;
}

} // namespace audio
