// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Renderer.hpp"

#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/RenderTexture.hpp"

namespace tcob::gfx {

void renderer::render_to_target(render_target& target, bool debug)
{
    prepare_render(target, debug);
    on_render_to_target(target);
    finalize_render(target);
}

void renderer::prepare_render(render_target& target, bool debug)
{
    target.prepare_render(debug);
}

void renderer::finalize_render(render_target& target)
{
    target.finalize_render();
}

////////////////////////////////////////////////////////////

point_renderer::point_renderer(buffer_usage_hint usage)
    : _vertexArray {std::make_unique<vertex_array>(usage)}
{
}

void point_renderer::set_material(material const* material)
{
    _material = material;
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

    target.bind_material(_material);
    _vertexArray->draw_arrays(primitive_type::Points, 0, _numVerts);
    target.unbind_material();
}

////////////////////////////////////////////////////////////

quad_renderer::quad_renderer(buffer_usage_hint usage)
    : _vertexArray {std::make_unique<vertex_array>(usage)}
{
}

void quad_renderer::set_material(material const* material)
{
    _material = material;
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

        std::vector<u32> inds(indCount);
        for (u32 i {0}, j {0}; i < quadCount; ++i, j += 4) {
            inds[(i * 6) + 0] = 3 + j;
            inds[(i * 6) + 1] = 1 + j;
            inds[(i * 6) + 2] = 0 + j;
            inds[(i * 6) + 3] = 3 + j;
            inds[(i * 6) + 4] = 2 + j;
            inds[(i * 6) + 5] = 1 + j;
        }

        _vertexArray->update_data(inds, 0);
    }
}

void quad_renderer::on_render_to_target(render_target& target)
{
    if (_numQuads == 0 || !_material) {
        return;
    }

    target.bind_material(_material);
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

void batch_quad_renderer::add_geometry(quad const& q, material const* mat)
{
    add_geometry(std::span {&q, 1}, mat);
}

void batch_quad_renderer::add_geometry(std::span<quad const> quads, material const* mat)
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
        ptr[_currentBatch.NumInds++] = (3 + j);
        ptr[_currentBatch.NumInds++] = (1 + j);
        ptr[_currentBatch.NumInds++] = (0 + j);
        ptr[_currentBatch.NumInds++] = (3 + j);
        ptr[_currentBatch.NumInds++] = (2 + j);
        ptr[_currentBatch.NumInds++] = (1 + j);
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

        target.bind_material(batch.MaterialPtr);
        _vertexArray->draw_elements(primitive_type::Triangles, batch.NumInds, batch.OffsetInds);
    }

    target.unbind_material();
}

////////////////////////////////////////////////////////////

polygon_renderer::polygon_renderer(buffer_usage_hint usage)
    : _vertexArray {std::make_unique<vertex_array>(usage)}
{
}

void polygon_renderer::set_material(material const* material)
{
    _material = material;
}

void polygon_renderer::set_geometry(geometry_data const& gd)
{
    prepare(gd.Vertices.size(), gd.Indices.size());
    modify_geometry(gd, 0);
    _numIndices = gd.Indices.size();
    _numVerts   = gd.Vertices.size();
}

void polygon_renderer::modify_geometry(geometry_data const& gd, usize offset)
{
    _vertexArray->update_data(gd.Indices, offset);
    _vertexArray->update_data(gd.Vertices, offset);
    _type = gd.Type;
}

void polygon_renderer::reset_geometry()
{
    _numIndices = 0;
    _numVerts   = 0;
}

void polygon_renderer::prepare(usize vcount, usize icount)
{
    if (vcount > _numVerts || icount > _numIndices) {
        _vertexArray->resize(vcount, icount);
    }
}

void polygon_renderer::on_render_to_target(render_target& target)
{
    if (_numIndices == 0 || _numVerts == 0 || !_material) {
        return;
    }

    target.bind_material(_material);
    _vertexArray->draw_elements(_type, _numIndices, 0);
    target.unbind_material();
}

////////////////////////////////////////////////////////////

batch_polygon_renderer::batch_polygon_renderer()
    : _vertexArray {std::make_unique<vertex_array>(buffer_usage_hint::DynamicDraw)}
{
}

void batch_polygon_renderer::add_geometry(geometry_data const& gd, material const* mat)
{
    // check if we have to break the batch
    if (_currentBatch.NumInds > 0 && (_currentBatch.Type != gd.Type || *_currentBatch.MaterialPtr != *mat)) {
        _batches.push_back(_currentBatch);
        _currentBatch.OffsetInds += _currentBatch.NumInds;
        _currentBatch.OffsetVerts += _currentBatch.NumVerts;
        _currentBatch.NumInds  = 0;
        _currentBatch.NumVerts = 0;
    }

    _currentBatch.MaterialPtr = mat;
    _currentBatch.Type        = gd.Type;

    // copy indices
    if (!_verts.empty()) {
        _indices.reserve(_indices.size() + gd.Indices.size());
        for (auto const& ind : gd.Indices) {
            _indices.push_back(static_cast<u32>(ind + _verts.size()));
        }
    } else {
        _indices.insert(_indices.end(), gd.Indices.begin(), gd.Indices.end());
    }
    _currentBatch.NumInds += static_cast<u32>(gd.Indices.size());

    // copy vertices
    _verts.insert(_verts.end(), gd.Vertices.begin(), gd.Vertices.end());
    _currentBatch.NumVerts += static_cast<u32>(gd.Vertices.size());

    _vertexArray->resize(_verts.size(), _indices.size());
}

void batch_polygon_renderer::reset_geometry()
{
    _batches.clear();
    _currentBatch = {};

    _verts.clear();
    _indices.clear();
}

void batch_polygon_renderer::on_render_to_target(render_target& target)
{
    if (_currentBatch.NumVerts == 0 && _batches.empty()) { // nothing to draw
        return;
    }

    if (_currentBatch.NumVerts > 0) { // push current batch
        _batches.push_back(_currentBatch);
        _currentBatch = {};

        _vertexArray->update_data(_indices, 0);
        _vertexArray->update_data(_verts, 0);
    }

    for (auto const& batch : _batches) { // draw batches
        if (batch.NumVerts == 0 || !batch.MaterialPtr) {
            continue;
        }

        target.bind_material(batch.MaterialPtr);
        _vertexArray->draw_elements(primitive_type::Triangles, batch.NumInds, batch.OffsetInds);
    }

    target.unbind_material();
}

////////////////////////////////////////////////////////////

canvas_renderer::canvas_renderer(canvas& c)
    : _vertexArray {std::make_unique<vertex_array>(buffer_usage_hint::StaticDraw)}
    , _canvas {c}
{
    usize const vertCount {4};
    usize const indCount {6};

    _vertexArray->resize(vertCount, indCount);

    std::array<u32, 6> inds {3, 1, 0, 3, 2, 1};
    _vertexArray->update_data(inds, 0);
}

void canvas_renderer::set_bounds(rect_f const& bounds)
{
    quad q {};
    geometry::set_position(q, bounds);
    geometry::set_color(q, colors::White);
    geometry::set_texcoords(q, {.UVRect = render_texture::GetTexcoords(), .Level = 0});
    _vertexArray->update_data(std::span {&q, 1}, 0);
}

void canvas_renderer::set_layer(i32 layer)
{
    _material->Texture = _canvas.get_texture(layer);
}

void canvas_renderer::prepare_render(render_target& target, bool debug)
{
    target.get_camera().push_state();
    target.prepare_render(debug);
}

void canvas_renderer::on_render_to_target(render_target& target)
{
    target.bind_material(_material.get_ptr());
    _vertexArray->draw_elements(primitive_type::Triangles, 6, 0);
    target.unbind_material();
}

void canvas_renderer::finalize_render(render_target& target)
{
    target.get_camera().pop_state();
    target.finalize_render();
}

}
