// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#include "tcob/core/Color.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Texture.hpp"

struct SDL_Window;

namespace tcob::gfx::render_backend {
////////////////////////////////////////////////////////////

class canvas_base {
public:
    virtual ~canvas_base() = default;

    void virtual flush(size_f size) = 0;
    void virtual cancel()           = 0;

    void virtual render_fill(
        canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor,
        f32 fringe, vec4 const& bounds, std::vector<canvas_path> const& paths) = 0;
    void virtual render_stroke(
        canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor,
        f32 fringe, f32 strokeWidth, std::vector<canvas_path> const& paths) = 0;
    void virtual render_triangles(
        canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor,
        std::span<vertex const> verts, f32 fringe)                     = 0;
    void virtual add_gradient(i32 idx, color_gradient const& gradient) = 0;
};

////////////////////////////////////////////////////////////

class render_target_base {
public:
    virtual ~render_target_base() = default;

    void virtual prepare_render(render_properties const& props) = 0;
    void virtual finalize_render() const                        = 0;
    void virtual bind_material(material const* mat) const       = 0;
    void virtual unbind_material() const                        = 0;

    void virtual enable_scissor(rect_i const& rect, i32 height) const = 0;
    void virtual disable_scissor() const                              = 0;
    void virtual clear(color c) const                                 = 0;

    void virtual on_resize(size_i size) = 0;

    auto virtual copy_to_image(rect_i const& rect) const -> image = 0;
};

////////////////////////////////////////////////////////////

class shader_base {
public:
    virtual ~shader_base() = default;

    auto virtual compile(string const& vert, string const& frag) -> bool = 0; // TODO: change to files

    auto virtual is_valid() const -> bool = 0;
};

////////////////////////////////////////////////////////////

class texture_base {
public:
    virtual ~texture_base() = default;

    void virtual create(size_i texsize, u32 depth, texture::format format = texture::format::RGBA8) = 0;

    void virtual update(point_i origin, size_i size, void const* data, u32 depth, i32 rowLength, i32 alignment) const = 0;

    auto virtual get_filtering() const -> texture::filtering   = 0;
    void virtual set_filtering(texture::filtering props) const = 0;

    auto virtual get_wrapping() const -> texture::wrapping   = 0;
    void virtual set_wrapping(texture::wrapping props) const = 0;

    auto virtual copy_to_image(u32 depth) const -> image = 0;

    auto virtual is_valid() const -> bool = 0;
};

////////////////////////////////////////////////////////////

class uniform_buffer_base {
public:
    virtual ~uniform_buffer_base() = default;

    void virtual update(void const* data, usize size, usize offset) const = 0;

    void virtual bind_base(u32 index) const = 0;
};

////////////////////////////////////////////////////////////

class vertex_array_base {
public:
    virtual ~vertex_array_base() = default;

    void virtual resize(usize vertCount, usize indCount) = 0;

    void virtual update_data(std::span<vertex const> verts, usize vertOffset) const = 0;
    void virtual update_data(std::span<quad const> quads, usize quadOffset) const   = 0;
    void virtual update_data(std::span<u32 const> inds, usize indOffset) const      = 0;

    void virtual draw_elements(primitive_type mode, usize count, u32 offset) const = 0;
    void virtual draw_arrays(primitive_type mode, i32 first, usize count) const    = 0;
};

////////////////////////////////////////////////////////////

class window_base {
public:
    virtual ~window_base() = default;

    auto virtual get_vsync() const -> bool = 0;
    void virtual set_vsync(bool value)     = 0;

    void virtual clear(color c) const = 0;

    void virtual swap_buffer() const = 0;

    void virtual set_viewport(rect_i const& rect) = 0;

    auto virtual get_handle() const -> SDL_Window* = 0;
};
}
