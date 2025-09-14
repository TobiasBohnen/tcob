// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>
#include <vector>

#include <SDL3/SDL.h>

#include "tcob/audio/Audio.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API sdl_audio_stream final : public audio_stream {
public:
    sdl_audio_stream(u32 device, specification const& info, bool input);
    ~sdl_audio_stream() override;

    void bind() override;
    void unbind() override;
    auto is_bound() const -> bool override;

    auto get_volume() const -> f32 override;
    void set_volume(f32 val) override;

    void put(std::span<f32 const> data) override;
    void flush() override;
    void clear() override;

    auto get() -> std::vector<f32> override;

    auto available_bytes() const -> i32 override;
    auto queued_bytes() const -> i32 override;

private:
    SDL_AudioStream* _impl {nullptr};
    u32              _device;
};

}
