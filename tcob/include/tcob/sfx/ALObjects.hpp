// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <vector>

#define AL_FORMAT_MONO_FLOAT32 0x10010
#define AL_FORMAT_STEREO_FLOAT32 0x10011

namespace tcob {
enum class AudioState {
    Initial,
    Stopped,
    Paused,
    Playing
};

struct AudioInfo {
    i32 Channels { 0 };
    i32 Frequency { 0 };
    u64 SampleCount { 0 };
};
}

namespace tcob::al {

class Buffer final {
public:
    Buffer();
    ~Buffer();

    Buffer(const Buffer& other) = delete;
    auto operator=(const Buffer& other) -> Buffer& = delete;

    void buffer_data(const i16* data, u64 frameCount, i32 channels, i32 freq) const;
    void buffer_data(const f32* data, u64 frameCount, i32 channels, i32 freq) const;

    auto frequency() const -> i32;

    auto size() const -> i32;

    auto bits() const -> i32;

    auto channels() const -> i32;

    u32 ID { 0 };
};

////////////////////////////////////////////////////////////

class Source final {
public:
    Source();
    ~Source();

    Source(const Source& other);
    auto operator=(const Source& other) -> Source&;

    void play();
    void stop();
    void pause();

    void buffer(u32 bufferID);

    auto pitch() const -> f32;
    void pitch(f32 value) const;

    auto gain() const -> f32;
    void gain(f32 value) const;

    auto position() const -> std::array<f32, 3>;
    void position(const std::array<f32, 3>& value) const;

    auto direction() const -> std::array<f32, 3>;
    void direction(const std::array<f32, 3>& value) const;

    auto rolloff_factor() const -> f32;
    void rolloff_factor(f32 value) const;

    auto source_relatvie() const -> bool;
    void source_relatvie(bool value) const;

    auto sec_offset() const -> f32;
    void sec_offset(f32 value) const;

    auto looping() const -> bool;
    void looping(bool value) const;

    auto buffers_queued() const -> i32;
    auto buffers_processed() const -> i32;

    auto state() const -> AudioState;

    void queue_buffers(const u32* buffers, i32 bufferCount);
    auto unqueue_buffers(i32 bufferCount) -> std::vector<u32>;

    u32 ID { 0 };
};

}