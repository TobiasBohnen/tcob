// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <future>
#include <span>

#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/TypeFactory.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API buffer final {
public:
    struct info {
        i32 Channels {0};
        i32 SampleRate {0};
        i64 FrameCount {0};
    };

    auto get_info() const -> info const&; // TODO: get_

    auto data() const -> std::span<f32 const>;
    auto data() -> std::span<f32>;

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

////////////////////////////////////////////////////////////

class TCOB_API decoder {
public:
    struct factory : public type_factory<std::unique_ptr<decoder>> {
        static inline char const* service_name {"audio::decoder::factory"};
    };

    decoder();
    decoder(decoder const& other) noexcept                    = delete;
    auto operator=(decoder const& other) noexcept -> decoder& = delete;
    virtual ~decoder();

    auto open(std::shared_ptr<io::istream> in, std::any& ctx) -> std::optional<buffer::info>;

    auto decode(isize size) -> std::optional<std::vector<f32>>;

    void virtual seek_from_start(milliseconds pos) = 0;

protected:
    auto virtual open() -> std::optional<buffer::info>       = 0;
    auto virtual decode(std::span<f32> outputSamples) -> i32 = 0;

    auto stream() -> io::istream&;
    auto context() -> std::any&;

private:
    std::shared_ptr<io::istream> _stream;
    std::any                     _ctx;
    std::optional<buffer::info>  _info;
};

////////////////////////////////////////////////////////////

class TCOB_API encoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<encoder>> {
        static inline char const* service_name {"audio::encoder::factory"};
    };

    encoder()          = default;
    virtual ~encoder() = default;

    auto virtual encode(std::span<f32 const> samples, buffer::info const& info, io::ostream& out) const -> bool = 0;
};

}
