// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLRenderer.hpp>

#include <cstring>

#include <glad/gl.h>

#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/gl/GLRenderTarget.hpp>
#include <tcob/gfx/gl/GLShaderProgram.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>

namespace tcob::gl {
void Renderer::render_to_target(RenderTarget& target, bool debug)
{
    target.setup_render(debug);

    // set polygon mode
    if (debug) {
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glEnable(GL_LINE_SMOOTH);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    draw();

    // disable and unbind
    glDisable(GL_BLEND);

    target.finish_render();
}

void Renderer::bind_material(Material* mat)
{
    if (!mat) {
        return;
    }

    if (mat->Texture) {
        auto* tex { mat->Texture.object() };
        if (tex->ID) {
            tex->bind_texture_unit(0);
        }
    }

    if (mat->Shader) {
        auto* shd { mat->Shader.object() };
        if (shd->ID) {
            shd->use();
        }
    }

    // set blend mode
    glEnable(GL_BLEND);
    glBlendFuncSeparate(
        convert_enum(mat->BlendFuncs.SourceColorBlendFunc), convert_enum(mat->BlendFuncs.DestinationColorBlendFunc),
        convert_enum(mat->BlendFuncs.SourceAlphaBlendFunc), convert_enum(mat->BlendFuncs.DestinationAlphaBlendFunc));
    glBlendEquation(convert_enum(mat->BlendEquation));
}

////////////////////////////////////////////////////////////

void StaticQuadRenderer::prepare(isize quadCount)
{
    _vertexArray.resize(static_cast<isize>(quadCount) * 4, static_cast<isize>(quadCount) * 6, gl::BufferUsage::StaticDraw);
}

void StaticQuadRenderer::set_geometry(const Quad* quads, isize count)
{
    prepare(count);

    //copy vertices
    _vertexArray.update(quads, count, 0);

    //copy indices
    std::vector<u32> inds;
    inds.reserve(count * 6);
    for (u32 i { 0 }, j { 0 }; i < count; ++i, j += 4) {
        inds.push_back(0 + j);
        inds.push_back(1 + j);
        inds.push_back(3 + j);
        inds.push_back(1 + j);
        inds.push_back(2 + j);
        inds.push_back(3 + j);
    }

    _vertexArray.update(inds.data(), inds.size(), 0);

    _numQuads = count;
}

void StaticQuadRenderer::modify_geometry(const Quad* quads, isize count, isize offset) const
{
    _vertexArray.update(quads, count, offset * 4);
}

void StaticQuadRenderer::material(Material* mat)
{
    _mat = mat;
}

void StaticQuadRenderer::draw()
{
    if (_numQuads == 0)
        return;

    bind_material(_mat);

    _vertexArray.draw_elements(GL_TRIANGLES, _numQuads * 6, 0);

    //unbind
    glBindTextureUnit(0, 0);
    glUseProgram(0);
}

////////////////////////////////////////////////////////////

void DynamicQuadRenderer::prepare(isize quadCount)
{
    if (quadCount > _numQuads) {
        isize vertCount { quadCount * 4 };
        isize indCount { quadCount * 6 };

        _vertexArray.resize(vertCount, indCount, gl::BufferUsage::DynamicDraw);

        //copy indices
        std::vector<u32> inds;
        inds.reserve(indCount);
        for (u32 i { 0 }, j { 0 }; i < quadCount; ++i, j += 4) {
            inds.push_back(0 + j);
            inds.push_back(1 + j);
            inds.push_back(3 + j);
            inds.push_back(1 + j);
            inds.push_back(2 + j);
            inds.push_back(3 + j);
        }

        _vertexArray.update(inds.data(), inds.size(), 0);
    }

    _numQuads = quadCount;
}

void DynamicQuadRenderer::material(Material* mat)
{
    _mat = mat;
}

void DynamicQuadRenderer::set_geometry(const Quad* vertices, isize count)
{
    prepare(count);

    //copy vertices
    _vertexArray.update(vertices, count, 0);

    _numQuads = count;
}

auto DynamicQuadRenderer::map(isize wantCount) -> Quad*
{
    prepare(wantCount);
    return static_cast<Quad*>(_vertexArray.map_vertexbuffer());
}

void DynamicQuadRenderer::unmap(isize haveCount)
{
    _numQuads = haveCount;
    _vertexArray.unmap_vertexbuffer();
}

void DynamicQuadRenderer::reset()
{
    _numQuads = 0;
}

void DynamicQuadRenderer::draw()
{
    if (_numQuads == 0)
        return;

    bind_material(_mat);

    _vertexArray.draw_elements(GL_TRIANGLES, _numQuads * 6, 0);

    //unbind
    glBindTextureUnit(0, 0);
    glUseProgram(0);
}

////////////////////////////////////////////////////////////

void BatchQuadRenderer::prepare(isize quadCount)
{
    isize vertCount { quadCount * 4 };
    isize indCount { quadCount * 6 };

    _vertexArray.resize(vertCount, indCount, gl::BufferUsage::StreamDraw);

    _indices.reserve(indCount);
    _vertices.reserve(vertCount);
    _batches.clear();
    _currentBatch = {};
}

void BatchQuadRenderer::add_quads(const Quad* quads, u32 quadCount, Material* mat)
{
    //TODO: add VertexArray size check

    //check if we have to break the batch
    if (_currentBatch.NumInds > 0 && *_currentBatch.MaterialPtr != *mat) {
        _batches.push_back(_currentBatch);
        _currentBatch.OffsetInds += _currentBatch.NumInds;
        _currentBatch.OffsetVerts += _currentBatch.NumVerts;
        _currentBatch.NumInds = 0;
        _currentBatch.NumVerts = 0;
    }

    _currentBatch.MaterialPtr = mat;

    //copy vertices
    memcpy(&_vertices.data()[_currentBatch.NumVerts + _currentBatch.OffsetVerts], quads, quadCount * sizeof(Quad));

    //copy indices
    GLuint* ptr { &_indices.data()[_currentBatch.OffsetInds] };

    for (u32 i { 0 }, j = _currentBatch.NumVerts + _currentBatch.OffsetVerts; i < quadCount; ++i, j += 4) {
        ptr[_currentBatch.NumInds++] = (0 + j);
        ptr[_currentBatch.NumInds++] = (1 + j);
        ptr[_currentBatch.NumInds++] = (3 + j);
        ptr[_currentBatch.NumInds++] = (1 + j);
        ptr[_currentBatch.NumInds++] = (2 + j);
        ptr[_currentBatch.NumInds++] = (3 + j);
        _currentBatch.NumVerts += 4;
    }
}

void BatchQuadRenderer::draw()
{
    if (_currentBatch.NumVerts == 0 && _batches.size() == 0)
        return;

    _batches.push_back(_currentBatch);
    _currentBatch = {};

    _vertexArray.update(_indices.data(), _indices.capacity(), 0);
    _vertexArray.update(_vertices.data(), _vertices.capacity(), 0);

    for (const Batch& batch : _batches) {
        if (batch.NumVerts == 0)
            continue;

        bind_material(batch.MaterialPtr);

        _vertexArray.draw_elements(GL_TRIANGLES, batch.NumInds, batch.OffsetInds);
    }

    //unbind
    glBindTextureUnit(0, 0);
    glUseProgram(0);
}

////////////////////////////////////////////////////////////

void StaticPointRenderer::prepare(isize vertCount)
{
    _vertexArray.resize(static_cast<isize>(vertCount), 0, gl::BufferUsage::StaticDraw);
}

void StaticPointRenderer::set_geometry(const Vertex* vertices, isize count)
{
    prepare(count);

    //copy vertices
    _vertexArray.update(vertices, count, 0);

    _numVerts = count;
}

void StaticPointRenderer::modify_geometry(const Vertex* vertices, isize count, isize offset) const
{
    _vertexArray.update(vertices, count, offset);
}

void StaticPointRenderer::material(Material* mat, f32 size)
{
    _mat = mat;
    _pointSize = size;
}

void StaticPointRenderer::draw()
{
    if (_numVerts == 0)
        return;

    bind_material(_mat);
    // glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(_pointSize);
    _vertexArray.draw_arrays(GL_POINTS, 0, _numVerts);
    glPointSize(1);

    //unbind
    glBindTextureUnit(0, 0);
    glUseProgram(0);
}

////////////////////////////////////////////////////////////

void StreamPointRenderer::prepare(isize vertCount)
{
    _vertexArray.resize(static_cast<isize>(vertCount), 0, gl::BufferUsage::StreamDraw);
}

void StreamPointRenderer::set_geometry(const Vertex* vertices, isize count)
{
    prepare(count);

    //copy vertices
    _vertexArray.update(vertices, count, 0);

    _numVerts = count;
}

void StreamPointRenderer::material(Material* mat, f32 size)
{
    _mat = mat;
    _pointSize = size;
}

void StreamPointRenderer::draw()
{
    if (_numVerts == 0)
        return;

    bind_material(_mat);
    // glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(_pointSize);
    _vertexArray.draw_arrays(GL_POINTS, 0, _numVerts);
    glPointSize(1);

    //unbind
    glBindTextureUnit(0, 0);
    glUseProgram(0);
}
}