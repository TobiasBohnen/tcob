// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/VertexArray.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API renderer : public non_copyable {
public:
    virtual ~renderer() = default;

    void render_to_target(render_target& target, bool debug = false);

protected:
    void virtual prepare_render(render_target& target, bool debug);
    void virtual on_render_to_target(render_target& target) = 0;
    void virtual finalize_render(render_target& target);
};

////////////////////////////////////////////////////////////

class TCOB_API point_renderer final : public renderer {
public:
    explicit point_renderer(buffer_usage_hint usage);

    void set_material(assets::asset_ptr<material> material);
    void set_geometry(vertex const& v);
    void set_geometry(std::span<vertex const> vertices);
    void modify_geometry(std::span<vertex const> vertices, usize offset) const;
    void reset_geometry();

private:
    void prepare(usize vertCount);
    void on_render_to_target(render_target& target) override;

    assets::asset_ptr<material> _material {nullptr};
    usize                       _numVerts {0};

    std::unique_ptr<vertex_array> _vertexArray;
};

////////////////////////////////////////////////////////////

class TCOB_API quad_renderer final : public renderer {
public:
    explicit quad_renderer(buffer_usage_hint usage);

    void set_material(assets::asset_ptr<material> material);
    void set_geometry(quad const& q);
    void set_geometry(std::span<quad const> quads);
    void modify_geometry(std::span<quad const> quads, usize offset) const;
    void reset_geometry();

private:
    void prepare(usize quadCount);
    void on_render_to_target(render_target& target) override;

    assets::asset_ptr<material> _material {nullptr};
    usize                       _numQuads {0};

    std::unique_ptr<vertex_array> _vertexArray;
};

////////////////////////////////////////////////////////////

class TCOB_API batch_quad_renderer final : public renderer {
public:
    batch_quad_renderer();

    void prepare(usize quadCount);
    void add_geometry(quad const& q, assets::asset_ptr<material> const& mat);
    void add_geometry(std::span<quad const> quads, assets::asset_ptr<material> const& mat);

private:
    void on_render_to_target(render_target& target) override;

    std::vector<u32>  _indices;
    std::vector<quad> _quads;

    std::unique_ptr<vertex_array> _vertexArray;

    struct batch {
        assets::asset_ptr<material> MaterialPtr {nullptr};
        u32                         NumQuads {0};
        u32                         NumInds {0};
        u32                         OffsetQuads {0};
        u32                         OffsetInds {0};
    };

    batch              _currentBatch;
    std::vector<batch> _batches;
};

////////////////////////////////////////////////////////////

class TCOB_API polygon_renderer final : public renderer {
public:
    explicit polygon_renderer(buffer_usage_hint usage);

    void set_material(assets::asset_ptr<material> material);
    void set_geometry(std::span<vertex const> vertices, std::span<u32 const> indices);
    void modify_geometry(std::span<vertex const> vertices, std::span<u32 const> indices, usize offset) const;
    void reset_geometry();

private:
    void prepare(usize vcount, usize icount);
    void on_render_to_target(render_target& target) override;

    assets::asset_ptr<material> _material {nullptr};
    usize                       _numIndices {0};

    std::unique_ptr<vertex_array> _vertexArray;
};

////////////////////////////////////////////////////////////
class canvas;

class TCOB_API canvas_renderer final : public renderer {
public:
    explicit canvas_renderer(canvas& c);

    void set_bounds(rect_f const& bounds);
    void set_layer(i32 layer);

protected:
    void prepare_render(render_target& target, bool debug) override;
    void on_render_to_target(render_target& target) override;
    void finalize_render(render_target& target) override;

    std::unique_ptr<vertex_array>      _vertexArray;
    canvas&                            _canvas;
    assets::manual_asset_ptr<material> _material {};
    camera                             _oldCam;
};

}
