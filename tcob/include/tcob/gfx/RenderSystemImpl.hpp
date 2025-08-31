// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

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
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::render_backend {
////////////////////////////////////////////////////////////

class canvas_base {
public:
    virtual ~canvas_base() = default;

    virtual void flush(size_f size) = 0;
    virtual void cancel()           = 0;

    virtual void render_fill(canvas::paint const& paint, blend_funcs const& blend, canvas::scissor const& scissor, f32 fringe, vec4 const& bounds, std::vector<canvas::path> const& paths) = 0;
    virtual void render_stroke(canvas::paint const& paint, blend_funcs const& blend, canvas::scissor const& scissor, f32 fringe, f32 strokeWidth, std::vector<canvas::path> const& paths)  = 0;
    virtual void render_triangles(canvas::paint const& paint, blend_funcs const& blend, canvas::scissor const& scissor, f32 fringe, std::span<vertex const> verts)                         = 0;
    virtual void render_clip(canvas::scissor const& scissor, f32 fringe, std::vector<canvas::path> const& paths)                                                                           = 0;
    virtual void add_gradient(i32 idx, color_gradient const& gradient)                                                                                                                     = 0;
};

////////////////////////////////////////////////////////////

class render_target_base {
public:
    virtual ~render_target_base() = default;

    virtual void prepare_render(render_properties const& props) = 0;
    virtual void finalize_render() const                        = 0;
    virtual void bind_material(material const* mat) const       = 0;
    virtual void unbind_material() const                        = 0;

    virtual void enable_scissor(rect_i const& rect) const = 0; // TODO: move to render_properties
    virtual void disable_scissor() const                  = 0; // TODO: move to render_properties
    virtual void clear(color c) const                     = 0;

    virtual void on_resize(size_i size) = 0;

    virtual auto copy_to_image(rect_i const& rect) const -> image = 0;
};

////////////////////////////////////////////////////////////

class shader_base {
public:
    virtual ~shader_base() = default;

    virtual auto compile(string const& vert, string const& frag) -> bool = 0; // TODO: change to streams

    virtual auto is_valid() const -> bool = 0;
};

////////////////////////////////////////////////////////////

class texture_base {
public:
    virtual ~texture_base() = default;

    virtual void create(size_i texsize, u32 depth, texture::format format = texture::format::RGBA8) = 0;

    virtual void update(point_i origin, size_i size, void const* data, u32 depth, i32 rowLength, i32 alignment) const = 0;

    virtual auto get_filtering() const -> texture::filtering   = 0;
    virtual void set_filtering(texture::filtering props) const = 0;

    virtual auto get_wrapping() const -> texture::wrapping   = 0;
    virtual void set_wrapping(texture::wrapping props) const = 0;

    virtual auto copy_to_image(u32 depth) const -> image = 0;

    virtual auto is_valid() const -> bool = 0;
};

////////////////////////////////////////////////////////////

class uniform_buffer_base {
public:
    virtual ~uniform_buffer_base() = default;

    virtual void update(void const* data, usize size, usize offset) const = 0;

    virtual void bind_base(u32 index) const = 0;
};

////////////////////////////////////////////////////////////

class vertex_array_base {
public:
    virtual ~vertex_array_base() = default;

    virtual void resize(usize vertCount, usize indCount) = 0;

    virtual void update_data(std::span<vertex const> verts, usize vertOffset) const = 0;
    virtual void update_data(std::span<quad const> quads, usize quadOffset) const   = 0;
    virtual void update_data(std::span<u32 const> inds, usize indOffset) const      = 0;

    virtual void draw_elements(primitive_type mode, usize count, u32 offset) const = 0;
    virtual void draw_arrays(primitive_type mode, i32 first, usize count) const    = 0;
};

////////////////////////////////////////////////////////////

class window_base {
public:
    virtual ~window_base() = default;

    virtual auto get_vsync() const -> bool = 0;
    virtual void set_vsync(bool value)     = 0;

    virtual void clear(color c) const = 0;

    virtual void swap_buffer() const = 0;

    virtual void set_viewport(rect_i const& rect) = 0;

    virtual auto get_handle() const -> void* = 0;
};
}
