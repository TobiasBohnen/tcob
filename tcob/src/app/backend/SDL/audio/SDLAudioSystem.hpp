// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include <SDL3/SDL.h>

#include "tcob/audio/Audio.hpp"
#include "tcob/core/Interfaces.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API sdl_audio_system final : public system, public non_copyable {
public:
    sdl_audio_system();
    ~sdl_audio_system() override;

    auto create_output(specification const& info) const -> std::unique_ptr<audio_stream> override;
    auto create_input() const -> std::unique_ptr<audio_stream> override;

private:
    u32 _devicePlayback;
    u32 _deviceRecording;
};

}
