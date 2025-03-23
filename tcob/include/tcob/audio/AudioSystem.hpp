// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/audio/Audio.hpp"
#include "tcob/core/Interfaces.hpp"

struct SDL_AudioStream;

namespace tcob::audio {
////////////////////////////////////////////////////////////

constexpr i32 RECORDING_SAMPLE_RATE {22050};

class TCOB_API system final : public non_copyable {
public:
    system();
    ~system();

    static inline char const* service_name {"audio_system"};

    auto create_output(specification const& info) const -> std::unique_ptr<detail::audio_stream>;
    auto create_input() const -> std::unique_ptr<detail::audio_stream>;

private:
    u32 _devicePlayback;
    u32 _deviceRecording;
};

}
