// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/audio/AudioSource.hpp"
#include "tcob/core/Interfaces.hpp"

#define AL_FORMAT_MONO_FLOAT32 0x10010
#define AL_FORMAT_STEREO_FLOAT32 0x10011

#include <AL/al.h>

////////////////////////////////////////////////////////////

namespace tcob::audio::al {

class TCOB_API al_buffer final : public non_copyable {
public:
    al_buffer();
    ~al_buffer();

    void buffer_data(std::span<i16 const> data, i32 channels, i32 freq) const;
    void buffer_data(std::span<f32 const> data, i32 channels, i32 freq) const;

    auto get_frequency() const -> i32;
    auto get_size() const -> i32;
    auto get_bits() const -> i32;
    auto get_channels() const -> i32;
    auto get_id() const -> u32;

    auto static GetSize(u32 bufferID) -> i32;

private:
    u32 _id {0};
};

////////////////////////////////////////////////////////////

class TCOB_API al_source final {
public:
    al_source();
    al_source(al_source const& other) noexcept;
    auto operator=(al_source const& other) noexcept -> al_source&;
    ~al_source();

    void play() const;
    void stop() const;
    void pause() const;

    void set_buffer(u32 bufferID) const;

    auto get_pitch() const -> f32;
    void set_pitch(f32 value) const;

    auto get_gain() const -> f32;
    void set_gain(f32 value) const;

    auto get_position() const -> std::array<f32, 3>;
    void set_position(std::array<f32, 3> const& value) const;

    auto get_direction() const -> std::array<f32, 3>;
    void set_direction(std::array<f32, 3> const& value) const;

    auto get_rolloff_factor() const -> f32;
    void set_rolloff_factor(f32 value) const;

    auto get_source_relative() const -> bool;
    void set_source_relative(bool value) const;

    auto get_sec_offset() const -> f32;
    void set_sec_offset(f32 value) const;

    auto is_looping() const -> bool;
    void set_looping(bool value) const;

    auto get_buffers_queued() const -> i32;
    auto get_buffers_processed() const -> i32;

    auto get_status() const -> source::status;

    void queue_buffers(u32 const* buffers, i32 bufferCount) const;
    auto unqueue_buffers(i32 bufferCount) const -> std::vector<u32>;

    auto get_id() const -> u32;

private:
    u32 _id {0};
};

}
