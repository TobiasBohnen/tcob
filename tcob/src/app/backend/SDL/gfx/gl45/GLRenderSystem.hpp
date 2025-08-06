// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/core/Size.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::gl45 {

class gl_render_system final : public render_system {
public:
    auto name() const -> string override;
    auto device_name() const -> string override;
    auto capabilities() const -> render_capabilities override;

    auto create_canvas() -> std::unique_ptr<render_backend::canvas_base> override;
    auto create_render_target(texture* tex) -> std::unique_ptr<render_backend::render_target_base> override;
    auto create_shader() -> std::unique_ptr<render_backend::shader_base> override;
    auto create_texture() -> std::unique_ptr<render_backend::texture_base> override;
    auto create_uniform_buffer(usize size) -> std::unique_ptr<render_backend::uniform_buffer_base> override;
    auto create_vertex_array(buffer_usage_hint usage) -> std::unique_ptr<render_backend::vertex_array_base> override;
    auto create_window(size_i size) -> std::unique_ptr<gfx::window> override;
};

} // namespace gfx
