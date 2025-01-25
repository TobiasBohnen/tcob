// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <future>
#include <optional>

#include "tcob/audio/Buffer.hpp"
#include "tcob/audio/Source.hpp"
#include "tcob/core/Common.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API sound final : public source {
public:
    sound();
    explicit sound(audio::buffer const& buffer);
    ~sound() override;

    auto info() const -> std::optional<buffer::information>;
    auto duration() const -> milliseconds override;
    auto playback_position() const -> milliseconds override;

    auto load [[nodiscard]] (path const& file) noexcept -> load_status;
    auto load [[nodiscard]] (std::shared_ptr<io::istream> in, string const& ext) noexcept -> load_status;
    auto load_async [[nodiscard]] (path const& file) noexcept -> std::future<load_status>;

    static inline char const* asset_name {"sound"};

private:
    auto on_start() -> bool override;
    auto on_stop() -> bool override;

    void stop_source();

    buffer::information                   _info;
    std::unique_ptr<audio::al::al_buffer> _buffer;
};
}
