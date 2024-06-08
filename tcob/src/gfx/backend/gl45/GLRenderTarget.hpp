// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "GLFramebuffer.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx::gl45 {
////////////////////////////////////////////////////////////

class gl_render_target : public tcob::gfx::render_backend::render_target_base {
public:
    gl_render_target(texture* tex);

    void prepare_render(render_properties const& props) override;
    void finalize_render() const override;

    void enable_scissor(rect_i const& rect, i32 height) const override;
    void disable_scissor() const override;

    void clear(color c) const override;

    void on_resize(size_i size) override;

    auto copy_to_image(rect_i const& rect) const -> image override;

    void bind_material(material* mat) const override;
    void unbind_material() const override;

private:
    void set_viewport(rect_i const& rect);

    texture*                        _tex {nullptr};
    std::unique_ptr<gl_framebuffer> _frameBuffer;
};

}
