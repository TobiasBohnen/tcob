// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <atomic>
#include <memory>
#include <optional>
#include <queue>

#include "tcob/audio/Audio.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/audio/Source.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/easing/Tween.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API music final : public source {
    constexpr static i64 STREAM_BUFFER_SIZE {4096};
    constexpr static u8  STREAM_BUFFER_COUNT {4};
    constexpr static i64 STREAM_BUFFER_THRESHOLD {STREAM_BUFFER_SIZE * (STREAM_BUFFER_COUNT - 1)};

public:
    music() = default;
    ~music() override;

    prop<milliseconds> FadeIn;
    prop<milliseconds> FadeOut;

    auto info() const -> std::optional<specification> override;
    auto duration() const -> milliseconds override;
    auto playback_position() const -> milliseconds;

    auto open [[nodiscard]] (path const& file) -> load_status;
    auto open [[nodiscard]] (std::shared_ptr<io::istream> in, string const& ext) -> load_status;

    static inline char const* AssetName {"music"};

private:
    auto on_start() -> bool override;
    auto on_stop() -> bool override;

    void update_stream();
    void stop_stream();
    void fill_buffers();

    std::unique_ptr<decoder>     _decoder {};
    usize                        _samplesPlayed {0};
    std::optional<specification> _info;
    i64                          _totalFrameCount {0};

    struct stream_buffer {
        std::array<f32, STREAM_BUFFER_SIZE> Data {};
        isize                               Size {0};
        bool                                Queued {false};
    };
    std::array<stream_buffer, STREAM_BUFFER_COUNT> _buffers;
    std::queue<stream_buffer*>                     _bufferQueue {};

    std::unique_ptr<linear_tween<f32>> _fadeTween;
    uid                                _deferred {INVALID_ID};

    std::atomic_bool _isRunning {false};
    std::atomic_bool _stopRequested {false};
};
}
