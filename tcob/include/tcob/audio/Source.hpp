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
    prop<f32> Panning; // TODO: prop_val

    virtual auto info() const -> std::optional<specification> = 0;
    virtual auto duration() const -> milliseconds             = 0;
    auto         state() const -> playback_state;

    auto play() -> bool;
    auto stop() -> bool;
    auto restart() -> bool;

    void pause();
    void resume();
    void toggle_pause();

protected:
    virtual auto on_start() -> bool = 0;
    virtual auto on_stop() -> bool  = 0;

    void create_output();
    void write_to_output(std::span<f32 const> data);
    void flush_output();
    void stop_output();
    auto queued_bytes() const -> i32;

private:
    std::unique_ptr<audio_stream> _output;
    bool                          _canPan {false};
};

}
