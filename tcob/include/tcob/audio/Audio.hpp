// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>
#include <vector>

// TODO:
// master volume/pan in audio::system -> signals
// source position and listener -> volume scaling

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API specification {
public:
    i32 Channels {0};
    i32 SampleRate {0};

    explicit operator bool() const;
    auto     is_valid() const -> bool;

    auto operator==(specification const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API audio_stream_base {
public:
    virtual ~audio_stream_base() = default;

    void virtual bind()                   = 0;
    void virtual unbind()                 = 0;
    auto virtual is_bound() const -> bool = 0;

    auto virtual get_volume() const -> f32 = 0;
    void virtual set_volume(f32 val)       = 0;

    void virtual put(std::span<f32 const> data) = 0;
    void virtual flush()                        = 0;
    void virtual clear()                        = 0;

    auto virtual get() -> std::vector<f32> = 0;

    auto virtual available_bytes() const -> i32 = 0;
    auto virtual queued_bytes() const -> i32    = 0;
};

////////////////////////////////////////////////////////////

constexpr i32 RECORDING_SAMPLE_RATE {22050};

class TCOB_API system {
public:
    virtual ~system() = default;

    static inline char const* ServiceName {"audio_system"};

    auto virtual create_output(specification const& info) const -> std::unique_ptr<audio_stream_base> = 0;
    auto virtual create_input() const -> std::unique_ptr<audio_stream_base>                           = 0;
};

////////////////////////////////////////////////////////////
// forward declarations

class buffer;
class decoder;
class source;
class effect_base;

}
