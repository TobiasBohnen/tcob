// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>
#include <vector>

#include "tcob/core/TypeFactory.hpp"

// TODO:
// master volume/pan in audio::system -> signals
// source position and listener -> volume scaling

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API specification final {
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

    virtual void bind()                   = 0;
    virtual void unbind()                 = 0;
    virtual auto is_bound() const -> bool = 0;

    virtual auto get_volume() const -> f32 = 0;
    virtual void set_volume(f32 val)       = 0;

    virtual void put(std::span<f32 const> data) = 0;
    virtual void flush()                        = 0;
    virtual void clear()                        = 0;

    virtual auto get() -> std::vector<f32> = 0;

    virtual auto available_bytes() const -> i32 = 0;
    virtual auto queued_bytes() const -> i32    = 0;
};

////////////////////////////////////////////////////////////

constexpr i32 RECORDING_SAMPLE_RATE {22050};

class TCOB_API system {
public:
    struct factory : public type_factory<std::shared_ptr<system>> {
        static inline char const* ServiceName {"audio::system::factory"};
    };

    virtual ~system() = default;

    static inline char const* ServiceName {"audio::system"};

    virtual auto create_output(specification const& info) const -> std::unique_ptr<audio_stream_base> = 0;
    virtual auto create_input() const -> std::unique_ptr<audio_stream_base>                           = 0;
};

////////////////////////////////////////////////////////////
// forward declarations

class buffer;
class decoder;
class source;
class effect_base;

}
