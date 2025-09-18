// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>
#include <unordered_map>

#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API texture {
public:
    ////////////////////////////////////////////////////////////
    enum class format : u8 {
        R8,
        RGB8,
        RGBA8
    };

    enum class filtering : u8 {
        Linear,
        NearestNeighbor
    };

    enum class wrapping : u8 {
        ClampToEdge,
        ClampToBorder,
        MirroredRepeat,
        Repeat,
        MirrorClampToEdge
    };

    struct information final {
        size_i Size;
        format Format {};
        u32    Depth {};

        auto operator==(information const& other) const -> bool = default;
    };
    ////////////////////////////////////////////////////////////

    texture();
    texture(size_i size, u32 depth, format f);
    virtual ~texture();

    explicit operator bool() const;
    auto     is_valid() const -> bool;

    prop_fn<filtering> Filtering;
    prop_fn<wrapping>  Wrapping;

    auto info() const -> information;

    void resize(size_i size, u32 depth, format f);

    void update_data(void const* data, u32 depth, i32 rowLength = 0, i32 alignment = 4) const;
    void update_data(std::span<u8 const> data, u32 depth, i32 rowLength = 0, i32 alignment = 4) const;
    void update_data(image const& img, u32 depth, i32 rowLength = 0, i32 alignment = 4) const;
    void update_data(point_i origin, size_i size, void const* data, u32 depth, i32 rowLength = 0, i32 alignment = 4) const;

    auto copy_to_image(u32 level) const -> image;

    auto regions(this auto&& self) -> decltype(auto);

    template <std::derived_from<render_backend::texture_base> T>
    auto get_impl() const -> T*;

    static inline char const* AssetName {"texture"};

private:
    std::unique_ptr<render_backend::texture_base> _impl;

    std::unordered_map<string, texture_region> _regions {}; // perfcrit

    size_i _size {size_i::Zero};
    format _format {format::RGBA8};
    u32    _depth {0};
};

auto texture::regions(this auto&& self) -> decltype(auto)
{
    return (self._regions);
}

////////////////////////////////////////////////////////////

class TCOB_API animated_texture final : public texture, public updatable {
public:
    animated_texture()           = default;
    ~animated_texture() override = default;

    signal<> NewFrame;

    auto load [[nodiscard]] (path const& file) noexcept -> bool;
    auto load [[nodiscard]] (std::shared_ptr<io::istream> in, string const& ext) noexcept -> bool;

    auto state() const -> playback_state;
    auto is_looping() const -> bool;

    void start(bool looping = false);
    void stop();
    void restart();

    void pause();
    void resume();
    void toggle_pause();

protected:
    void reset_decoder();

    void on_update(milliseconds deltaTime) override;

private:
    image::information _frameInfo {};
    milliseconds       _elapsedTime {0};

    std::unique_ptr<animated_image_decoder> _decoder {};

    playback_state _state {playback_state::Stopped};
    bool           _isLooping {false};
};

}

#include "Texture.inl"
