// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/AudioSystem.hpp"

#include <cassert>
#include <memory>

#include "Output.hpp"

#include <SDL3/SDL.h>

#include "tcob/audio/Buffer.hpp"

namespace tcob::audio {

system::system()
    : _device {SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)}
{
    SDL_ResumeAudioDevice(_device);
}

system::~system()
{
    SDL_CloseAudioDevice(_device);
}

auto system::create_output(buffer::information const& info) const -> std::unique_ptr<detail::output>
{
    return std::make_unique<detail::output>(_device, info);
}

}
