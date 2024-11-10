// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/RenderSystem.hpp"

namespace tcob::gfx::gles30 {

class gl_render_system final : public render_system {
public:
    auto name() const -> string override;
    auto get_device_name() const -> string override;
    auto get_capabilities() const -> capabilities override;
    auto get_rtt_coords() const -> rect_f override;

    auto create_canvas() -> std::unique_ptr<render_backend::canvas_base> override;
    auto create_render_target(texture* tex) -> std::unique_ptr<render_backend::render_target_base> override;
    auto create_shader() -> std::unique_ptr<render_backend::shader_base> override;
    auto create_texture() -> std::unique_ptr<render_backend::texture_base> override;
    auto create_uniform_buffer(usize size) -> std::unique_ptr<render_backend::uniform_buffer_base> override;
    auto create_vertex_array(buffer_usage_hint usage) -> std::unique_ptr<render_backend::vertex_array_base> override;
    auto create_window(size_i size) -> std::unique_ptr<render_backend::window_base> override;
};

} // namespace gfx
