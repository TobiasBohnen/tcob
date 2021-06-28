// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLNVGcontext.hpp"

#include <cstring>

namespace tcob::detail {

enum GLNVGshaderType {
    NSVG_SHADER_FILLGRAD,
    NSVG_SHADER_FILLIMG,
    NSVG_SHADER_SIMPLE,
    NSVG_SHADER_IMG
};

enum GLNVGuniformBindings {
    GLNVG_FRAG_BINDING = 0,
};

enum GLNVGcallType {
    GLNVG_NONE = 0,
    GLNVG_FILL,
    GLNVG_CONVEXFILL,
    GLNVG_STROKE,
    GLNVG_TRIANGLES,
};

GLNVGcontext::GLNVGcontext()
{
    i32 align { 0 };

    static const char* fillVertShader {
#include "nanovg.vert"
    };
    static const char* fillFragShader {
#include "nanovg.frag"
    };

    if (!_shader.create(fillVertShader, fillFragShader))
        throw std::runtime_error("failed to compile nanovg shader");
    _shader.set_uniform("texture0", 0);

    // Create UBOs
    glUniformBlockBinding(_shader.ID, 0, GLNVG_FRAG_BINDING);
    glGenBuffers(1, &_fragBuf);

    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
    _fragSize = sizeof(GLNVGfragUniforms) + align - sizeof(GLNVGfragUniforms) % align;
}

GLNVGcontext::~GLNVGcontext()
{
    if (_fragBuf != 0)
        glDeleteBuffers(1, &_fragBuf);
}

void GLNVGcontext::set_stencil_mask(GLuint mask)
{
    if (_stencilMask != mask) {
        _stencilMask = mask;
        glStencilMask(mask);
    }
}

void GLNVGcontext::set_stencil_func(GLenum func, GLint ref, GLuint mask)
{
    if ((_stencilFunc != func) || (_stencilFuncRef != ref) || (_stencilFuncMask != mask)) {
        _stencilFunc = func;
        _stencilFuncRef = ref;
        _stencilFuncMask = mask;
        glStencilFunc(func, ref, mask);
    }
}
void GLNVGcontext::set_blendfunc_separate(const GLNVGblend& blend)
{
    if ((_blendFunc.srcRGB != blend.srcRGB) || (_blendFunc.dstRGB != blend.dstRGB) || (_blendFunc.srcAlpha != blend.srcAlpha) || (_blendFunc.dstAlpha != blend.dstAlpha)) {
        _blendFunc = blend;
        glBlendFuncSeparate(blend.srcRGB, blend.dstRGB, blend.srcAlpha, blend.dstAlpha);
    }
}

void GLNVGcontext::xform_to_mat3x4(mat3x4& m3, const mat2x3& t)
{
    m3[0] = t[0];
    m3[1] = t[1];
    m3[2] = 0.0f;
    m3[3] = 0.0f;
    m3[4] = t[2];
    m3[5] = t[3];
    m3[6] = 0.0f;
    m3[7] = 0.0f;
    m3[8] = t[4];
    m3[9] = t[5];
    m3[10] = 1.0f;
    m3[11] = 0.0f;
}

auto GLNVGcontext::convert_paint(GLNVGfragUniforms* frag, const CanvasPaint& paint,
    const NVGscissor& scissor, f32 width, f32 fringe, f32 strokeThr) -> bool
{
    Transform invxform;
    memset(frag, 0, sizeof(*frag));

    frag->gradient = paint.gradient.colors();
    frag->isSingleColor = paint.gradient.is_single_color();

    if (scissor.extent[0] < -0.5f || scissor.extent[1] < -0.5f) {
        frag->scissorMat.fill(0);
        frag->scissorExt[0] = 1.0f;
        frag->scissorExt[1] = 1.0f;
        frag->scissorScale[0] = 1.0f;
        frag->scissorScale[1] = 1.0f;
    } else {
        invxform = scissor.xform.inverse();
        frag->scissorMat = invxform.matrix4();
        frag->scissorExt[0] = scissor.extent[0];
        frag->scissorExt[1] = scissor.extent[1];
        auto mat { scissor.xform.matrix3() };
        frag->scissorScale[0] = std::sqrt(mat[0] * mat[0] + mat[3] * mat[3]) / fringe;
        frag->scissorScale[1] = std::sqrt(mat[1] * mat[1] + mat[4] * mat[4]) / fringe;
    }

    frag->extent = paint.extent;
    frag->strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
    frag->strokeThr = strokeThr;

    if (paint.image) {
        invxform = paint.xform.inverse();
        frag->type = NSVG_SHADER_FILLIMG;

        if (paint.image->format() == gl::TextureFormat::RGBA8)
            frag->texType = 1;
        else
            frag->texType = 2;
    } else {
        frag->type = NSVG_SHADER_FILLGRAD;
        frag->radius = paint.radius;
        frag->feather = paint.feather;
        invxform = paint.xform.inverse();
    }

    frag->paintMat = invxform.matrix4();

    frag->text_outline_color = {
        paint.text_outline_color.R / 255.f,
        paint.text_outline_color.G / 255.f,
        paint.text_outline_color.B / 255.f,
        paint.text_outline_color.A / 255.f
    };
    frag->textOutlineThickness = (1 - paint.text_outline_thickness) * 0.5f;

    return true;
}

void GLNVGcontext::set_uniforms(isize uniformOffset)
{
    glBindBufferRange(GL_UNIFORM_BUFFER, GLNVG_FRAG_BINDING, _fragBuf, uniformOffset, sizeof(GLNVGfragUniforms));
}

void GLNVGcontext::set_uniforms(isize uniformOffset, gl::Texture2D* image)
{
    set_uniforms(uniformOffset);

    if (image) {
        image->bind_texture_unit();
    } else {
        glBindTextureUnit(0, 0);
    }
}

void GLNVGcontext::set_viewport(SizeF size)
{
    _view = size;
}

void GLNVGcontext::fill(const GLNVGcall& call)
{
    // Draw shapes
    glEnable(GL_STENCIL_TEST);
    set_stencil_mask(0xff);
    set_stencil_func(GL_ALWAYS, 0, 0xff);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // set bindpoint for solid loc
    set_uniforms(call.uniformOffset);

    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    glDisable(GL_CULL_FACE);
    for (isize i { call.pathOffset }; i < call.pathOffset + call.pathCount; ++i)
        _vertexArray.draw_arrays(GL_TRIANGLE_FAN, static_cast<i32>(_paths[i].fillOffset), _paths[i].fillCount);
    glEnable(GL_CULL_FACE);

    // Draw anti-aliased pixels
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    set_uniforms(call.uniformOffset + _fragSize, call.image);

    set_stencil_func(GL_EQUAL, 0x00, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    // Draw fringes
    for (isize i { call.pathOffset }; i < call.pathOffset + call.pathCount; ++i)
        _vertexArray.draw_arrays(GL_TRIANGLE_STRIP, static_cast<i32>(_paths[i].strokeOffset), _paths[i].strokeCount);

    // Draw fill
    set_stencil_func(GL_NOTEQUAL, 0x0, 0xff);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    _vertexArray.draw_arrays(GL_TRIANGLE_STRIP, static_cast<i32>(call.triangleOffset), call.triangleCount);

    glDisable(GL_STENCIL_TEST);
}

void GLNVGcontext::convex_fill(const GLNVGcall& call)
{
    set_uniforms(call.uniformOffset, call.image);

    for (isize i { call.pathOffset }; i < call.pathOffset + call.pathCount; ++i) {
        _vertexArray.draw_arrays(GL_TRIANGLE_FAN, static_cast<i32>(_paths[i].fillOffset), _paths[i].fillCount);
        // Draw fringes
        if (_paths[i].strokeCount > 0) {
            _vertexArray.draw_arrays(GL_TRIANGLE_STRIP, static_cast<i32>(_paths[i].strokeOffset), _paths[i].strokeCount);
        }
    }
}

void GLNVGcontext::stroke(const GLNVGcall& call)
{
    glEnable(GL_STENCIL_TEST);
    set_stencil_mask(0xff);

    // Fill the stroke base without overlap
    set_stencil_func(GL_EQUAL, 0x0, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    set_uniforms(call.uniformOffset + _fragSize, call.image);
    for (isize i { call.pathOffset }; i < call.pathOffset + call.pathCount; ++i)
        _vertexArray.draw_arrays(GL_TRIANGLE_STRIP, static_cast<i32>(_paths[i].strokeOffset), _paths[i].strokeCount);

    // Draw anti-aliased pixels.
    set_uniforms(call.uniformOffset, call.image);
    set_stencil_func(GL_EQUAL, 0x00, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    for (isize i { call.pathOffset }; i < call.pathOffset + call.pathCount; ++i)
        _vertexArray.draw_arrays(GL_TRIANGLE_STRIP, static_cast<i32>(_paths[i].strokeOffset), _paths[i].strokeCount);

    // Clear stencil buffer.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    set_stencil_func(GL_ALWAYS, 0x0, 0xff);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    for (isize i { call.pathOffset }; i < call.pathOffset + call.pathCount; ++i)
        _vertexArray.draw_arrays(GL_TRIANGLE_STRIP, static_cast<i32>(_paths[i].strokeOffset), _paths[i].strokeCount);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glDisable(GL_STENCIL_TEST);
}

void GLNVGcontext::triangles(const GLNVGcall& call)
{
    set_uniforms(call.uniformOffset, call.image);

    _vertexArray.draw_arrays(GL_TRIANGLES, static_cast<i32>(call.triangleOffset), call.triangleCount);
}

void GLNVGcontext::cancel()
{
    _nverts = 0;
    _paths.clear();
    _calls.clear();
}

auto GLNVGcontext::get_blend_composite_operation(const gl::BlendFuncs& op) -> GLNVGblend
{
    GLNVGblend blend {
        .srcRGB = gl::convert_enum(op.SourceColorBlendFunc),
        .dstRGB = gl::convert_enum(op.DestinationColorBlendFunc),
        .srcAlpha = gl::convert_enum(op.SourceAlphaBlendFunc),
        .dstAlpha = gl::convert_enum(op.DestinationAlphaBlendFunc),
    };
    if (blend.srcRGB == GL_INVALID_ENUM || blend.dstRGB == GL_INVALID_ENUM || blend.srcAlpha == GL_INVALID_ENUM || blend.dstAlpha == GL_INVALID_ENUM) {
        blend.srcRGB = GL_ONE;
        blend.dstRGB = GL_ONE_MINUS_SRC_ALPHA;
        blend.srcAlpha = GL_ONE;
        blend.dstAlpha = GL_ONE_MINUS_SRC_ALPHA;
    }
    return blend;
}

void GLNVGcontext::flush()
{
    if (!_calls.empty()) {
        // Setup require GL state.
        _shader.use();
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glStencilMask(0xffffffff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
        _stencilMask = 0xffffffff;
        _stencilFunc = GL_ALWAYS;
        _stencilFuncRef = 0;
        _stencilFuncMask = 0xffffffff;
        _blendFunc.srcRGB = GL_INVALID_ENUM;
        _blendFunc.srcAlpha = GL_INVALID_ENUM;
        _blendFunc.dstRGB = GL_INVALID_ENUM;
        _blendFunc.dstAlpha = GL_INVALID_ENUM;

        // Upload ubo for frag shaders
        glBindBuffer(GL_UNIFORM_BUFFER, _fragBuf);
        glBufferData(GL_UNIFORM_BUFFER, _nuniforms * _fragSize, _uniforms.data(), GL_STREAM_DRAW);

        // Upload vertex data
        _vertexArray.resize(_nverts, 0, tcob::gl::BufferUsage::StreamDraw);
        _vertexArray.update(_verts.data(), _nverts, 0);

        // Set view and texture just once per frame.
        _shader.set_uniform("viewSize", _view);

        glBindBuffer(GL_UNIFORM_BUFFER, _fragBuf);

        for (auto& call : _calls) {
            set_blendfunc_separate(call.blendFunc);
            if (call.type == GLNVG_FILL)
                fill(call);
            else if (call.type == GLNVG_CONVEXFILL)
                convex_fill(call);
            else if (call.type == GLNVG_STROKE)
                stroke(call);
            else if (call.type == GLNVG_TRIANGLES)
                triangles(call);
        }

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

auto GLNVGcontext::get_max_vertcount(const std::vector<NVGpath>& paths) -> isize
{
    isize count { 0 };
    for (const auto& path : paths) {
        count += path.nfill;
        count += path.nstroke;
    }
    return count;
}

auto GLNVGcontext::alloc_verts(isize n) -> isize
{
    isize ret { 0 };
    if (_nverts + n > _verts.capacity()) {
        isize cverts { std::max<isize>(_nverts + n, 4096) + _verts.capacity() / 2 };
        _verts.reserve(cverts);
    }
    ret = _nverts;
    _nverts += n;
    return ret;
}

auto GLNVGcontext::alloc_frag_uniforms(isize n) -> isize
{
    isize ret { 0 }, structSize { _fragSize };
    if ((_nuniforms + n) * structSize > _uniforms.capacity()) {
        isize cuniforms { std::max<isize>(_nuniforms + n, 128) + _uniforms.capacity() / 2 };
        _uniforms.reserve(structSize * cuniforms);
    }
    ret = _nuniforms * structSize;
    _nuniforms += n;
    return ret;
}

auto GLNVGcontext::get_frag_uniformptr(isize i) -> GLNVGfragUniforms*
{
    ubyte* data { _uniforms.data() };
    return reinterpret_cast<GLNVGfragUniforms*>(&data[i]);
}

void GLNVGcontext::render_fill(const CanvasPaint& paint, const gl::BlendFuncs& compositeOperation, const NVGscissor& scissor, f32 fringe,
    const vec4& bounds, const std::vector<NVGpath>& paths)
{
    GLNVGcall& call { _calls.emplace_back() };
    GLNVGfragUniforms* frag { nullptr };
    i32 npaths { static_cast<i32>(paths.size()) };

    call.type = GLNVG_FILL;
    call.triangleCount = 4;
    call.pathOffset = _paths.size();

    call.pathCount = npaths;
    call.image = paint.image;
    call.blendFunc = get_blend_composite_operation(compositeOperation);

    if (npaths == 1 && paths[0].convex) {
        call.type = GLNVG_CONVEXFILL;
        call.triangleCount = 0; // Bounding box fill quad not needed for convex fill
    }

    // Allocate vertices for all the paths.
    isize maxverts { get_max_vertcount(paths) + call.triangleCount };
    isize offset { alloc_verts(maxverts) };

    for (const auto& path : paths) {
        GLNVGpath& copy { _paths.emplace_back() };
        if (path.nfill > 0) {
            copy.fillOffset = offset;
            copy.fillCount = path.nfill;
            memcpy(&_verts.data()[offset], path.fill, sizeof(Vertex) * path.nfill);
            offset += path.nfill;
        }
        if (path.nstroke > 0) {
            copy.strokeOffset = offset;
            copy.strokeCount = path.nstroke;
            memcpy(&_verts.data()[offset], path.stroke, sizeof(Vertex) * path.nstroke);
            offset += path.nstroke;
        }
    }

    // Setup uniforms for draw calls
    if (call.type == GLNVG_FILL) {
        // Quad
        call.triangleOffset = offset;
        Vertex* quad { &_verts.data()[call.triangleOffset] };
        quad[0].Position = { bounds[2], bounds[3] };
        quad[0].TexCoords = { 0.5f, 1.0f, 0 };
        quad[1].Position = { bounds[2], bounds[1] };
        quad[1].TexCoords = { 0.5f, 1.0f, 0 };
        quad[2].Position = { bounds[0], bounds[3] };
        quad[2].TexCoords = { 0.5f, 1.0f, 0 };
        quad[3].Position = { bounds[0], bounds[1] };
        quad[3].TexCoords = { 0.5f, 1.0f, 0 };

        call.uniformOffset = alloc_frag_uniforms(2);

        // Simple shader for stencil
        frag = get_frag_uniformptr(call.uniformOffset);
        memset(frag, 0, sizeof(*frag));
        frag->strokeThr = -1.0f;
        frag->type = NSVG_SHADER_SIMPLE;

        // Fill shader
        convert_paint(get_frag_uniformptr(call.uniformOffset + _fragSize), paint, scissor, fringe, fringe, -1.0f);
    } else {
        call.uniformOffset = alloc_frag_uniforms(1);

        // Fill shader
        convert_paint(get_frag_uniformptr(call.uniformOffset), paint, scissor, fringe, fringe, -1.0f);
    }
}

void GLNVGcontext::render_stroke(const CanvasPaint& paint, const gl::BlendFuncs& compositeOperation, const NVGscissor& scissor, f32 fringe,
    f32 strokeWidth, const std::vector<NVGpath>& paths)
{
    GLNVGcall& call { _calls.emplace_back() };
    isize npaths { paths.size() };

    call.type = GLNVG_STROKE;
    call.pathOffset = _paths.size();

    call.pathCount = npaths;
    call.image = paint.image;
    call.blendFunc = get_blend_composite_operation(compositeOperation);

    // Allocate vertices for all the paths.
    isize maxverts { get_max_vertcount(paths) };
    isize offset { alloc_verts(maxverts) };

    for (isize i { 0 }; i < npaths; ++i) {
        GLNVGpath& copy { _paths.emplace_back() };
        const NVGpath path { paths[i] };
        if (path.nstroke) {
            copy.strokeOffset = offset;
            copy.strokeCount = path.nstroke;
            memcpy(&_verts.data()[offset], path.stroke, sizeof(Vertex) * path.nstroke);
            offset += path.nstroke;
        }
    }

    // Fill shader
    call.uniformOffset = alloc_frag_uniforms(2);

    convert_paint(get_frag_uniformptr(call.uniformOffset), paint, scissor, strokeWidth, fringe, -1.0f);
    convert_paint(get_frag_uniformptr(call.uniformOffset + _fragSize), paint, scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f);
}

void GLNVGcontext::render_triangles(const CanvasPaint& paint, const gl::BlendFuncs& compositeOperation, const NVGscissor& scissor,
    const Vertex* verts, i32 nverts)
{
    GLNVGcall& call { _calls.emplace_back() };

    call.type = GLNVG_TRIANGLES;
    call.image = paint.image;
    call.blendFunc = get_blend_composite_operation(compositeOperation);

    // Allocate vertices for all the paths.
    call.triangleOffset = alloc_verts(nverts);

    call.triangleCount = nverts;

    memcpy(&_verts.data()[call.triangleOffset], verts, sizeof(Vertex) * nverts);

    // Fill shader
    call.uniformOffset = alloc_frag_uniforms(1);

    GLNVGfragUniforms* frag { get_frag_uniformptr(call.uniformOffset) };
    convert_paint(frag, paint, scissor, 1.0f, 1.0f, -1.0f);
    frag->type = NSVG_SHADER_IMG;
}
}