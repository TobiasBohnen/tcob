// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "NullRenderSystem.hpp"

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

auto null_render_system::name() const -> string { return "NULL"; }
auto null_render_system::device_name() const -> string { return "NULL"; }
auto null_render_system::capabilities() const -> render_capabilities { return {}; }
auto null_render_system::create_canvas() -> std::unique_ptr<render_backend::canvas_base> { return std::make_unique<null_canvas>(); }
auto null_render_system::create_render_target(texture*) -> std::unique_ptr<render_backend::render_target_base> { return std::make_unique<null_render_target>(); }
auto null_render_system::create_shader() -> std::unique_ptr<render_backend::shader_base> { return std::make_unique<null_shader>(); }
auto null_render_system::create_texture() -> std::unique_ptr<render_backend::texture_base> { return std::make_unique<null_texture>(); }
auto null_render_system::create_uniform_buffer(usize) -> std::unique_ptr<render_backend::uniform_buffer_base> { return std::make_unique<null_uniform_buffer>(); }
auto null_render_system::create_vertex_array(buffer_usage_hint) -> std::unique_ptr<render_backend::vertex_array_base> { return std::make_unique<null_vertex_array>(); }
auto null_render_system::create_window(size_i) -> std::unique_ptr<gfx::window> { return std::make_unique<null_window>(); }

void null_render_target::prepare_render(render_properties const&) { }
void null_render_target::finalize_render() const { }
void null_render_target::enable_scissor(rect_i const&) const { }
void null_render_target::disable_scissor() const { }
void null_render_target::clear(color) const { }
void null_render_target::on_resize(size_i) { }
auto null_render_target::copy_to_image(rect_i const&) const -> image { return {}; }
void null_render_target::bind_pass(pass const&) const { }
void null_render_target::unbind_pass() const { }

auto null_shader::compile(string const&, string const&) -> bool { return true; }
auto null_shader::is_valid() const -> bool { return true; }

void null_texture::resize(size_i, u32, texture::format) { }
void null_texture::update(point_i, size_i, void const*, u32, i32, i32) const { }
auto null_texture::get_filtering() const -> texture::filtering { return texture::filtering::Linear; }
void null_texture::set_filtering(texture::filtering) const { }
auto null_texture::get_wrapping() const -> texture::wrapping { return texture::wrapping::Repeat; }
void null_texture::set_wrapping(texture::wrapping) const { }
auto null_texture::copy_to_image(u32) const -> image { return {}; }
auto null_texture::is_valid() const -> bool { return true; }

void null_vertex_array::resize(usize, usize) { }
void null_vertex_array::update_data(std::span<vertex const>, usize) const { }
void null_vertex_array::update_data(std::span<u32 const>, usize) const { }
void null_vertex_array::draw_elements(primitive_type, usize, u32) const { }
void null_vertex_array::draw_arrays(primitive_type, i32, usize) const { }

auto null_window_impl::get_vsync() const -> bool { return true; }
void null_window_impl::set_vsync(bool) { }
void null_window_impl::swap_buffer() const { }
void null_window_impl::clear(color) const { }
void null_window_impl::set_viewport(rect_i const&) { }

null_window::null_window()
    : tcob::gfx::window(std::make_unique<null_window_impl>())
{
}
void null_window::load_icon(path const&) { }
auto null_window::has_focus() const -> bool { return true; }
void null_window::grab_input(bool) { }
void null_window::process_events(void*) { }
auto null_window::get_size() const -> size_i { return size_i::Zero; }
void null_window::set_size(size_i) { }
auto null_window::get_fullscreen() const -> bool { return true; }
void null_window::set_fullscreen(bool) { }
auto null_window::get_title() const -> string { return ""; }
void null_window::set_title(string const&) { }

void null_canvas::cancel() { }
void null_canvas::flush(size_f) { }
void null_canvas::render_fill(canvas::paint const&, blend_funcs const&, canvas::scissor const&, f32, vec4 const&, std::vector<canvas::path> const&) { }
void null_canvas::render_stroke(canvas::paint const&, blend_funcs const&, canvas::scissor const&, f32, f32, std::vector<canvas::path> const&) { }
void null_canvas::render_triangles(canvas::paint const&, blend_funcs const&, canvas::scissor const&, f32, std::span<vertex const>) { }
void null_canvas::render_clip(canvas::scissor const&, f32, std::vector<canvas::path> const&) { }
void null_canvas::add_gradient(i32, color_gradient const&) { }

void null_uniform_buffer::update(void const*, usize, usize) const { }
void null_uniform_buffer::bind_base(u32) const { }

}
