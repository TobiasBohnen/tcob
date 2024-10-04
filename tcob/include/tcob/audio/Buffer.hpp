// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <future>

#include "tcob/core/Common.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API buffer final {
public:
    struct info {
        i32 Channels {0};
        i32 SampleRate {0};
        i64 FrameCount {0};
    };

    auto get_info() const -> info const&;

    auto get_data() const -> std::span<f32 const>;
    auto get_data() -> std::span<f32>;

    auto load [[nodiscard]] (path const& file, std::any& ctx) noexcept -> load_status;
    auto load [[nodiscard]] (std::shared_ptr<io::istream> in, string const& ext, std::any& ctx) noexcept -> load_status;
    auto load_async [[nodiscard]] (path const& file, std::any& ctx) noexcept -> std::future<load_status>;

    auto save [[nodiscard]] (path const& file) const -> bool;
    auto save [[nodiscard]] (io::ostream& out, string const& ext) const -> bool;
    auto save_async [[nodiscard]] (path const& file) const -> std::future<bool>;

    auto static Create(info const& info, std::span<f32> data) -> buffer;

private:
    info             _info;
    std::vector<f32> _buffer;
};

}
