// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioStream.hpp"

#include <cassert>
#include <span>
#include <vector>

#include <SDL3/SDL.h>

#include "tcob/audio/Audio.hpp"

namespace tcob::audio::detail {

audio_stream::audio_stream(u32 device, specification const& info)
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

audio_stream::~audio_stream()
{
    SDL_DestroyAudioStream(_impl);
}

void audio_stream::bind()
{
    [[maybe_unused]] bool const err {SDL_BindAudioStream(_device, _impl)};
    assert(err);
}

void audio_stream::unbind()
{
    SDL_UnbindAudioStream(_impl);
}

auto audio_stream::is_bound() const -> bool
{
    return SDL_GetAudioStreamDevice(_impl) != 0;
}

auto audio_stream::get_volume() const -> f32
{
    return SDL_GetAudioStreamGain(_impl);
}

void audio_stream::set_volume(f32 val)
{
    SDL_SetAudioStreamGain(_impl, val);
}

void audio_stream::put(std::span<f32 const> data)
{
    [[maybe_unused]] bool const err {SDL_PutAudioStreamData(_impl, data.data(), static_cast<i32>(data.size_bytes()))};
    assert(err);
}

void audio_stream::flush()
{
    SDL_FlushAudioStream(_impl);
}

void audio_stream::clear()
{
    SDL_ClearAudioStream(_impl);
}

auto audio_stream::get() -> std::vector<f32>
{
    std::vector<f32> data(available_bytes());
    SDL_GetAudioStreamData(_impl, data.data(), static_cast<i32>(data.size()));
    return data;
}

auto audio_stream::available_bytes() const -> i32
{
    return SDL_GetAudioStreamAvailable(_impl);
}

auto audio_stream::queued_bytes() const -> i32
{
    return SDL_GetAudioStreamQueued(_impl);
}

}
