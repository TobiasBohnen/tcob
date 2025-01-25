// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SoundFont.hpp"

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)

namespace tcob::audio {

template <std::derived_from<sound_font_command> T>
inline void sound_font_commands::add(auto&&... args)
{
    _commands.back().second.push_back(std::make_unique<T>(std::move(args)...));
}

}

#endif
