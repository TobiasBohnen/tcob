// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/drawables/PointCloud.hpp>

namespace tcob {

void PointCloud::add(const Vertex& point)
{
    _points.push_back(point);
    _isGeometryDirty = true;
}

auto PointCloud::get(isize index) const -> const Vertex&
{
    return _points[index];
}

void PointCloud::set(isize index, const Vertex& point)
{
    _points[index] = point;
    _renderer.modify_geometry(&_points[index], 1, index);
}

void PointCloud::clear()
{
    _points.clear();
    _isGeometryDirty = true;
}

auto PointCloud::point_count() const -> isize
{
    return _points.size();
}

auto PointCloud::material() const -> ResourcePtr<Material>
{
    return _material;
}

void PointCloud::material(ResourcePtr<Material> material)
{
    _material = std::move(material);
    _areRenderStatesDirty = true;
}

void PointCloud::point_size(f32 size)
{
    _pointSize = size;
    _areRenderStatesDirty = true;
}

auto PointCloud::point_size() const -> f32
{
    return _pointSize;
}

void PointCloud::update([[maybe_unused]] f64 deltaTime)
{
    if (!is_visible() || _points.empty() || !_material) {
        return;
    }

    if (_isGeometryDirty) {
        _renderer.set_geometry(_points.data(), _points.size());
        _isGeometryDirty = false;
    }

    if (_areRenderStatesDirty) {
        _renderer.material(_material.object(), _pointSize);
        _areRenderStatesDirty = false;
    }
}

void PointCloud::draw(gl::RenderTarget& target)
{
    if (_points.empty() || !_material) {
        return;
    }

    _renderer.render_to_target(target);
}

}