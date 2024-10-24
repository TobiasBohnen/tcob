// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES30Canvas.hpp"

#include <cstring>
#include <variant>

#include "../GLES30.hpp"
#include "../GLES30Enum.hpp"

#include "tcob/gfx/ColorGradient.hpp"

namespace tcob::gfx::gles30 {

static char const* fillVertShader {
#include "nanovg.vert"
};
static char const* fillFragShader {
#include "nanovg.frag"
};

static u32 const GLNVG_FRAG_BINDING {0};

gl_canvas::gl_canvas()
{
    if (!_shader.compile(fillVertShader, fillFragShader)) {
        throw std::runtime_error("failed to compile nanovg shader");
    }

    // gradient
    _gradientTexture.create({color_gradient::Size, 1024}, 1, texture::format::RGBA8);
    _gradientTexture.set_wrapping(texture::wrapping::ClampToEdge);
    _shader.set_uniform(_shader.get_uniform_location("gradientTexture"), 1);

    // Create UBOs
    GLCHECK(glUniformBlockBinding(_shader.ID, glGetUniformBlockIndex(_shader.ID, "Ubo"), GLNVG_FRAG_BINDING));
    GLCHECK(glGenBuffers(1, &_fragBuf));

    i32 align {0};
    GLCHECK(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align));
    _fragSize = sizeof(nvg_frag_uniforms) + static_cast<u32>(align) - (sizeof(nvg_frag_uniforms) % static_cast<u32>(align));
}

gl_canvas::~gl_canvas()
{
    if (_fragBuf != 0) {
        GLCHECK(glDeleteBuffers(1, &_fragBuf));
    }
}

void gl_canvas::set_stencil_mask(GLuint mask)
{
    if (_stencilMask != mask) {
        _stencilMask = mask;
        GLCHECK(glStencilMask(mask));
    }
}

void gl_canvas::set_stencil_func(GLenum func, GLint ref, GLuint mask)
{
    if ((_stencilFunc != func) || (_stencilFuncRef != ref) || (_stencilFuncMask != mask)) {
        _stencilFunc     = func;
        _stencilFuncRef  = ref;
        _stencilFuncMask = mask;
        GLCHECK(glStencilFunc(func, ref, mask));
    }
}
void gl_canvas::set_blendfunc_separate(blend_funcs const& blend)
{
    if (_blendFunc != blend) {
        _blendFunc = blend;
        GLCHECK(glBlendFuncSeparate(convert_enum(blend.SourceColorBlendFunc),
                                    convert_enum(blend.DestinationColorBlendFunc),
                                    convert_enum(blend.SourceAlphaBlendFunc),
                                    convert_enum(blend.DestinationAlphaBlendFunc)));
    }
}

auto gl_canvas::convert_paint(canvas_paint const& paint, canvas_scissor const& scissor, f32 width, f32 fringe, f32 strokeThr) -> nvg_frag_uniforms
{
    nvg_frag_uniforms retValue {};

    if (auto const* arg0 {std::get_if<color>(&paint.Color)}) {
        auto const c {arg0->as_alpha_premultiplied()};
        retValue.GradientIndex = -1;
        retValue.GradientAlpha = 1;
        retValue.GradientColor = {c.R / 255.0f, c.G / 255.0f, c.B / 255.0f, c.A / 255.0f};
    } else if (auto const* arg1 {std::get_if<paint_gradient>(&paint.Color)}) {
        retValue.GradientIndex = arg1->second / (_gradientTexture.get_size().Height - 1.f);
        retValue.GradientAlpha = arg1->first;
        retValue.GradientColor = {1, 1, 1, 1};
    }

    if (scissor.Extent[0] < -0.5f || scissor.Extent[1] < -0.5f) {
        retValue.ScissorMatrix.fill(0);
        retValue.ScissorExtent[0] = 1.0f;
        retValue.ScissorExtent[1] = 1.0f;
        retValue.ScissorScale[0]  = 1.0f;
        retValue.ScissorScale[1]  = 1.0f;
    } else {
        retValue.ScissorMatrix    = scissor.XForm.as_inverted().as_matrix4();
        retValue.ScissorExtent[0] = scissor.Extent[0];
        retValue.ScissorExtent[1] = scissor.Extent[1];
        auto const& mat {scissor.XForm.Matrix};
        retValue.ScissorScale[0] = std::sqrt((mat[0] * mat[0]) + (mat[3] * mat[3])) / fringe;
        retValue.ScissorScale[1] = std::sqrt((mat[1] * mat[1]) + (mat[4] * mat[4])) / fringe;
    }

    retValue.Extent     = paint.Extent;
    retValue.StrokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
    retValue.StrokeThr  = strokeThr;

    if (paint.Image) {
        retValue.Type = nvg_shader_type::Image;

        if (paint.Image->get_format() == texture::format::R8) {
            retValue.TexType = 2;
        } else {
            retValue.TexType = 1;
        }
    } else {
        retValue.Type    = nvg_shader_type::Gradient;
        retValue.Radius  = paint.Radius;
        retValue.Feather = paint.Feather;
    }

    retValue.PaintMatrix = paint.XForm.as_inverted().as_matrix4();

    return retValue;
}

void gl_canvas::set_uniforms(usize uniformOffset, texture* image) const
{
    GLCHECK(glBindBufferRange(GL_UNIFORM_BUFFER, GLNVG_FRAG_BINDING, _fragBuf, uniformOffset, sizeof(nvg_frag_uniforms)));

    GLCHECK(glActiveTexture(GL_TEXTURE0));
    if (image) {
        GLCHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, image->get_impl<gl_texture>()->get_id()));
    } else {
        GLCHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));
    }

    GLCHECK(glActiveTexture(GL_TEXTURE1));
    GLCHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, _gradientTexture.get_id()));
}

void gl_canvas::set_size(size_f size)
{
    _view = size;
}

void gl_canvas::fill(nvg_call const& call)
{
    // Draw shapes
    glEnable(GL_STENCIL_TEST);
    set_stencil_mask(0xff);
    set_stencil_func(GL_ALWAYS, 0, 0xff);
    GLCHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

    // set bindpoint for solid loc
    set_uniforms(call.UniformOffset);

    GLCHECK(glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP));
    GLCHECK(glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP));
    GLCHECK(glDisable(GL_CULL_FACE));
    for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
        _vertexArray.draw_arrays(primitive_type::TriangleFan, static_cast<i32>(_paths[i].FillOffset), _paths[i].FillCount);
    }
    GLCHECK(glEnable(GL_CULL_FACE));

    // Draw anti-aliased pixels
    GLCHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

    set_uniforms(call.UniformOffset + _fragSize, call.Image);

    set_stencil_func(GL_EQUAL, 0x00, 0xff);
    GLCHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
    // Draw fringes
    for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
        _vertexArray.draw_arrays(primitive_type::TriangleStrip, static_cast<i32>(_paths[i].StrokeOffset), _paths[i].StrokeCount);
    }

    // Draw fill
    set_stencil_func(GL_NOTEQUAL, 0x0, 0xff);
    GLCHECK(glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO));
    _vertexArray.draw_arrays(primitive_type::TriangleStrip, static_cast<i32>(call.TriangleOffset), call.TriangleCount);

    GLCHECK(glDisable(GL_STENCIL_TEST));
}

void gl_canvas::convex_fill(nvg_call const& call)
{
    set_uniforms(call.UniformOffset, call.Image);

    for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
        _vertexArray.draw_arrays(primitive_type::TriangleFan, static_cast<i32>(_paths[i].FillOffset), _paths[i].FillCount);
        // Draw fringes
        if (_paths[i].StrokeCount > 0) {
            _vertexArray.draw_arrays(primitive_type::TriangleStrip, static_cast<i32>(_paths[i].StrokeOffset), _paths[i].StrokeCount);
        }
    }
}

void gl_canvas::stroke(nvg_call const& call)
{
    GLCHECK(glEnable(GL_STENCIL_TEST));
    set_stencil_mask(0xff);

    // Fill the stroke base without overlap
    set_stencil_func(GL_EQUAL, 0x0, 0xff);
    GLCHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_INCR));
    set_uniforms(call.UniformOffset + _fragSize, call.Image);
    for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
        _vertexArray.draw_arrays(primitive_type::TriangleStrip, static_cast<i32>(_paths[i].StrokeOffset), _paths[i].StrokeCount);
    }

    // Draw anti-aliased pixels.
    set_uniforms(call.UniformOffset, call.Image);
    set_stencil_func(GL_EQUAL, 0x00, 0xff);
    GLCHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
    for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
        _vertexArray.draw_arrays(primitive_type::TriangleStrip, static_cast<i32>(_paths[i].StrokeOffset), _paths[i].StrokeCount);
    }

    // Clear stencil buffer.
    GLCHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
    set_stencil_func(GL_ALWAYS, 0x0, 0xff);
    GLCHECK(glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO));
    for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
        _vertexArray.draw_arrays(primitive_type::TriangleStrip, static_cast<i32>(_paths[i].StrokeOffset), _paths[i].StrokeCount);
    }
    GLCHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

    GLCHECK(glDisable(GL_STENCIL_TEST));
}

void gl_canvas::triangles(nvg_call const& call)
{
    set_uniforms(call.UniformOffset, call.Image);

    _vertexArray.draw_arrays(primitive_type::Triangles, static_cast<i32>(call.TriangleOffset), call.TriangleCount);
}

void gl_canvas::cancel()
{
    _nverts = 0;
    _paths.clear();
    _calls.clear();
}

void gl_canvas::flush()
{
    if (!_calls.empty()) {
        // Setup require GL state.
        GLCHECK(glEnable(GL_CULL_FACE));
        GLCHECK(glCullFace(GL_BACK));
        GLCHECK(glFrontFace(GL_CCW));
        GLCHECK(glEnable(GL_BLEND));
        GLCHECK(glDisable(GL_DEPTH_TEST));
        GLCHECK(glDisable(GL_SCISSOR_TEST));
        GLCHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
        GLCHECK(glStencilMask(0xffffffff));
        GLCHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
        GLCHECK(glStencilFunc(GL_ALWAYS, 0, 0xffffffff));
        _stencilMask     = 0xffffffff;
        _stencilFunc     = GL_ALWAYS;
        _stencilFuncRef  = 0;
        _stencilFuncMask = 0xffffffff;
        _blendFunc       = {.SourceColorBlendFunc      = blend_func::Invalid,
                            .DestinationColorBlendFunc = blend_func::Invalid,
                            .SourceAlphaBlendFunc      = blend_func::Invalid,
                            .DestinationAlphaBlendFunc = blend_func::Invalid};

        // Upload ubo for frag shaders
        GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, _fragBuf));
        GLCHECK(glBufferData(GL_UNIFORM_BUFFER, _nuniforms * _fragSize, _uniforms.data(), GL_STREAM_DRAW));

        // Upload vertex data
        _vertexArray.resize(_nverts, 0);
        _vertexArray.update_data({_verts.data(), _nverts}, 0);

        // Set view and texture just once per frame.
        _shader.set_uniform(_shader.get_uniform_location("viewSize"), _view);
        _shader.set_uniform(_shader.get_uniform_location("texture0"), 0);

        GLCHECK(glBindBuffer(GL_UNIFORM_BUFFER, _fragBuf));

        for (auto& call : _calls) {
            set_blendfunc_separate(call.BlendFunc);
            switch (call.Type) {
            case nvg_call_type::Fill:
                fill(call);
                break;
            case nvg_call_type::ConvexFill:
                convex_fill(call);
                break;
            case nvg_call_type::Stroke:
                stroke(call);
                break;
            case nvg_call_type::Triangles:
                triangles(call);
                break;
            default:
                break;
            }
        }

        GLCHECK(glDisable(GL_CULL_FACE));
        GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLCHECK(glUseProgram(0));
    }

    // Reset calls
    _nverts = 0;

    _paths.clear();
    _calls.clear();

    _nuniforms = 0;
}

auto gl_canvas::get_max_vertcount(std::vector<canvas_path> const& paths) -> usize
{
    usize count {0};
    for (auto const& path : paths) {
        count += path.FillCount;
        count += path.StrokeCount;
    }
    return count;
}

auto gl_canvas::alloc_verts(usize n) -> usize
{
    if (_nverts + n > _verts.capacity()) {
        usize cverts {std::max<usize>(_nverts + n, 4096) + (_verts.capacity() / 2)};
        _verts.resize(cverts);
    }
    usize ret {_nverts};
    _nverts += n;
    return ret;
}

auto gl_canvas::alloc_frag_uniforms(usize n) -> usize
{
    usize ret {0}, structSize {_fragSize};
    if ((_nuniforms + n) * structSize > _uniforms.capacity()) {
        usize cuniforms {std::max<usize>(_nuniforms + n, 128) + (_uniforms.capacity() / structSize / 2)};
        _uniforms.resize(structSize * cuniforms);
    }
    ret = _nuniforms * structSize;
    _nuniforms += n;
    return ret;
}

auto gl_canvas::get_frag_uniformptr(usize i) -> nvg_frag_uniforms*
{
    ubyte* data {_uniforms.data()};
    return reinterpret_cast<nvg_frag_uniforms*>(&data[i]);
}

void gl_canvas::render_fill(canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor, f32 fringe,
                            vec4 const& bounds, std::vector<canvas_path> const& paths)
{
    auto& call {_calls.emplace_back()};
    usize pathCount {paths.size()};

    call.Type          = nvg_call_type::Fill;
    call.TriangleCount = 4;
    call.PathOffset    = _paths.size();

    call.PathCount = pathCount;
    call.Image     = paint.Image;
    call.BlendFunc = compositeOperation;

    if (pathCount == 1 && paths[0].Convex) {
        call.Type          = nvg_call_type::ConvexFill;
        call.TriangleCount = 0; // Bounding box fill quad not needed for convex fill
    }

    // Allocate vertices for all the paths.
    usize maxverts {get_max_vertcount(paths) + call.TriangleCount};
    usize offset {alloc_verts(maxverts)};

    for (auto const& path : paths) {
        nvg_path& copy {_paths.emplace_back()};
        if (path.FillCount > 0) {
            copy.FillOffset = offset;
            copy.FillCount  = path.FillCount;
            memcpy(&_verts[offset], path.Fill, sizeof(vertex) * path.FillCount);
            offset += path.FillCount;
        }
        if (path.StrokeCount > 0) {
            copy.StrokeOffset = offset;
            copy.StrokeCount  = path.StrokeCount;
            memcpy(&_verts[offset], path.Stroke, sizeof(vertex) * path.StrokeCount);
            offset += path.StrokeCount;
        }
    }

    // Setup uniforms for draw calls
    if (call.Type == nvg_call_type::Fill) {
        // Quad
        call.TriangleOffset = offset;
        vertex* quad {&_verts[call.TriangleOffset]};
        quad[0].Position  = {bounds[2], bounds[3]};
        quad[0].TexCoords = {0.5f, 1.0f, 0};
        quad[1].Position  = {bounds[2], bounds[1]};
        quad[1].TexCoords = {0.5f, 1.0f, 0};
        quad[2].Position  = {bounds[0], bounds[3]};
        quad[2].TexCoords = {0.5f, 1.0f, 0};
        quad[3].Position  = {bounds[0], bounds[1]};
        quad[3].TexCoords = {0.5f, 1.0f, 0};

        call.UniformOffset = alloc_frag_uniforms(2);

        // Simple shader for stencil
        auto* frag {get_frag_uniformptr(call.UniformOffset)};
        *frag           = {};
        frag->StrokeThr = -1.0f;
        frag->Type      = nvg_shader_type::StencilFill;

        // Fill shader
        *get_frag_uniformptr(call.UniformOffset + _fragSize) = convert_paint(paint, scissor, fringe, fringe, -1.0f);
    } else {
        call.UniformOffset = alloc_frag_uniforms(1);

        // Fill shader
        *get_frag_uniformptr(call.UniformOffset) = convert_paint(paint, scissor, fringe, fringe, -1.0f);
    }
}

void gl_canvas::render_stroke(canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor, f32 fringe,
                              f32 strokeWidth, std::vector<canvas_path> const& paths)
{
    auto& call {_calls.emplace_back()};

    call.Type       = nvg_call_type::Stroke;
    call.PathOffset = _paths.size();

    call.PathCount = paths.size();
    call.Image     = paint.Image;
    call.BlendFunc = compositeOperation;

    // Allocate vertices for all the paths.
    usize maxverts {get_max_vertcount(paths)};
    usize offset {alloc_verts(maxverts)};

    for (auto const& path : paths) {
        auto& copy {_paths.emplace_back()};

        if (path.StrokeCount) {
            copy.StrokeOffset = offset;
            copy.StrokeCount  = path.StrokeCount;
            memcpy(_verts.data() + offset, path.Stroke, sizeof(vertex) * path.StrokeCount);
            offset += path.StrokeCount;
        }
    }

    // Fill shader
    call.UniformOffset = alloc_frag_uniforms(2);

    *get_frag_uniformptr(call.UniformOffset)             = convert_paint(paint, scissor, strokeWidth, fringe, -1.0f);
    *get_frag_uniformptr(call.UniformOffset + _fragSize) = convert_paint(paint, scissor, strokeWidth, fringe, 1.0f - (0.5f / 255.0f));
}

void gl_canvas::render_triangles(canvas_paint const& paint, blend_funcs const& compositeOperation, canvas_scissor const& scissor,
                                 std::span<vertex const> verts, f32 fringe)
{
    auto& call {_calls.emplace_back()};

    call.Type      = nvg_call_type::Triangles;
    call.Image     = paint.Image;
    call.BlendFunc = compositeOperation;

    // Allocate vertices for all the paths.
    call.TriangleOffset = alloc_verts(verts.size());
    call.TriangleCount  = verts.size();

    memcpy(&_verts[call.TriangleOffset], verts.data(), verts.size_bytes());

    // Fill shader
    call.UniformOffset = alloc_frag_uniforms(1);

    auto* frag {get_frag_uniformptr(call.UniformOffset)};
    *frag      = convert_paint(paint, scissor, 1.0f, fringe, -1.0f);
    frag->Type = nvg_shader_type::Triangles;
}

void gl_canvas::add_gradient(i32 idx, color_gradient const& gradient)
{
    i32 const size {_gradientTexture.get_size().Height};
    if (idx >= size) { // grow texture
        auto const img {_gradientTexture.copy_to_image(0)};
        _gradientTexture.create({color_gradient::Size, size * 2}, 1, texture::format::RGBA8);
        _gradientTexture.set_wrapping(texture::wrapping::ClampToEdge);
        _gradientTexture.update(point_i::Zero, img.get_info().Size, img.get_data().data(), 0, color_gradient::Size, 1);
    }

    auto const colors {gradient.get_colors()};
    _gradientTexture.update({0, idx}, {color_gradient::Size, 1}, colors.data(), 0, color_gradient::Size, 1);
}

}
