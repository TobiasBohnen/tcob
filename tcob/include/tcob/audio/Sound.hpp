// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <future>
#include <memory>
#include <optional>

#include "tcob/audio/Audio.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/audio/Source.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API sound final : public source {
public:
    sound() = default;
    explicit sound(buffer buffer);
    ~sound() override = default;

    auto info() const -> std::optional<specification> override;
    auto duration() const -> milliseconds override;

    auto load [[nodiscard]] (path const& file) noexcept -> bool;
    auto load [[nodiscard]] (std::shared_ptr<io::istream> in, string const& ext) noexcept -> bool;
    auto load_async [[nodiscard]] (path const& file) noexcept -> std::future<bool>;

    static inline char const* AssetName {"sound"};

private:
    auto on_start() -> bool override;
    auto on_stop() -> bool override;

    buffer _buffer;
};
}
