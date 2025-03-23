// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <memory>
#include <optional>
#include <span>

#include "tcob/audio/Audio.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API source : public non_copyable {
public:
    source();
    virtual ~source();

    std::any DecoderContext;

    prop<f32> Volume;

    auto virtual info() const -> std::optional<specification> = 0;
    auto virtual duration() const -> milliseconds             = 0;
    auto status() const -> playback_status;

    void play();
    void stop();
    void restart();

    void pause();
    void resume();
    void toggle_pause();

protected:
    auto virtual on_start() -> bool = 0;
    auto virtual on_stop() -> bool  = 0;

    void create_output(specification const& info);
    void write_to_output(std::span<f32 const> data);
    void flush_output();
    auto queued_bytes() const -> i32;

private:
    std::unique_ptr<detail::audio_stream> _output;
};

}
