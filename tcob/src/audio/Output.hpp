// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#include "tcob/audio/Buffer.hpp"

struct SDL_AudioStream;

namespace tcob::audio::detail {
////////////////////////////////////////////////////////////

class TCOB_API output final {
public:
    output(u32 device, buffer::information const& info);
    ~output();

    void bind();
    void unbind();
    auto is_bound() const -> bool;

    auto get_volume() const -> f32;
    void set_volume(f32 val);

    void put(std::span<f32 const> data);
    void flush();
    void clear();

    auto available_bytes() const -> i32;
    auto queued_bytes() const -> i32;

private:
    SDL_AudioStream* _impl {nullptr};
    u32              _device;
};

}
