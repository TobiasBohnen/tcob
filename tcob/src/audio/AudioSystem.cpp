// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/AudioSystem.hpp"

#include <cassert>
#include <memory>

#include <SDL3/SDL.h>

#include "AudioStream.hpp"

#include "tcob/audio/Audio.hpp"

namespace tcob::audio {

system::system()
    : _devicePlayback {SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)}
    , _deviceRecording {SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, nullptr)}
{
}

system::~system()
{
    SDL_CloseAudioDevice(_devicePlayback);
    SDL_CloseAudioDevice(_deviceRecording);
}

auto system::create_output(specification const& info) const -> std::unique_ptr<detail::audio_stream>
{
    return std::make_unique<detail::audio_stream>(_devicePlayback, info);
}

auto system::create_input() const -> std::unique_ptr<detail::audio_stream>
{
    return std::make_unique<detail::audio_stream>(_deviceRecording, specification {.Channels = 1, .SampleRate = RECORDING_SAMPLE_RATE});
}

}
