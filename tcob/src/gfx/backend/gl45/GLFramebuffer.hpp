// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/gfx/Texture.hpp"

#include "GLObject.hpp"

namespace tcob::gfx::gl45 {
////////////////////////////////////////////////////////////

class gl_framebuffer final : public gl_object {
public:
    gl_framebuffer();
    ~gl_framebuffer() override;

    void bind() const;
    void bind_default() const;

    void clear(color c) const;

    void attach_texture(texture const* tex);

    void get_subimage(rect_i const& rect, std::span<u8> pixels);
    auto read_pixel(point_i pos) const -> color;

protected:
    void do_destroy() override;

private:
    u32 _rbo {0};
    u32 _texID {0};
};
}
