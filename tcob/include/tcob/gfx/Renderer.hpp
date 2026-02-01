// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Stats.hpp"
#include "tcob/gfx/VertexArray.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API renderer_base : public non_copyable {
public:
    renderer_base();
    virtual ~renderer_base() = default;

    void render_to_target(render_target& target, bool prepare = true);

protected:
    virtual void prepare_render(render_target& target);
    virtual void on_render_to_target(render_target& target) = 0;
    virtual void finalize_render(render_target& target);

    render_statistics& _stats;
};

////////////////////////////////////////////////////////////

class TCOB_API renderer final : public renderer_base {
public:
    explicit renderer(buffer_usage_hint usage);

    void set_geometry(std::span<vertex const> vertices, pass const* pass);
    void set_geometry(std::span<quad const> quads, pass const* pass);
    void set_geometry(geometry_data const& gd, pass const* pass);
    void reset_geometry();

private:
    void on_render_to_target(render_target& target) override;

    pass const* _pass {nullptr};
    usize       _numIndices {0};
    usize       _numVerts {0};

    primitive_type _type {};
    vertex_array   _vertexArray;
};

////////////////////////////////////////////////////////////

class TCOB_API batch_renderer final : public renderer_base {
public:
    batch_renderer();

    void add_geometry(geometry_data const& gd, pass const* pass);
    void reset_geometry();

private:
    void on_render_to_target(render_target& target) override;

    std::vector<u32>    _indices;
    std::vector<vertex> _verts;

    vertex_array _vertexArray;

    struct batch {
        pass const*    Pass {nullptr};
        primitive_type Type {};
        u32            NumVerts {0};
        u32            NumInds {0};
        u32            OffsetVerts {0};
        u32            OffsetInds {0};
    };

    batch              _currentBatch;
    std::vector<batch> _batches;
};

////////////////////////////////////////////////////////////

class TCOB_API canvas_renderer final : public renderer_base {
public:
    explicit canvas_renderer(canvas& c);

    void add_layer(i32 layer);

    void set_bounds(rect_f const& bounds);

    void set_shader(asset_ptr<shader> shader);

protected:
    void prepare_render(render_target& target) override;
    void on_render_to_target(render_target& target) override;
    void finalize_render(render_target& target) override;

    vertex_array              _vertexArray;
    canvas&                   _canvas;
    asset_owner_ptr<material> _material {};
    std::vector<i32>          _layers;
};

}
