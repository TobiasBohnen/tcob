// Copyright (c) 2021 Tobias Bohnen
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
#include <glad/gl.h>

#include <tcob/core/data/Color.hpp>
#include <tcob/gfx/Canvas.hpp>
#include <tcob/gfx/gl/GLShaderProgram.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>
#include <tcob/gfx/gl/GLVertexArray.hpp>
#include <tcob/tcob_config.hpp>

namespace tcob::detail {

struct GLNVGblend {
    GLenum srcRGB { 0 };
    GLenum dstRGB { 0 };
    GLenum srcAlpha { 0 };
    GLenum dstAlpha { 0 };
};

struct GLNVGcall {
    i32 type { 0 };
    gl::Texture2D* image;
    isize pathOffset { 0 };
    isize pathCount { 0 };
    isize triangleOffset { 0 };
    isize triangleCount { 0 };
    isize uniformOffset { 0 };
    GLNVGblend blendFunc;
};

struct GLNVGpath {
    isize fillOffset { 0 };
    isize fillCount { 0 };
    isize strokeOffset { 0 };
    isize strokeCount { 0 };
};

struct GLNVGfragUniforms {
    mat4 scissorMat {};
    mat4 paintMat {};
    vec4 text_outline_color {};
    std::array<vec4, 256> gradient;
    vec2 scissorExt {};
    vec2 scissorScale {};
    vec2 extent {};
    float textOutlineThickness { 0 };
    f32 radius { 0 };
    f32 feather { 0 };
    f32 strokeMult { 0 };
    f32 strokeThr { 0 };
    i32 texType { 0 };
    i32 type { 0 };
    bool isSingleColor { false };
};

class GLNVGcontext final {
public:
    GLNVGcontext();
    ~GLNVGcontext();

    void set_viewport(SizeF size);
    void cancel();
    void flush();
    void render_fill(const CanvasPaint& paint, NVGcompositeOperationState compositeOperation, const NVGscissor& scissor, f32 fringe,
        const vec4& bounds, const std::vector<NVGpath>& paths);
    void render_stroke(const CanvasPaint& paint, NVGcompositeOperationState compositeOperation, const NVGscissor& scissor, f32 fringe,
        f32 strokeWidth, const std::vector<NVGpath>& paths);
    void render_triangles(const CanvasPaint& paint, NVGcompositeOperationState compositeOperation, const NVGscissor& scissor,
        const Vertex* verts, i32 nverts);

private:
    void set_stencil_mask(GLuint mask);
    void set_stencil_func(GLenum func, GLint ref, GLuint mask);
    void set_blendfunc_separate(const GLNVGblend& blend);
    void set_uniforms(isize uniformOffset);
    void set_uniforms(isize uniformOffset, gl::Texture2D* image);
    void xform_to_mat3x4(mat3x4& m3, const mat2x3& t);
    auto convert_paint(GLNVGfragUniforms* frag, const CanvasPaint& paint, const NVGscissor& scissor, f32 width, f32 fringe, f32 strokeThr) -> bool;
    void fill(const GLNVGcall& call);
    void convex_fill(const GLNVGcall& call);
    void stroke(const GLNVGcall& call);
    void triangles(const GLNVGcall& call);
    auto get_blend_composite_operation(const NVGcompositeOperationState& op) -> GLNVGblend;
    auto get_max_vertcount(const std::vector<NVGpath>& paths) -> isize;
    auto alloc_verts(isize n) -> isize;
    auto alloc_frag_uniforms(isize n) -> isize;
    auto get_frag_uniformptr(isize i) -> GLNVGfragUniforms*;

    gl::ShaderProgram _shader;

    SizeF _view {};

    gl::VertexArray _vertexArray;

    GLuint _fragBuf { 0 };
    isize _fragSize { 0 };

    // Per frame buffers
    std::vector<GLNVGcall> _calls;
    std::vector<GLNVGpath> _paths;

    std::vector<Vertex> _verts;
    isize _nverts { 0 };

    std::vector<ubyte> _uniforms;
    isize _nuniforms { 0 };

    // cached state
    GLuint _stencilMask { 0 };
    GLenum _stencilFunc { 0 };
    GLint _stencilFuncRef { 0 };
    GLuint _stencilFuncMask { 0 };
    GLNVGblend _blendFunc;
};
}