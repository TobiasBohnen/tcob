// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <future>
#include <optional>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TypeFactory.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API image final {
public:
    ////////////////////////////////////////////////////////////
    enum class format : u8 {
        RGB,
        RGBA
    };

    class TCOB_API info {
    public:
        size_i Size {size_i::Zero};
        format Format {};

        auto size_in_bytes() const -> isize;
        auto bytes_per_pixel() const -> i32;
        auto stride() const -> i32;

        auto static GetBPP(format f) -> i32;
    };

    ////////////////////////////////////////////////////////////

    image() = default;

    auto get_info() const -> info const&;

    auto get_data() const -> std::span<u8 const>;
    auto get_data() -> std::span<u8>;
    auto get_data(rect_i const& bounds) const -> std::vector<u8>;

    auto load [[nodiscard]] (path const& file) noexcept -> load_status;
    auto load [[nodiscard]] (istream& in, string const& ext) noexcept -> load_status;
    auto load_async [[nodiscard]] (path const& file) noexcept -> std::future<load_status>;

    auto save [[nodiscard]] (path const& file) const -> bool;
    auto save [[nodiscard]] (ostream& out, string const& ext) const -> bool;
    auto save_async [[nodiscard]] (path const& file) const -> std::future<bool>;

    void flip_horizontally();
    void flip_vertically();

    auto get_pixel(point_i pos) const -> color;
    void set_pixel(point_i pos, color c);

    auto static Create(size_i size, format f, std::span<u8 const> data) -> image;
    auto static CreateEmpty(size_i size, format f) -> image;

    auto static Load(path const& file) noexcept -> std::optional<image>;               // TODO: change to result
    auto static Load(istream& in, string const& ext) noexcept -> std::optional<image>; // TODO: change to result
    auto static LoadInfo(path const& file) noexcept -> std::optional<info>;            // TODO: change to result

private:
    image(size_i size, format f);
    image(size_i size, format f, std::span<u8 const> data);

    std::vector<u8> _buffer;
    info            _info;
};

////////////////////////////////////////////////////////////

class TCOB_API image_decoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<image_decoder>> {
        static inline char const* service_name {"gfx::image_decoder::factory"};
    };

    image_decoder()          = default;
    virtual ~image_decoder() = default;

    auto virtual decode(istream& in) -> std::optional<image>              = 0;
    auto virtual decode_header(istream& in) -> std::optional<image::info> = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API image_encoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<image_encoder>> {
        static inline char const* service_name {"gfx::image_encoder::factory"};
    };

    image_encoder()          = default;
    virtual ~image_encoder() = default;

    auto virtual encode(image const& img, ostream& out) const -> bool = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API animated_image_decoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<animated_image_decoder>> {
        static inline char const* service_name {"gfx::animated_image_decoder::factory"};
    };

    enum class status : u8 {
        NewFrame,
        OldFrame,
        NoMoreFrames,
        DecodeFailure
    };

    animated_image_decoder()          = default;
    virtual ~animated_image_decoder() = default;

    auto open(std::shared_ptr<istream> in) -> std::optional<image::info>;

    auto virtual get_current_frame() const -> u8 const*       = 0;
    auto virtual seek_from_current(milliseconds ts) -> status = 0;
    void virtual reset()                                      = 0;

protected:
    auto virtual open() -> std::optional<image::info> = 0;

    auto get_stream() -> istream&;

private:
    std::shared_ptr<istream> _stream;
};

////////////////////////////////////////////////////////////

class TCOB_API animated_image_encoder : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<animated_image_encoder>> {
        static inline char const* service_name {"gfx::animated_image_encoder::factory"};
    };

    animated_image_encoder()          = default;
    virtual ~animated_image_encoder() = default;

    struct frame {
        image        Image;
        milliseconds TimeStamp {0};
    };

    auto virtual encode(std::span<frame const> frames, ostream& out) -> bool = 0;
};

////////////////////////////////////////////////////////////

}
