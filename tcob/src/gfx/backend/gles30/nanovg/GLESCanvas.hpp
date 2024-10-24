// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// based on:
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#pragma once
#include "tcob/tcob_config.hpp"

#include <glad/gles30.h>

#include "../GLESShaderProgram.hpp"
#include "../GLESTexture.hpp"
#include "../GLESVertexArray.hpp"

#include "tcob/gfx/Canvas.hpp"

namespace tcob::gfx::gles30 {

enum class nvg_shader_type : i32 { // NOLINT(performance-enum-size) part of frag shader
    Gradient    = 0,
    Image       = 1,
    StencilFill = 2,
    Triangles   = 3
};

enum class nvg_call_type : u8 {
    None = 0,
    Fill,
    ConvexFill,
    Stroke,
    Triangles,
};

struct nvg_call {
    nvg_call_type Type {0};
    blend_funcs   BlendFunc {};
    texture*      Image {};
    usize         PathOffset {0};
    usize         PathCount {0};
    usize         TriangleOffset {0};
    usize         TriangleCount {0};
    usize         UniformOffset {0};
};

struct nvg_path {
    usize FillOffset {0};
    usize FillCount {0};
    usize StrokeOffset {0};
    usize StrokeCount {0};
};

struct nvg_frag_uniforms {
    mat4 ScissorMatrix {};
    mat4 PaintMatrix {};

    vec2 ScissorExtent {0};
    vec2 ScissorScale {0};

    vec2 Extent {0};
    f32  Radius {0};
    f32  Feather {0};

    f32 StrokeMult {0};
    f32 StrokeThr {0};

    i32             TexType {0};
    nvg_shader_type Type {};

    vec4 GradientColor {0};
    f32  GradientIndex {0};
    f32  GradientAlpha {0};

    vec2 padding {};
};

class gl_canvas final : public tcob::gfx::render_backend::canvas_base {
public:
    gl_canvas();
    ~gl_canvas() override;

    void set_size(size_f size) override;
    void cancel() override;
    void flush() override;
    void render_fill(canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor, f32 fringe,
                     vec4 const& bounds, std::vector<canvas_path> const& paths) override;
    void render_stroke(canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor, f32 fringe,
                       f32 strokeWidth, std::vector<canvas_path> const& paths) override;
    void render_triangles(canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor,
                          std::span<vertex const> verts, f32 fringe) override;
    void add_gradient(i32 idx, color_gradient const& gradient) override;

private:
    void set_stencil_mask(GLuint mask);
    void set_stencil_func(GLenum func, GLint ref, GLuint mask);
    void set_blendfunc_separate(blend_funcs const& blend);
    void set_uniforms(usize uniformOffset, texture* image = nullptr) const;
    auto convert_paint(canvas_paint const& paint, canvas_scissor const& scissor, f32 width, f32 fringe, f32 strokeThr) -> nvg_frag_uniforms;
    void fill(nvg_call const& call);
    void convex_fill(nvg_call const& call);
    void stroke(nvg_call const& call);
    void triangles(nvg_call const& call);
    auto get_max_vertcount(std::vector<canvas_path> const& paths) -> usize;
    auto alloc_verts(usize n) -> usize;
    auto alloc_frag_uniforms(usize n) -> usize;
    auto get_frag_uniformptr(usize i) -> nvg_frag_uniforms*;

    gl_shader             _shader;
    size_f                _view {};
    gl_vertex_array       _vertexArray {buffer_usage_hint::StreamDraw};
    GLuint                _fragBuf {0};
    usize                 _fragSize {0};
    // Per frame buffers
    std::vector<nvg_call> _calls;
    std::vector<nvg_path> _paths;
    std::vector<vertex>   _verts;
    usize                 _nverts {0};
    std::vector<ubyte>    _uniforms;
    usize                 _nuniforms {0};
    // cached state
    GLuint                _stencilMask {0};
    GLenum                _stencilFunc {0};
    GLint                 _stencilFuncRef {0};
    GLuint                _stencilFuncMask {0};
    blend_funcs           _blendFunc {blend_func::Invalid, blend_func::Invalid, blend_func::Invalid, blend_func::Invalid};

    gl_texture _gradientTexture;
};
}
