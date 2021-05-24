// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/gl/GLEnum.hpp>
#include <tcob/gfx/gl/GLVertexArray.hpp>

namespace tcob::gl {
class Renderer {
public:
    void render_to_target(RenderTarget& target, bool debug = false);

protected:
    void bind_material(Material* mat);

private:
    virtual void draw() = 0;
};

////////////////////////////////////////////////////////////

class StaticQuadRenderer final : public Renderer {
public:
    void set_geometry(const Quad* quads, isize count);
    void modify_geometry(const Quad* quads, isize count, isize offset) const;

    void material(Material* mat);

private:
    void prepare(isize quadCount);
    void draw() override;

    isize _numQuads { 0 };

    VertexArray _vertexArray;
    Material* _mat { nullptr };
};

////////////////////////////////////////////////////////////

class DynamicQuadRenderer final : public Renderer {
public:
    void set_geometry(const Quad* quads, isize count);
    auto map(isize wantCount) -> Quad*;
    void unmap(isize haveCount);

    void reset();

    void material(Material* mat);

private:
    void prepare(isize quadCount);

    void draw() override;

    isize _numQuads { 0 };

    VertexArray _vertexArray;
    Material* _mat { nullptr };
};

////////////////////////////////////////////////////////////

class BatchQuadRenderer final : public Renderer {
public:
    void prepare(isize quadCount);
    void add_quads(const Quad* quads, u32 quadCount, Material* mat);

private:
    void draw() override;

    std::vector<u32> _indices;
    std::vector<Vertex> _vertices;

    VertexArray _vertexArray;

    struct Batch {
        Material* MaterialPtr { nullptr };
        u32 NumVerts { 0 };
        u32 NumInds { 0 };
        u32 OffsetVerts { 0 };
        u32 OffsetInds { 0 };
    };

    Batch _currentBatch;
    std::vector<Batch> _batches;
};

////////////////////////////////////////////////////////////

class StaticPointRenderer final : public Renderer {
public:
    void set_geometry(const Vertex* vertices, isize count);
    void modify_geometry(const Vertex* vertices, isize count, isize offset) const;

    void material(Material* mat, f32 size);

private:
    void prepare(isize vertCount);
    void draw() override;

    isize _numVerts { 0 };
    f32 _pointSize { 1.f };

    VertexArray _vertexArray;
    Material* _mat { nullptr };
};

////////////////////////////////////////////////////////////

class StreamPointRenderer final : public Renderer {
public:
    void set_geometry(const Vertex* vertices, isize count);

    void material(Material* mat, f32 size);

private:
    void prepare(isize vertCount);
    void draw() override;

    isize _numVerts { 0 };
    f32 _pointSize { 1.f };

    VertexArray _vertexArray;
    Material* _mat { nullptr };
};
}