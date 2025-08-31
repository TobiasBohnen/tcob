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

    auto data(this auto&& self) -> decltype(auto);
    auto ptr(this auto&& self) -> decltype(auto);

    auto load [[nodiscard]] (path const& file, std::any const& ctx) noexcept -> bool;
    auto load [[nodiscard]] (std::shared_ptr<io::istream> in, string const& ext, std::any const& ctx) noexcept -> bool;
    auto load_async [[nodiscard]] (path const& file, std::any& ctx) noexcept -> std::future<bool>;

    auto save [[nodiscard]] (path const& file) const noexcept -> bool;
    auto save [[nodiscard]] (io::ostream& out, string const& ext) const noexcept -> bool;
    auto save_async [[nodiscard]] (path const& file) const noexcept -> std::future<bool>;

    static auto Create(specification const& info, std::span<f32 const> data) -> buffer;

    static auto Load(path const& file) -> std::optional<buffer>;
    static auto Load(std::shared_ptr<io::istream> in, string const& ext) -> std::optional<buffer>;

private:
    information      _info;
    std::vector<f32> _buffer;
};

auto buffer::data(this auto&& self) -> decltype(auto)
{
    return std::span {self._buffer.data(), self._buffer.size()};
}

auto buffer::ptr(this auto&& self) -> decltype(auto)
{
    return self._buffer.data();
}

////////////////////////////////////////////////////////////

class TCOB_API decoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<decoder>> {
        static inline char const* ServiceName {"audio::decoder::factory"};
    };

    decoder();
    virtual ~decoder();

    auto open(std::shared_ptr<io::istream> in, std::any const& ctx) -> std::optional<buffer::information>;

    virtual auto decode(std::span<f32> outputSamples) -> isize = 0;

    virtual void seek_from_start(milliseconds pos) = 0;

protected:
    virtual auto open() -> std::optional<buffer::information> = 0;

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

    virtual auto encode(std::span<f32 const> samples, buffer::information const& info, io::ostream& out) const -> bool = 0;
};

}
