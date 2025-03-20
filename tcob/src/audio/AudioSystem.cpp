// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/AudioSystem.hpp"
#include "tcob/audio/Buffer.hpp"

#include <cassert>
#include <memory>
#include <span>

#include <SDL3/SDL.h>

namespace tcob::audio {

output::output(u32 device, buffer::information const& info)
    : _device {device}
{
    SDL_AudioSpec srcSpec;
    srcSpec.channels = info.Channels;
    srcSpec.freq     = info.SampleRate;
    srcSpec.format   = SDL_AUDIO_F32;

    SDL_AudioSpec               dstSpec;
    [[maybe_unused]] bool const err {SDL_GetAudioDeviceFormat(_device, &dstSpec, nullptr)};
    assert(err);

    _impl = SDL_CreateAudioStream(&srcSpec, &dstSpec);
    assert(_impl);
}

output::~output()
{
    SDL_DestroyAudioStream(_impl);
}

void output::bind()
{
    [[maybe_unused]] bool const err {SDL_BindAudioStream(_device, _impl)};
    assert(err);
}

void output::unbind()
{
    SDL_UnbindAudioStream(_impl);
}

auto output::is_bound() const -> bool
{
    return SDL_GetAudioStreamDevice(_impl) != 0;
}

auto output::get_volume() const -> f32
{
    return SDL_GetAudioStreamGain(_impl);
}

void output::set_volume(f32 val)
{
    SDL_SetAudioStreamGain(_impl, val);
}

void output::put(std::span<f32 const> data)
{
    [[maybe_unused]] bool const err {SDL_PutAudioStreamData(_impl, data.data(), data.size_bytes())};
    assert(err);
}

void output::flush()
{
    SDL_FlushAudioStream(_impl);
}

void output::clear()
{
    SDL_ClearAudioStream(_impl);
}

auto output::available_bytes() const -> i32
{
    return SDL_GetAudioStreamAvailable(_impl);
}

auto output::queued_bytes() const -> i32
{
    return SDL_GetAudioStreamQueued(_impl);
}

////////////////////////////////////////////////////////////

system::system()
    : _device {SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)}
{
    SDL_ResumeAudioDevice(_device);
}

system::~system()
{
    SDL_CloseAudioDevice(_device);
}

auto system::create_output(buffer::information const& info) const -> std::unique_ptr<output>
{
    return std::make_unique<output>(_device, info);
}

}
