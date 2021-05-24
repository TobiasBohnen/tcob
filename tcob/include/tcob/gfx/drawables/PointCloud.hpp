// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/data/Color.hpp>
#include <tcob/core/data/Point.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>

namespace tcob {

class PointCloud final : public Drawable {
public:
    void add(const Vertex& point);

    auto get(isize index) const -> const Vertex&;
    void set(isize index, const Vertex& point);

    void clear();
    auto point_count() const -> isize;

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material);

    auto point_size() const -> f32;
    void point_size(f32 size);

    void update(f64 deltaTime) override;

    void draw(gl::RenderTarget& target) override;

private:
    gl::StaticPointRenderer _renderer {};
    ResourcePtr<Material> _material;
    f32 _pointSize { 1.f };
    std::vector<Vertex> _points {};

    bool _areRenderStatesDirty { true };
    bool _isGeometryDirty { true };
};
}