// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/RenderSystem.hpp"

namespace tcob::gfx::null {
////////////////////////////////////////////////////////////

class null_render_system final : public render_system {
public:
    auto get_name() const -> string override;
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

////////////////////////////////////////////////////////////

class null_render_target : public tcob::gfx::render_backend::render_target_base {
public:
    void prepare_render(render_properties const& props) override;
    void finalize_render() const override;

    void enable_scissor(rect_i const& rect, i32 height) const override;
    void disable_scissor() const override;

    void clear(color c) const override;

    void on_resize(size_i size) override;

    auto copy_to_image(rect_i const& rect) const -> image override;

    void bind_material(material const* mat) const override;
    void unbind_material() const override;
};

////////////////////////////////////////////////////////////

class null_shader : public tcob::gfx::render_backend::shader_base {
public:
    auto compile(string const& vertexShaderSource, string const& fragmentShaderSource) -> bool override;

    auto get_uniform_block_binding(string const& name) const -> i32 override;

    auto is_valid() const -> bool override;
};

////////////////////////////////////////////////////////////

class null_texture : public tcob::gfx::render_backend::texture_base {
public:
    void create(size_i texsize, u32 depth, texture::format format = texture::format::RGBA8) override;
    void update(point_i origin, size_i size, void const* data, u32 depth, texture::format format, i32 rowLength = 0, i32 alignment = 4) const override;

    auto get_filtering() const -> texture::filtering override;
    void set_filtering(texture::filtering val) const override;
    auto get_wrapping() const -> texture::wrapping override;
    void set_wrapping(texture::wrapping val) const override;

    auto copy_to_image(u32 depth) const -> image override;

    auto is_valid() const -> bool override;
};

////////////////////////////////////////////////////////////

class null_uniform_buffer final : public tcob::gfx::render_backend::uniform_buffer_base {
public:
    void update(void const* data, usize size, usize offset) const override;

    void bind_base(u32 index) const override;
};

////////////////////////////////////////////////////////////

class null_vertex_array final : public tcob::gfx::render_backend::vertex_array_base {
public:
    void resize(usize vertCount, usize indCount) override;

    void update_data(std::span<vertex const> verts, usize vertOffset) const override;
    void update_data(std::span<quad const> quads, usize quadOffset) const override;
    void update_data(std::span<u32 const> inds, usize indOffset) const override;

    void draw_elements(primitive_type mode, usize count, u32 offset) const override;
    void draw_arrays(primitive_type mode, i32 first, usize count) const override;
};

////////////////////////////////////////////////////////////

class null_window final : public tcob::gfx::render_backend::window_base {
public:
    auto get_vsync() const -> bool override;
    void set_vsync(bool value) override;

    void swap_buffer() const override;

    void clear(color c) const override;

    void set_viewport(rect_i const& rect) override;

    auto get_handle() const -> SDL_Window* override
    {
        return nullptr;
    }
};

////////////////////////////////////////////////////////////

class null_canvas final : public tcob::gfx::render_backend::canvas_base {
public:
    void set_size(size_f size) override;
    void cancel() override;
    void flush() override;
    void render_fill(canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor, f32 fringe,
                     vec4 const& bounds, std::vector<canvas_path> const& paths) override;
    void render_stroke(canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor, f32 fringe,
                       f32 strokeWidth, std::vector<canvas_path> const& paths) override;
    void render_triangles(canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor,
                          std::span<vertex const> verts, f32 fringe) override;
};

} // namespace gfx
