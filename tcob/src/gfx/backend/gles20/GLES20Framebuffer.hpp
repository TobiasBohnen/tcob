// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#include <glad/gles20.h>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/gfx/Texture.hpp"

#include "GLES20Object.hpp"
#include "GLES20Texture.hpp"

namespace tcob::gfx::gles20 {
////////////////////////////////////////////////////////////

class gl_framebuffer final : public gl_object {
public:
    gl_framebuffer();
    ~gl_framebuffer() override;

    void bind() const;
    void bind_default() const;

    void clear(color c) const;

    void attach_texture(texture const* tex);
    void attach_texture(gl_texture const* tex, u32 depth);

    void get_subimage(rect_i const& rect, std::span<u8> pixels, GLenum format) const;
    auto read_pixel(point_i pos) const -> color;

protected:
    void do_destroy() override;

private:
    u32 _rbo {0};
    u32 _texID {0};
};
}
