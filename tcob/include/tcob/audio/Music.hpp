// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <atomic>

#include "tcob/audio/Buffer.hpp"
#include "tcob/audio/Source.hpp"
#include "tcob/core/Common.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

constexpr i64 STREAM_BUFFER_SIZE {4096};
constexpr u8  STREAM_BUFFER_COUNT {4};

class decoder;

////////////////////////////////////////////////////////////

class TCOB_API music : public source {
public:
    music() = default;
    ~music() override;

    auto info() const -> std::optional<buffer::information>;
    auto duration() const -> milliseconds override;
    auto playback_position() const -> milliseconds override;

    auto open [[nodiscard]] (path const& file) -> load_status;
    auto open [[nodiscard]] (std::shared_ptr<io::istream> in, string const& ext) -> load_status;

    static inline char const* asset_name {"music"};

private:
    auto on_start() -> bool override;
    auto on_stop() -> bool override;

    void update_stream();
    void stop_stream();
    void queue_buffers(std::vector<u32> const& bufferIDs);
    void initialize_buffers();

    using buffer_array = std::array<std::shared_ptr<audio::al::al_buffer>, STREAM_BUFFER_COUNT>;

    std::unique_ptr<decoder>           _decoder {};
    i32                                _samplesPlayed {0};
    std::optional<buffer::information> _info;
    buffer_array                       _buffers {};

    std::atomic_bool _isRunning {false};
    std::atomic_bool _stopRequested {false};
};
}
