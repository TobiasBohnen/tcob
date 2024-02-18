// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

#include "GLObject.hpp"

namespace tcob::gfx::gl45 {
////////////////////////////////////////////////////////////

class gl_texture : public gl_object, public tcob::gfx::render_backend::texture_base {
public:
    ~gl_texture() override;

    void create(size_i texsize, u32 depth, texture::format format = texture::format::RGBA8) override;
    void update(point_i origin, size_i size, void const* data, u32 depth, texture::format format, i32 rowLength = 0, i32 alignment = 4) const override;

    auto get_filtering() const -> texture::filtering override;
    void set_filtering(texture::filtering val) const override;
    auto get_wrapping() const -> texture::wrapping override;
    void set_wrapping(texture::wrapping val) const override;

    auto copy_to_image(u32 depth) const -> image override;

    auto is_valid() const -> bool override;

    auto get_id() const -> u32;

protected:
    void create(i32 type);

    void do_destroy() override;

private:
    size_i _size {size_i::Zero};
};

}
