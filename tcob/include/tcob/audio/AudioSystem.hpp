// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>

#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Interfaces.hpp"

struct SDL_AudioStream;

namespace tcob::audio {
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

////////////////////////////////////////////////////////////

class TCOB_API system final : public non_copyable {
public:
    system();
    ~system();

    static inline char const* service_name {"audio_system"};

    auto create_output(buffer::information const& info) const -> std::unique_ptr<output>;

private:
    u32 _device;
};

}
