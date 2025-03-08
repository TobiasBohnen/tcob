// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "GLES30Object.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::gles30 {
////////////////////////////////////////////////////////////

class gl_texture : public gl_object, public tcob::gfx::render_backend::texture_base {
public:
    ~gl_texture() override;

    void create(size_i texsize, u32 depth, texture::format format = texture::format::RGBA8) override;
    void update(point_i origin, size_i size, void const* data, u32 depth, i32 rowLength = 0, i32 alignment = 4) const override;

    auto get_filtering() const -> texture::filtering override;
    void set_filtering(texture::filtering val) const override;
    auto get_wrapping() const -> texture::wrapping override;
    void set_wrapping(texture::wrapping val) const override;

    auto copy_to_image(u32 depth) const -> image override;

    auto is_valid() const -> bool override;

    auto get_size() const -> size_i;

protected:
    void create();

    void do_destroy() override;

private:
    void bind() const;

    size_i          _size {size_i::Zero};
    texture::format _format {};
};

}
