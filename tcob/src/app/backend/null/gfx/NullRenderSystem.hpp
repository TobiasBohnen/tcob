// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::null {
////////////////////////////////////////////////////////////

class null_render_system final : public render_system {
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

////////////////////////////////////////////////////////////

class null_render_target : public tcob::gfx::render_backend::render_target_base {
public:
    void prepare_render(render_properties const& props) override;
    void finalize_render() const override;

    void enable_scissor(rect_i const& rect) const override;
    void disable_scissor() const override;

    void clear(color c) const override;

    void on_resize(size_i size) override;

    auto copy_to_image(rect_i const& rect) const -> image override;

    void bind_pass(pass const& pass) const override;
    void unbind_pass() const override;
};

////////////////////////////////////////////////////////////

class null_shader : public tcob::gfx::render_backend::shader_base {
public:
    auto compile(string const& vertexShaderSource, string const& fragmentShaderSource) -> bool override;

    auto is_valid() const -> bool override;
};

////////////////////////////////////////////////////////////

class null_texture : public tcob::gfx::render_backend::texture_base {
public:
    void resize(size_i texsize, u32 depth, texture::format format = texture::format::RGBA8) override;
    void update(point_i origin, size_i size, void const* data, u32 depth, i32 rowLength = 0, i32 alignment = 4) const override;

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
    void update_data(std::span<u32 const> inds, usize indOffset) const override;

    void draw_elements(primitive_type mode, usize count, u32 offset) const override;
    void draw_arrays(primitive_type mode, i32 first, usize count) const override;
};

////////////////////////////////////////////////////////////

class null_window_impl final : public tcob::gfx::render_backend::window_base {
public:
    auto get_vsync() const -> bool override;
    void set_vsync(bool value) override;

    void swap_buffer() const override;

    void clear(color c) const override;

    void set_viewport(rect_i const& rect) override;

    auto get_handle() const -> void* override
    {
        return nullptr;
    }
};

class null_window final : public tcob::gfx::window {
public:
    null_window();

    void load_icon(path const& file) override;

    auto has_focus() const -> bool override;
    void grab_input(bool grab) override;

    void process_events(void* ev) override;

    auto get_size() const -> size_i override;
    void set_size(size_i newsize) override;

    auto get_fullscreen() const -> bool override;
    void set_fullscreen(bool value) override;

    auto get_title() const -> string override;
    void set_title(string const& value) override;
};

////////////////////////////////////////////////////////////

class null_canvas final : public tcob::gfx::render_backend::canvas_base {
public:
    void flush(size_f size) override;
    void cancel() override;
    void render_fill(canvas::paint const& paint, blend_funcs const& compositeOperation, canvas::scissor const& scissor, f32 fringe,
                     vec4 const& bounds, std::vector<canvas::path> const& paths) override;
    void render_stroke(canvas::paint const& paint, blend_funcs const& compositeOperation, canvas::scissor const& scissor, f32 fringe,
                       f32 strokeWidth, std::vector<canvas::path> const& paths) override;
    void render_triangles(canvas::paint const& paint, blend_funcs const& compositeOperation, canvas::scissor const& scissor, f32 fringe,
                          std::span<vertex const> verts) override;
    void render_clip(canvas::scissor const& scissor, f32 fringe, std::vector<canvas::path> const& paths) override;
    void add_gradient(i32 idx, color_gradient const& gradient) override;
};

}
