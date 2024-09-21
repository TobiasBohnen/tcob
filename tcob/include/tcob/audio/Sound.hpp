// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <future>
#include <optional>

#include "tcob/audio/AudioSource.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Common.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API sound final : public source {
public:
    sound();
    explicit sound(audio::buffer const& buffer);
    explicit sound(std::shared_ptr<audio::al::al_buffer> buffer);
    sound(sound const& other) noexcept                    = default;
    auto operator=(sound const& other) noexcept -> sound& = default;
    sound(sound&& other) noexcept                         = default;
    auto operator=(sound&& other) noexcept -> sound&      = default;
    ~sound() override;

    auto get_info() const -> std::optional<buffer::info>;
    auto get_duration() const -> milliseconds override;
    auto get_playback_position() const -> milliseconds override;

    auto load [[nodiscard]] (path const& file) noexcept -> load_status;
    auto load [[nodiscard]] (std::shared_ptr<istream> in, string const& ext) noexcept -> load_status;
    auto load_async [[nodiscard]] (path const& file) noexcept -> std::future<load_status>;

    static inline char const* asset_name {"sound"};

private:
    auto on_start() -> bool override;
    auto on_stop() -> bool override;

    void stop_source();

    buffer::info                          _info;
    std::shared_ptr<audio::al::al_buffer> _buffer;
};
}
