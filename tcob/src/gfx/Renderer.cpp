// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Renderer.hpp"

#include <span>
#include <utility>
#include <vector>

#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/RenderTexture.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/VertexArray.hpp"

namespace tcob::gfx {

renderer_base::renderer_base()
    : _stats {locate_service<render_system>().statistics()}
{
}

void renderer_base::render_to_target(render_target& target, bool prepare)
{
    if (prepare) { prepare_render(target); }
    on_render_to_target(target);
    if (prepare) { finalize_render(target); }
}

void renderer_base::prepare_render(render_target& target)
{
    target.prepare_render({.ViewMatrix            = target.camera().matrix(),
                           .Viewport              = target.camera().viewport(),
                           .MousePosition         = locate_service<input::system>().mouse().get_position(),
                           .Time                  = _stats.current_time(),
                           .Debug                 = false, // TODO
                           .UseDefaultFramebuffer = false});
}

void renderer_base::finalize_render(render_target& target)
{
    target.finalize_render();
}

////////////////////////////////////////////////////////////

renderer::renderer(buffer_usage_hint usage)
    : _vertexArray {usage}
{
}

void renderer::set_geometry(std::span<vertex const> vertices, pass const* pass)
{
    set_geometry(geometry_data {.Vertices = vertices, .Indices = {}, .Type = primitive_type::Points}, pass);
}

void renderer::set_geometry(std::span<quad const> quads, pass const* pass)
{
    set_geometry(geometry_data {.Vertices = geometry::flatten(quads),
                                .Indices  = geometry::get_indices(quads.size()),
                                .Type     = primitive_type::Triangles},
                 pass);
}

void renderer::set_geometry(geometry_data const& gd, pass const* pass)
{
    usize const vCount {gd.Vertices.size()};
    usize const iCount {gd.Indices.size()};

    if (vCount > _numVerts || iCount > _numIndices) {
        _vertexArray.resize(vCount, iCount);
    }

    _vertexArray.update_data(gd.Vertices, 0);
    _vertexArray.update_data(gd.Indices, 0);
    _type = gd.Type;

    _numVerts   = vCount;
    _numIndices = iCount;

    _pass = pass;
}

void renderer::reset_geometry()
{
    _numIndices = 0;
    _numVerts   = 0;
}

void renderer::on_render_to_target(render_target& target)
{
    if (_numVerts == 0 || !_pass) { return; }

    target.bind_pass(*_pass);
    if (_numIndices == 0) {
        _vertexArray.draw_arrays(_type, 0, _numVerts);
    } else {
        _vertexArray.draw_elements(_type, _numIndices, 0);
    }

    target.unbind_pass();
}

////////////////////////////////////////////////////////////

batch_renderer::batch_renderer()
    : _vertexArray {buffer_usage_hint::StreamDraw}
{
}

void batch_renderer::add_geometry(geometry_data const& gd, pass const* pass)
{
    if (gd.Vertices.empty()) { return; }

    // check if we have to break the batch
    if (!_currentBatch.is_empty() && (_currentBatch.Type != gd.Type || *_currentBatch.Pass != *pass)) {
        _batches.push_back(_currentBatch);
        _currentBatch.OffsetInds += _currentBatch.NumInds;
        _currentBatch.OffsetVerts += _currentBatch.NumVerts;
        _currentBatch.NumInds  = 0;
        _currentBatch.NumVerts = 0;
    }

    _currentBatch.Pass = pass;
    _currentBatch.Type = gd.Type;

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

    _vertexArray.resize(_verts.size(), _indices.size());
}

void batch_renderer::reset_geometry()
{
    _batches.clear();
    _currentBatch = {};

    _verts.clear();
    _indices.clear();
}

void batch_renderer::on_render_to_target(render_target& target)
{
    if (_currentBatch.is_empty() && _batches.empty()) { // nothing to draw
        return;
    }

    // geometry was changed -> update vertex arrays
    if (!_currentBatch.is_empty()) {
        _batches.push_back(_currentBatch);
        _currentBatch = {};

        _vertexArray.update_data(_indices, 0);
        _vertexArray.update_data(_verts, 0);
    }

    for (auto const& batch : _batches) { // draw batches
        if (batch.NumVerts == 0 || !batch.Pass) { continue; }

        target.bind_pass(*batch.Pass);
        _vertexArray.draw_elements(primitive_type::Triangles, batch.NumInds, batch.OffsetInds);
        target.unbind_pass();
    }
}

////////////////////////////////////////////////////////////

canvas_renderer::canvas_renderer(canvas& c)
    : _vertexArray {buffer_usage_hint::StaticDraw}
    , _canvas {c}
{
    usize const vertCount {4};
    usize const indCount {6};

    _vertexArray.resize(vertCount, indCount);
    _vertexArray.update_data(QuadIndicies, 0);
}

void canvas_renderer::set_bounds(rect_f const& bounds)
{
    quad q {};
    geometry::set_position(q, bounds);
    geometry::set_color(q, colors::White);
    geometry::set_texcoords(q, {.UVRect = render_texture::UVRect(), .Level = 0});
    _vertexArray.update_data(std::span {&q, 1}, 0);
}

void canvas_renderer::queue_layer(i32 layer)
{
    _layers.push_back(layer);
}

void canvas_renderer::set_shader(asset_ptr<shader> shader)
{
    _material->first_pass().Shader = std::move(shader);
}

void canvas_renderer::prepare_render(render_target& target)
{
    target.camera().push_state();
    renderer_base::prepare_render(target);
}

void canvas_renderer::on_render_to_target(render_target& target)
{
    for (auto layer : _layers) {
        _material->first_pass().Texture = _canvas.get_texture(layer);
        target.bind_pass(_material->first_pass());
        _vertexArray.draw_elements(primitive_type::Triangles, 6, 0);
    }
    target.unbind_pass();
    _layers.clear();
}

void canvas_renderer::finalize_render(render_target& target)
{
    target.camera().pop_state();
    target.finalize_render();
}

}
