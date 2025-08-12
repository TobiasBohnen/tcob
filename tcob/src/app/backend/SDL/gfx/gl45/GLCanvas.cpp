// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLCanvas.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <span>
#include <stdexcept>
#include <variant>
#include <vector>

#include "GLEnum.hpp"
#include "GLTexture.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::gl45 {

static char const* fillVertShader {
#include "./shaders/nanovg.vert"
};
static char const* fillFragShader {
#include "./shaders/nanovg.frag"
};

static u32 const GLNVG_FRAG_BINDING {0};

gl_canvas::gl_canvas()
{
    if (!_shader.compile(fillVertShader, fillFragShader)) {
        throw std::runtime_error("Failed to compile nanovg shader");
    }
    _shader.set_uniform(_shader.get_uniform_location("texture0"), 0);

    // gradient
    _gradientTexture.create({color_gradient::Size, 1024}, 1, texture::format::RGBA8);
    _gradientTexture.set_wrapping(texture::wrapping::ClampToEdge);
    _shader.set_uniform(_shader.get_uniform_location("gradientTexture"), 1);

    // Create UBOs
    glCreateBuffers(1, &_fragBuf);

    i32 align {0};
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
    _fragSize = sizeof(nvg_frag_uniforms) + static_cast<u32>(align) - (sizeof(nvg_frag_uniforms) % static_cast<u32>(align));
}

gl_canvas::~gl_canvas()
{
    if (_fragBuf != 0) {
        glDeleteBuffers(1, &_fragBuf);
    }
}

void gl_canvas::flush(size_f size)
{
    if (!_calls.empty()) {
        auto const clearStencil {[] {
            glStencilMask(0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glStencilFunc(GL_ALWAYS, 0, 0xFF);
            glClearStencil(0x80);
            glClear(GL_STENCIL_BUFFER_BIT);
        }};

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        clearStencil();

        glUseProgram(_shader.ID);

        // Upload ubo for frag shaders
        glNamedBufferData(_fragBuf, _nuniforms * _fragSize, _uniforms.data(), GL_STREAM_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _fragBuf);

        // Upload vertex data
        _vertexArray.resize(_nverts, 0);
        _vertexArray.update_data({_verts.data(), _nverts}, 0);

        // Set view and texture just once per frame.
        _shader.set_uniform(_shader.get_uniform_location("viewSize"), size);

        glBindVertexArray(_vertexArray.ID);

        for (auto& call : _calls) {
            glBlendFuncSeparate(convert_enum(call.BlendFunc.SourceColorBlendFunc),
                                convert_enum(call.BlendFunc.DestinationColorBlendFunc),
                                convert_enum(call.BlendFunc.SourceAlphaBlendFunc),
                                convert_enum(call.BlendFunc.DestinationAlphaBlendFunc));

            switch (call.Type) {
            case nvg_call_type::Fill:       fill(call); break;
            case nvg_call_type::ConvexFill: convex_fill(call); break;
            case nvg_call_type::Stroke:     stroke(call); break;
            case nvg_call_type::Triangles:  triangles(call); break;
            case nvg_call_type::Clip:       clip(call); break;
            case nvg_call_type::ClearClip:  clearStencil(); break;
            default:
                break;
            }
        }

        glBindVertexArray(0);
        glDisable(GL_CULL_FACE);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    }

    // Reset calls
    _nverts = 0;

    _paths.clear();
    _calls.clear();

    _nuniforms = 0;
}

void gl_canvas::cancel()
{
    _nverts = 0;
    _paths.clear();
    _calls.clear();
}

void gl_canvas::render_fill(canvas::paint const& paint, blend_funcs const& blend, canvas::scissor const& scissor, f32 fringe,
                            vec4 const& bounds, std::vector<canvas::path> const& paths)
{
    auto& call {_calls.emplace_back()};
    usize pathCount {paths.size()};

    call.PathOffset = _paths.size();
    call.PathCount  = pathCount;
    call.Image      = paint.Image;
    call.BlendFunc  = blend;

    if (pathCount == 1 && paths[0].Convex) {
        call.Type          = nvg_call_type::ConvexFill;
        call.TriangleCount = 0; // Bounding box fill quad not needed for convex fill
    } else {
        call.Type          = nvg_call_type::Fill;
        call.TriangleCount = 4;
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
        quad[0].TexCoords = {.U = 0.5f, .V = 1.0f, .Level = 0};
        quad[1].Position  = {bounds[2], bounds[1]};
        quad[1].TexCoords = {.U = 0.5f, .V = 1.0f, .Level = 0};
        quad[2].Position  = {bounds[0], bounds[3]};
        quad[2].TexCoords = {.U = 0.5f, .V = 1.0f, .Level = 0};
        quad[3].Position  = {bounds[0], bounds[1]};
        quad[3].TexCoords = {.U = 0.5f, .V = 1.0f, .Level = 0};

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

void gl_canvas::render_stroke(canvas::paint const& paint, blend_funcs const& blend, canvas::scissor const& scissor, f32 fringe,
                              f32 strokeWidth, std::vector<canvas::path> const& paths)
{
    auto& call {_calls.emplace_back()};

    call.Type       = nvg_call_type::Stroke;
    call.PathOffset = _paths.size();

    call.PathCount = paths.size();
    call.Image     = paint.Image;
    call.BlendFunc = blend;

    // Allocate vertices for all the paths.
    usize const maxverts {get_max_vertcount(paths)};
    usize       offset {alloc_verts(maxverts)};

    for (auto const& path : paths) {
        auto& copy {_paths.emplace_back()};

        if (path.StrokeCount > 0) {
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

void gl_canvas::render_triangles(canvas::paint const& paint, blend_funcs const& blend, canvas::scissor const& scissor, f32 fringe,
                                 std::span<vertex const> verts)
{
    auto& call {_calls.emplace_back()};

    call.Type      = nvg_call_type::Triangles;
    call.Image     = paint.Image;
    call.BlendFunc = blend;

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

void gl_canvas::render_clip(canvas::scissor const& scissor, f32 fringe, std::vector<canvas::path> const& paths)
{
    auto& call {_calls.emplace_back()};
    if (paths.empty()) {
        call.Type = nvg_call_type::ClearClip;
        return;
    }

    usize const pathCount {paths.size()};

    call.PathOffset    = _paths.size();
    call.PathCount     = pathCount;
    call.Image         = nullptr;
    call.Type          = nvg_call_type::Clip;
    call.TriangleCount = 0;

    // Allocate vertices.
    usize const maxverts {get_max_vertcount(paths) + call.TriangleCount};
    usize       offset {alloc_verts(maxverts)};

    for (auto const& path : paths) {
        nvg_path& copy {_paths.emplace_back()};
        if (path.FillCount > 0) {
            copy.FillOffset = offset;
            copy.FillCount  = path.FillCount;
            memcpy(&_verts[offset], path.Fill, sizeof(vertex) * path.FillCount);
            offset += path.FillCount;
        }
    }

    call.UniformOffset = alloc_frag_uniforms(1);
    auto* frag {get_frag_uniformptr(call.UniformOffset)};
    *frag           = convert_paint({}, scissor, 1.0f, fringe, -1.0f);
    frag->StrokeThr = -1.0f;
    frag->Type      = nvg_shader_type::StencilFill;
}

void gl_canvas::add_gradient(i32 idx, color_gradient const& gradient)
{
    i32 const size {_gradientTexture.get_size().Height};
    if (idx >= size) { // grow texture
        auto const img {_gradientTexture.copy_to_image(0)};
        _gradientTexture.create({color_gradient::Size, size * 2}, 1, texture::format::RGBA8);
        _gradientTexture.set_wrapping(texture::wrapping::ClampToEdge);
        _gradientTexture.update(point_i::Zero, img.info().Size, img.data().data(), 0, color_gradient::Size, 1);
    }

    auto const colors {gradient.colors()};
    _gradientTexture.update({0, idx}, {color_gradient::Size, 1}, colors.data(), 0, color_gradient::Size, 1);
}

void gl_canvas::set_uniforms(usize uniformOffset, texture* image) const
{
    glBindBufferRange(GL_UNIFORM_BUFFER, GLNVG_FRAG_BINDING, _fragBuf, uniformOffset, sizeof(nvg_frag_uniforms));

    if (image) {
        glBindTextureUnit(0, image->get_impl<gl_texture>()->ID);
    } else {
        glBindTextureUnit(0, 0);
    }

    glBindTextureUnit(1, _gradientTexture.ID);
}

auto gl_canvas::convert_paint(canvas::paint const& paint, canvas::scissor const& scissor, f32 width, f32 fringe, f32 strokeThr) -> nvg_frag_uniforms
{
    nvg_frag_uniforms retValue {};

    if (auto const* arg0 {std::get_if<color>(&paint.Color)}) {
        auto const c {arg0->as_alpha_premultiplied()};
        retValue.GradientIndex = -1;
        retValue.GradientAlpha = 1;
        retValue.GradientColor = {c.R / 255.0f, c.G / 255.0f, c.B / 255.0f, c.A / 255.0f};
    } else if (auto const* arg1 {std::get_if<paint_gradient>(&paint.Color)}) {
        retValue.GradientIndex = static_cast<f32>(arg1->second) / static_cast<f32>(_gradientTexture.get_size().Height - 1);
        retValue.GradientAlpha = arg1->first;
        retValue.GradientColor = {1, 1, 1, 1};
    }

    if (scissor.Extent.Width < -0.5f || scissor.Extent.Height < -0.5f) {
        retValue.ScissorMatrix.fill(0);
        retValue.ScissorExtent[0] = 1.0f;
        retValue.ScissorExtent[1] = 1.0f;
        retValue.ScissorScale[0]  = 1.0f;
        retValue.ScissorScale[1]  = 1.0f;
    } else {
        retValue.ScissorMatrix = scissor.XForm.as_inverted().as_matrix4();
        retValue.ScissorExtent = scissor.Extent.to_array();
        auto const& mat {scissor.XForm.Matrix};
        retValue.ScissorScale[0] = std::sqrt((mat[0] * mat[0]) + (mat[3] * mat[3])) / fringe;
        retValue.ScissorScale[1] = std::sqrt((mat[1] * mat[1]) + (mat[4] * mat[4])) / fringe;
    }

    retValue.Extent     = paint.Extent.to_array();
    retValue.StrokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
    retValue.StrokeThr  = strokeThr;

    if (paint.Image) {
        retValue.Type = nvg_shader_type::Image;

        if (paint.Image->info().Format == texture::format::R8) {
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

void gl_canvas::fill(nvg_call const& call)
{
    glEnable(GL_STENCIL_TEST);

    // --- Pass 1: Mark winding regions in stencil buffer ---
    // Use lower 7 bits for winding count (0x7F mask), keep high bit (0x80) for clip.
    glStencilMask(0x7F);
    glStencilFunc(GL_EQUAL, 0x80, 0x80); // Only affect pixels already inside clip (0x80).
    // Increment winding for front faces, decrement for back faces.
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);

    // No color output during stencil fill.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_CULL_FACE);

    // Draw filled paths into stencil buffer to count coverage.
    set_uniforms(call.UniformOffset);
    {
        std::vector<GLint> fillFirsts, fillCounts;
        fillFirsts.reserve(call.PathCount);
        fillCounts.reserve(call.PathCount);
        for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
            fillFirsts.push_back(static_cast<GLint>(_paths[i].FillOffset));
            fillCounts.push_back(static_cast<GLsizei>(_paths[i].FillCount));
        }
        glMultiDrawArrays(GL_TRIANGLE_FAN, fillFirsts.data(), fillCounts.data(),
                          static_cast<GLsizei>(fillFirsts.size()));
    }

    glEnable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // --- Pass 2: Stroke shapes (while stencil == clip bit) ---
    // Lock stencil writes for color pass.
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0x80, 0xFF); // Clip bit must match; ignore winding bits.
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Draw strokes over the filled shapes.
    set_uniforms(call.UniformOffset + _fragSize, call.Image);
    {
        std::vector<GLint> strokeFirsts, strokeCounts;
        strokeFirsts.reserve(call.PathCount);
        strokeCounts.reserve(call.PathCount);
        for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
            strokeFirsts.push_back(static_cast<GLint>(_paths[i].StrokeOffset));
            strokeCounts.push_back(static_cast<GLsizei>(_paths[i].StrokeCount));
        }
        glMultiDrawArrays(GL_TRIANGLE_STRIP, strokeFirsts.data(), strokeCounts.data(),
                          static_cast<GLsizei>(strokeFirsts.size()));
    }

    // --- Pass 3: Fill actual pixels where winding != 0 ---
    glStencilMask(0x7F);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0x7F); // Inside shape (non-zero winding count).
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO); // Clear winding after drawing.
    glDrawArrays(GL_TRIANGLE_STRIP,
                 static_cast<GLint>(call.TriangleOffset),
                 static_cast<GLsizei>(call.TriangleCount));

    glDisable(GL_STENCIL_TEST);
}

void gl_canvas::convex_fill(nvg_call const& call)
{
    glEnable(GL_STENCIL_TEST);

    // No stencil writes; just test against clip bit.
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0x80, 0x80);

    // Directly draw colors (convex fill doesn't require winding).
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    set_uniforms(call.UniformOffset, call.Image);

    // Fill convex polygons.
    {
        std::vector<GLint> fillFirsts, fillCounts;
        fillFirsts.reserve(call.PathCount);
        fillCounts.reserve(call.PathCount);
        for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
            fillFirsts.push_back(static_cast<GLint>(_paths[i].FillOffset));
            fillCounts.push_back(static_cast<GLsizei>(_paths[i].FillCount));
        }
        glMultiDrawArrays(GL_TRIANGLE_FAN, fillFirsts.data(), fillCounts.data(),
                          static_cast<GLsizei>(fillFirsts.size()));
    }

    // Optional stroke pass (if present).
    {
        std::vector<GLint> strokeFirsts, strokeCounts;
        strokeFirsts.reserve(call.PathCount);
        strokeCounts.reserve(call.PathCount);
        for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
            if (_paths[i].StrokeCount > 0) {
                strokeFirsts.push_back(static_cast<GLint>(_paths[i].StrokeOffset));
                strokeCounts.push_back(static_cast<GLsizei>(_paths[i].StrokeCount));
            }
        }
        if (!strokeFirsts.empty()) {
            glMultiDrawArrays(GL_TRIANGLE_STRIP, strokeFirsts.data(), strokeCounts.data(),
                              static_cast<GLsizei>(strokeFirsts.size()));
        }
    }

    glDisable(GL_STENCIL_TEST);
}

void gl_canvas::stroke(nvg_call const& call)
{
    glEnable(GL_STENCIL_TEST);

    // Use lower 7 bits for stroke mask; preserve clip in high bit.
    glStencilMask(0x7F);

    // Collect stroke geometry.
    std::vector<GLint> strokeFirsts, strokeCounts;
    strokeFirsts.reserve(call.PathCount);
    strokeCounts.reserve(call.PathCount);
    for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
        strokeFirsts.push_back(static_cast<GLint>(_paths[i].StrokeOffset));
        strokeCounts.push_back(static_cast<GLsizei>(_paths[i].StrokeCount));
    }

    // --- Pass 1: Mark stroke coverage in stencil ---
    glStencilFunc(GL_EQUAL, 0x80, 0x80);    // Only inside clip.
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); // Increase coverage count in stencil.
    set_uniforms(call.UniformOffset + _fragSize, call.Image);
    glMultiDrawArrays(GL_TRIANGLE_STRIP, strokeFirsts.data(), strokeCounts.data(),
                      static_cast<GLsizei>(strokeFirsts.size()));

    // --- Pass 2: Draw stroke color (still respecting clip) ---
    glStencilFunc(GL_EQUAL, 0x80, 0x80);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // Don't modify stencil during color draw.
    set_uniforms(call.UniformOffset, call.Image);
    glMultiDrawArrays(GL_TRIANGLE_STRIP, strokeFirsts.data(), strokeCounts.data(),
                      static_cast<GLsizei>(strokeFirsts.size()));

    // --- Pass 3: Cleanup stencil (erase stroke mask) ---
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // No color output.
    glStencilFunc(GL_GREATER, 0x80, 0xFF);               // Any coverage count above clip-only.
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);              // Reset stencil to 0 in these areas.
    glMultiDrawArrays(GL_TRIANGLE_STRIP, strokeFirsts.data(), strokeCounts.data(),
                      static_cast<GLsizei>(strokeFirsts.size()));
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glDisable(GL_STENCIL_TEST);
}

void gl_canvas::triangles(nvg_call const& call)
{
    set_uniforms(call.UniformOffset, call.Image);

    // Enable stencil test so triangles are rasterized only where clip wrote 0x80
    glEnable(GL_STENCIL_TEST);

    // Pass only where stencil == 0x80 (mask 0xFF). Do not write to stencil.
    glStencilFunc(GL_EQUAL, 0x80, 0xFF);
    glStencilMask(0x00); // prevent modifications of stencil
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glDrawArrays(GL_TRIANGLES,
                 static_cast<GLint>(call.TriangleOffset),
                 static_cast<GLsizei>(call.TriangleCount));

    // Restore stencil state (disable if you don't need it afterwards)
    glDisable(GL_STENCIL_TEST);
}

void gl_canvas::clip(nvg_call const& call)
{
    glEnable(GL_STENCIL_TEST);

    // Allow writing to all stencil bits.
    glStencilMask(0xFF);

    // Always pass stencil test; write reference value 0x80 (clip bit set).
    // Mask = 0xFF means all bits are affected.
    glStencilFunc(GL_ALWAYS, 0x80, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); // Replace stencil with ref on any pass/fail.

    // Clear stencil to 0 before defining clip region.
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    // Disable color writes; we only want to update stencil here.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // Disable face culling to ensure clip path is drawn regardless of winding.
    glDisable(GL_CULL_FACE);

    // Use uniforms for this draw (no image — just clip path).
    set_uniforms(call.UniformOffset);

    // Draw filled paths to set stencil = 0x80 inside clip region.
    {
        std::vector<GLint> fillFirsts, fillCounts;
        fillFirsts.reserve(call.PathCount);
        fillCounts.reserve(call.PathCount);
        for (usize i {call.PathOffset}; i < call.PathOffset + call.PathCount; ++i) {
            fillFirsts.push_back(static_cast<GLint>(_paths[i].FillOffset));
            fillCounts.push_back(static_cast<GLsizei>(_paths[i].FillCount));
        }
        glMultiDrawArrays(GL_TRIANGLE_FAN, fillFirsts.data(), fillCounts.data(),
                          static_cast<GLsizei>(fillFirsts.size()));
    }

    // Restore normal raster state.
    glEnable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Disable stencil test for subsequent draws unless explicitly needed.
    glDisable(GL_STENCIL_TEST);
}

auto gl_canvas::get_max_vertcount(std::vector<canvas::path> const& paths) -> usize
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
    byte* data {_uniforms.data()};
    return reinterpret_cast<nvg_frag_uniforms*>(&data[i]);
}

}
