// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <span>

#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/TypeFactory.hpp"

////////////////////////////////////////////////////////////

namespace tcob::audio {
////////////////////////////////////////////////////////////

namespace al {
    class al_buffer;
    class al_source;
}

////////////////////////////////////////////////////////////

class TCOB_API source {
public:
    source();
    source(source const& other) noexcept;
    auto operator=(source const& other) noexcept -> source&;
    virtual ~source();

    std::any DecoderContext;

    prop_fn<f32> Volume;

    auto virtual get_duration() const -> milliseconds          = 0;
    auto virtual get_playback_position() const -> milliseconds = 0;
    auto get_status() const -> playback_status;
    auto is_looping() const -> bool;

    void play(bool looping = false);
    void stop();
    void restart();

    void pause();
    void resume();
    void toggle_pause();

protected:
    auto virtual on_start() -> bool = 0;
    auto virtual on_stop() -> bool  = 0;

    auto get_source() const -> audio::al::al_source*;

private:
    std::shared_ptr<audio::al::al_source> _source;
};

////////////////////////////////////////////////////////////

class TCOB_API decoder {
public:
    struct factory : public type_factory<std::unique_ptr<decoder>> {
        static inline char const* service_name {"audio::decoder::factory"};
    };

    decoder();
    decoder(decoder const& other) noexcept                    = delete;
    auto operator=(decoder const& other) noexcept -> decoder& = delete;
    virtual ~decoder();

    auto open(std::shared_ptr<istream> in, std::any& ctx) -> std::optional<buffer::info>;

    auto decode_to_buffer(al::al_buffer* buffer, i64 wantSamples) -> bool;
    auto decode_to_buffer(std::span<f32> buffer) -> bool;

    void virtual seek_from_start(milliseconds pos) = 0;

protected:
    auto virtual open() -> std::optional<buffer::info>       = 0;
    auto virtual decode(std::span<f32> outputSamples) -> i32 = 0;

    auto get_stream() -> istream&;
    auto get_context() -> std::any&;

private:
    std::shared_ptr<istream>    _stream;
    std::any                    _ctx;
    std::optional<buffer::info> _info;
};

////////////////////////////////////////////////////////////

class TCOB_API encoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<encoder>> {
        static inline char const* service_name {"audio::encoder::factory"};
    };

    encoder()          = default;
    virtual ~encoder() = default;

    auto virtual encode(std::span<f32 const> samples, buffer::info const& info, ostream& out) const -> bool = 0;
};

}
