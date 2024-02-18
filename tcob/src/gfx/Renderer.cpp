// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Renderer.hpp"

#include <utility>

namespace tcob::gfx {

void renderer::render_to_target(render_target& target, bool debug)
{
    target.prepare_render(debug);
    on_render_to_target(target);
    target.finalize_render();
}

////////////////////////////////////////////////////////////

point_renderer::point_renderer(buffer_usage_hint usage)
    : _vertexArray {std::make_unique<vertex_array>(usage)}
{
}

void point_renderer::set_material(assets::asset_ptr<material> material)
{
    _material = std::move(material);
}

void point_renderer::set_geometry(vertex const& v)
{
    set_geometry(std::span {&v, 1});
}

void point_renderer::set_geometry(std::span<vertex const> vertices)
{
    prepare(vertices.size());
    modify_geometry(vertices, 0);
}

void point_renderer::modify_geometry(std::span<vertex const> vertices, usize offset) const
{
    _vertexArray->update_data(vertices, offset);
}

void point_renderer::reset_geometry()
{
    _numVerts = 0;
}

void point_renderer::prepare(usize vertCount)
{
    if (vertCount > _numVerts) {
        _vertexArray->resize(vertCount, 0);
    }
    _numVerts = vertCount;
}

void point_renderer::on_render_to_target(render_target& target)
{
    if (_numVerts == 0 || !_material) {
        return;
    }

    target.bind_material(_material.get_obj());
    _vertexArray->draw_arrays(primitive_type::Points, 0, _numVerts);
    target.unbind_material();
}

////////////////////////////////////////////////////////////

quad_renderer::quad_renderer(buffer_usage_hint usage)
    : _vertexArray {std::make_unique<vertex_array>(usage)}
{
}

void quad_renderer::set_material(assets::asset_ptr<material> material)
{
    _material = std::move(material);
}

void quad_renderer::set_geometry(quad const& q)
{
    set_geometry(std::span {&q, 1});
    _numQuads = 1;
}

void quad_renderer::set_geometry(std::span<quad const> quads)
{
    prepare(quads.size());
    modify_geometry(quads, 0);
    _numQuads = quads.size();
}

void quad_renderer::modify_geometry(std::span<quad const> quads, usize offset) const
{
    _vertexArray->update_data(quads, offset);
}

void quad_renderer::reset_geometry()
{
    _numQuads = 0;
}

void quad_renderer::prepare(usize quadCount)
{
    if (quadCount > _numQuads) {
        usize const vertCount {quadCount * 4};
        usize const indCount {quadCount * 6};

        _vertexArray->resize(vertCount, indCount);

        std::vector<u32> inds;
        inds.resize(indCount);
        for (u32 i {0}, j {0}; i < quadCount; ++i, j += 4) {
            inds[i * 6 + 0] = 0 + j;
            inds[i * 6 + 1] = 1 + j;
            inds[i * 6 + 2] = 3 + j;
            inds[i * 6 + 3] = 1 + j;
            inds[i * 6 + 4] = 2 + j;
            inds[i * 6 + 5] = 3 + j;
        }

        _vertexArray->update_data(inds, 0);
    }
}

void quad_renderer::on_render_to_target(render_target& target)
{
    if (_numQuads == 0 || !_material) {
        return;
    }

    target.bind_material(_material.get_obj());
    _vertexArray->draw_elements(primitive_type::Triangles, _numQuads * 6, 0);
    target.unbind_material();
}

////////////////////////////////////////////////////////////

batch_quad_renderer::batch_quad_renderer()
    : _vertexArray {std::make_unique<vertex_array>(buffer_usage_hint::StreamDraw)}
{
}

void batch_quad_renderer::prepare(usize quadCount)
{
    usize const vertCount {quadCount * 4};
    usize const indCount {quadCount * 6};

    _vertexArray->resize(vertCount, indCount);

    _indices.resize(indCount);
    _quads.resize(quadCount);
    _batches.clear();
    _currentBatch = {};
}

void batch_quad_renderer::add_geometry(quad const& q, assets::asset_ptr<material> const& mat)
{
    add_geometry(std::span {&q, 1}, mat);
}

void batch_quad_renderer::add_geometry(std::span<quad const> quads, assets::asset_ptr<material> const& mat)
{
    // TODO: add VertexArray size check

    // check if we have to break the batch
    if (_currentBatch.NumInds > 0 && *_currentBatch.MaterialPtr != *mat) {
        _batches.push_back(_currentBatch);
        _currentBatch.OffsetInds += _currentBatch.NumInds;
        _currentBatch.OffsetQuads += _currentBatch.NumQuads;
        _currentBatch.NumInds  = 0;
        _currentBatch.NumQuads = 0;
    }

    _currentBatch.MaterialPtr = mat;

    std::copy(quads.begin(), quads.end(), _quads.begin() + _currentBatch.NumQuads + _currentBatch.OffsetQuads);

    u32* ptr {&_indices[_currentBatch.OffsetInds]};

    for (u32 i {0}, j {(_currentBatch.NumQuads + _currentBatch.OffsetQuads) * 4}; i < quads.size(); ++i, j += 4) {
        ptr[_currentBatch.NumInds++] = (0 + j);
        ptr[_currentBatch.NumInds++] = (1 + j);
        ptr[_currentBatch.NumInds++] = (3 + j);
        ptr[_currentBatch.NumInds++] = (1 + j);
        ptr[_currentBatch.NumInds++] = (2 + j);
        ptr[_currentBatch.NumInds++] = (3 + j);
        _currentBatch.NumQuads++;
    }
}

void batch_quad_renderer::on_render_to_target(render_target& target)
{
    if (_currentBatch.NumQuads == 0 && _batches.empty()) { // nothing to draw
        return;
    }

    if (_currentBatch.NumQuads > 0) { // push current batch
        _batches.push_back(_currentBatch);
        _currentBatch = {};

        _vertexArray->update_data(_indices, 0);
        _vertexArray->update_data(_quads, 0);
    }

    for (auto const& batch : _batches) { // draw batches
        if (batch.NumQuads == 0 || !batch.MaterialPtr) {
            continue;
        }

        target.bind_material(batch.MaterialPtr.get_obj());
        _vertexArray->draw_elements(primitive_type::Triangles, batch.NumInds, batch.OffsetInds);
    }

    target.unbind_material();
}

////////////////////////////////////////////////////////////

polygon_renderer::polygon_renderer(buffer_usage_hint usage)
    : _vertexArray {std::make_unique<vertex_array>(usage)}
{
}

void polygon_renderer::set_material(assets::asset_ptr<material> material)
{
    _material = std::move(material);
}

void polygon_renderer::set_geometry(std::span<vertex const> vertices, std::span<u32 const> indices)
{
    prepare(vertices.size(), indices.size());
    modify_geometry(vertices, indices, 0);
    _numIndices = indices.size();
}

void polygon_renderer::modify_geometry(std::span<vertex const> vertices, std::span<u32 const> indices, usize offset) const
{
    _vertexArray->update_data(indices, offset);
    _vertexArray->update_data(vertices, offset);
}

void polygon_renderer::reset_geometry()
{
    _numIndices = 0;
}

void polygon_renderer::prepare(usize vcount, usize icount)
{
    if (icount > _numIndices) {
        _vertexArray->resize(vcount, icount);
    }
}

void polygon_renderer::on_render_to_target(render_target& target)
{
    if (_numIndices == 0 || !_material) {
        return;
    }

    target.bind_material(_material.get_obj());
    _vertexArray->draw_elements(primitive_type::Triangles, _numIndices, 0);
    target.unbind_material();
}

}
