// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "tcob/audio/Audio.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/TypeFactory.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API buffer final {
public:
    class TCOB_API information {
    public:
        specification Specs {};
        i64           FrameCount {0};

        auto operator==(information const& other) const -> bool = default;
    };

    auto info() const -> information const&;

    auto data() -> std::span<f32>;
    auto data() const -> std::span<f32 const>;
    auto ptr() -> f32*;
    auto ptr() const -> f32 const*;

    auto load [[nodiscard]] (path const& file, std::any const& ctx) noexcept -> load_status;
    auto load [[nodiscard]] (std::shared_ptr<io::istream> in, string const& ext, std::any const& ctx) noexcept -> load_status;
    auto load_async [[nodiscard]] (path const& file, std::any& ctx) noexcept -> std::future<load_status>;

    auto save [[nodiscard]] (path const& file) const noexcept -> bool;
    auto save [[nodiscard]] (io::ostream& out, string const& ext) const noexcept -> bool;
    auto save_async [[nodiscard]] (path const& file) const noexcept -> std::future<bool>;

    auto static Create(specification const& info, std::span<f32 const> data) -> buffer;

    auto static Load(path const& file) -> std::optional<buffer>;                                   // TODO: change to result
    auto static Load(std::shared_ptr<io::istream> in, string const& ext) -> std::optional<buffer>; // TODO: change to result

private:
    information      _info;
    std::vector<f32> _buffer;
};

////////////////////////////////////////////////////////////

class TCOB_API decoder {
public:
    struct factory : public type_factory<std::unique_ptr<decoder>> {
        static inline char const* ServiceName {"audio::decoder::factory"};
    };

    decoder();
    decoder(decoder const& other) noexcept                    = delete;
    auto operator=(decoder const& other) noexcept -> decoder& = delete;
    virtual ~decoder();

    auto open(std::shared_ptr<io::istream> in, std::any const& ctx) -> std::optional<buffer::information>;

    auto virtual decode(std::span<f32> outputSamples) -> isize = 0;

    void virtual seek_from_start(milliseconds pos) = 0;

protected:
    auto virtual open() -> std::optional<buffer::information> = 0;

    auto stream() -> io::istream&;
    auto context() -> std::any&;

private:
    std::shared_ptr<io::istream>       _stream;
    std::any                           _ctx;
    std::optional<buffer::information> _info;
};

////////////////////////////////////////////////////////////

class TCOB_API encoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<encoder>> {
        static inline char const* ServiceName {"audio::encoder::factory"};
    };

    encoder()          = default;
    virtual ~encoder() = default;

    auto virtual encode(std::span<f32 const> samples, buffer::information const& info, io::ostream& out) const -> bool = 0;
};

}
