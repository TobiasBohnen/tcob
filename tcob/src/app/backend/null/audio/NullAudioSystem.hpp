// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>
#include <vector>

#include "tcob/audio/Audio.hpp"
#include "tcob/core/Interfaces.hpp"

namespace tcob::audio::null {
////////////////////////////////////////////////////////////

class TCOB_API null_audio_stream final : public audio_stream_base {
public:
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
};

////////////////////////////////////////////////////////////

class TCOB_API null_audio_system final : public system, public non_copyable {
public:
    auto create_output(specification const& info) const -> std::unique_ptr<audio_stream_base> override;
    auto create_input() const -> std::unique_ptr<audio_stream_base> override;
};

}
