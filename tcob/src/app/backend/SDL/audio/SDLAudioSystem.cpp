// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "SDLAudioSystem.hpp"

#include <cassert>
#include <memory>

#include "SDLAudioStream.hpp"
#include "tcob/audio/Audio.hpp"

namespace tcob::audio {

sdl_audio_system::sdl_audio_system()
    : _devicePlayback {SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)}
    , _deviceRecording {SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, nullptr)}
{
}

sdl_audio_system::~sdl_audio_system()
{
    SDL_CloseAudioDevice(_devicePlayback);
    SDL_CloseAudioDevice(_deviceRecording);
}

auto sdl_audio_system::create_output(specification const& info) const -> std::unique_ptr<audio_stream_base>
{
    return std::make_unique<sdl_audio_stream>(_devicePlayback, info, false);
}

auto sdl_audio_system::create_input() const -> std::unique_ptr<audio_stream_base>
{
    return std::make_unique<sdl_audio_stream>(_deviceRecording, specification {.Channels = 1, .SampleRate = RECORDING_SAMPLE_RATE}, true);
}

}
