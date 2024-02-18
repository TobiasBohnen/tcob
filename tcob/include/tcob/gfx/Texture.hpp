// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

namespace render_backend {
    class texture_base;
}

////////////////////////////////////////////////////////////

class TCOB_API texture {
public:
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

    texture();
    virtual ~texture();

    explicit operator bool() const;

    prop_fn<filtering> Filtering;
    prop_fn<wrapping>  Wrapping;

    auto get_size() const -> size_i;
    auto get_format() const -> format;
    auto get_depth() const -> u32;

    void create(size_i size, u32 depth, format f);

    void update_data(void const* data, u32 depth, i32 rowLength = 0, i32 alignment = 4) const;
    void update_data(std::span<u8 const> data, u32 depth, i32 rowLength = 0, i32 alignment = 4) const;
    void update_data(point_i origin, size_i size, void const* data, u32 depth, i32 rowLength = 0, i32 alignment = 4) const;

    auto copy_to_image(u32 level) const -> image;

    auto get_region(string const& name) const -> texture_region const&;
    void add_region(string const& name, texture_region const& region);
    auto has_region(string const& name) const -> bool;

    template <std::derived_from<render_backend::texture_base> T>
    auto get_impl() const -> T*;

    static inline char const* asset_name {"texture"};

private:
    std::unique_ptr<render_backend::texture_base> _impl;

    std::unordered_map<string, texture_region> _regions {}; // perfcrit

    size_i _size {size_i::Zero};
    format _format {format::RGBA8};
    u32    _depth {0};
};

////////////////////////////////////////////////////////////

class TCOB_API animated_texture final : public texture, public updatable {
public:
    animated_texture()           = default;
    ~animated_texture() override = default;

    auto load [[nodiscard]] (path const& file) noexcept -> load_status;
    auto load [[nodiscard]] (std::shared_ptr<istream> in, string const& ext) noexcept -> load_status;

    auto is_running() const -> bool;
    auto is_paused() const -> bool;
    auto is_stopped() const -> bool;
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
    image::info  _frameInfo {};
    milliseconds _elapsedTime {0};

    std::unique_ptr<animated_image_decoder> _decoder {};

    bool            _updateTexture {true};
    playback_status _status {playback_status::Stopped};
    bool            _isLooping {false};
};

}

#include "Texture.inl"
