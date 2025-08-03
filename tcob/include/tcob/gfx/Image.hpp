// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <expected>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TypeFactory.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API image final {
public:
    ////////////////////////////////////////////////////////////
    enum class format : u8 {
        RGB,
        RGBA
    };

    class TCOB_API information {
    public:
        size_i Size {size_i::Zero};
        format Format {};

        auto size_in_bytes() const -> isize;
        auto bytes_per_pixel() const -> i32;
        auto stride() const -> i32;

        auto static GetBPP(format f) -> i32;
        auto static HasAlpha(format f) -> bool;
    };

    ////////////////////////////////////////////////////////////

    image();

    auto info() const -> information const&;

    auto data(this auto&& self) -> decltype(auto);
    auto data(rect_i const& bounds) const -> std::vector<u8>;
    auto ptr(this auto&& self) -> decltype(auto);

    auto load [[nodiscard]] (path const& file) noexcept -> bool;
    auto load [[nodiscard]] (io::istream& in, string const& ext) noexcept -> bool;
    auto load_async [[nodiscard]] (path const& file) noexcept -> std::future<bool>;

    auto save [[nodiscard]] (path const& file) const noexcept -> bool;
    auto save [[nodiscard]] (io::ostream& out, string const& ext) const noexcept -> bool;
    auto save_async [[nodiscard]] (path const& file) const noexcept -> std::future<bool>;

    void flip_horizontally();
    void flip_vertically();

    auto get_pixel(point_i pos) const -> color;
    void set_pixel(point_i pos, color c);

    void fill(rect_i const& rect, color c);
    void blit(point_i offset, image const& src);
    void blend(point_i offset, image const& src);
    auto crop(rect_i const& bounds) const -> image;

    auto count_colors [[nodiscard]] () const -> isize;

    auto static Create(size_i size, format f, std::span<u8 const> data) -> image;
    auto static CreateEmpty(size_i size, format f) -> image;

    auto static Load(path const& file) noexcept -> std::expected<image, bool>;
    auto static Load(io::istream& in, string const& ext) noexcept -> std::expected<image, bool>;
    auto static LoadInfo(path const& file) noexcept -> std::optional<information>;

private:
    image(size_i size, format f);
    image(size_i size, format f, std::span<u8 const> data);

    information     _info;
    std::vector<u8> _buffer;
};

inline auto image::data(this auto&& self) -> decltype(auto)
{
    return std::span {self._buffer.data(), self._buffer.size()};
}

inline auto image::ptr(this auto&& self) -> decltype(auto)
{
    return self._buffer.data();
}

////////////////////////////////////////////////////////////

class TCOB_API image_decoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<image_decoder>> {
        static inline char const* ServiceName {"gfx::image_decoder::factory"};
    };

    image_decoder()          = default;
    virtual ~image_decoder() = default;

    auto virtual decode(io::istream& in) -> std::optional<image>                   = 0;
    auto virtual decode_info(io::istream& in) -> std::optional<image::information> = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API image_encoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<image_encoder>> {
        static inline char const* ServiceName {"gfx::image_encoder::factory"};
    };

    image_encoder()          = default;
    virtual ~image_encoder() = default;

    auto virtual encode(image const& img, io::ostream& out) const -> bool = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API animated_image_decoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<animated_image_decoder>> {
        static inline char const* ServiceName {"gfx::animated_image_decoder::factory"};
    };

    enum class status : u8 {
        NewFrame,
        OldFrame,
        NoMoreFrames,
        DecodeFailure
    };

    animated_image_decoder()          = default;
    virtual ~animated_image_decoder() = default;

    auto open(std::shared_ptr<io::istream> in) -> std::optional<image::information>;

    auto virtual current_frame() const -> std::span<u8 const> = 0;
    auto virtual advance(milliseconds ts) -> status           = 0;
    void virtual reset()                                      = 0;

protected:
    auto virtual open() -> std::optional<image::information> = 0;

    auto stream() -> io::istream&;

private:
    std::shared_ptr<io::istream> _stream;
};

////////////////////////////////////////////////////////////

class TCOB_API animated_image_encoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<animated_image_encoder>> {
        static inline char const* ServiceName {"gfx::animated_image_encoder::factory"};
    };

    animated_image_encoder()          = default;
    virtual ~animated_image_encoder() = default;

    struct frame {
        image        Image;
        milliseconds TimeStamp {0};
    };

    auto virtual encode(std::span<frame const> frames, io::ostream& out) -> bool = 0;
};

////////////////////////////////////////////////////////////

}
