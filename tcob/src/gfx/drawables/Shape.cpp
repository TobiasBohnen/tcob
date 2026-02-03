// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Shape.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Polygon.hpp"
#include "tcob/gfx/Ray.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

shape_batch::shape_batch()
{
    _children.reserve(32);
}

auto shape_batch::remove_shape(shape const& shape) -> bool
{
    if (helper::erase_first(_children, [&shape](auto const& val) { return val.get() == &shape; })) {
        _isDirty = true;
        return true;
    }

    return false;
}

void shape_batch::clear()
{
    _children.clear();
    _isDirty = true;
}

void shape_batch::bring_to_front(shape const& shape)
{
    auto it {std::ranges::find_if(_children, [&shape](auto const& val) { return val.get() == &shape; })};
    if (it != _children.end() && std::next(it) != _children.end()) {
        std::rotate(it, it + 1, _children.end());
        _isDirty = true;
    }
}

void shape_batch::send_to_back(shape const& shape)
{
    auto it {std::ranges::find_if(_children, [&shape](auto const& val) { return val.get() == &shape; })};
    if (it != _children.end() && it != _children.begin()) {
        std::rotate(_children.begin(), it, it + 1);
        _isDirty = true;
    }
}

void shape_batch::sort_by_y_position()
{
    std::ranges::stable_sort(_children, [](auto const& a, auto const& b) {
        return a->aabb().bottom() < b->aabb().bottom();
    });
    _isDirty = true;
}

auto shape_batch::size() const -> isize
{
    return std::ssize(_children);
}

auto shape_batch::is_empty() const -> bool
{
    return _children.empty();
}

auto shape_batch::get_shape_at(isize index) const -> shape&
{
    return *_children.at(static_cast<usize>(index));
}

auto shape_batch::intersect(ray const& ray, u32 mask) const -> std::unordered_map<shape*, std::vector<ray::result>>
{
    std::unordered_map<shape*, std::vector<ray::result>> retValue;
    for (auto const& child : _children) {
        if (child->IntersectMask & mask) {
            auto points {child->intersect(ray)};
            if (points.empty()) { continue; }
            retValue.emplace(child.get(), std::move(points));
        }
    }
    return retValue;
}

auto shape_batch::intersect(rect_f const& rect, u32 mask) const -> std::vector<shape*>
{
    std::vector<shape*> retValue;
    for (auto const& child : _children) {
        if (child->IntersectMask & mask) {
            if (child->aabb().intersects(rect, true)) {
                retValue.push_back(child.get());
            }
        }
    }
    return retValue;
}

void shape_batch::on_update(milliseconds deltaTime)
{
    for (auto& child : _children) {
        if (child->is_dirty()) { _isDirty = true; }

        child->update(deltaTime);
    }
}

auto shape_batch::can_draw() const -> bool
{
    return !_children.empty();
}

void shape_batch::on_draw_to(render_target& target)
{
    if (_isDirty) {
        _isDirty = false;

        _renderer.reset_geometry();

        isize maxPasses {0};
        for (auto& shape : _children) {
            if (!shape->is_visible()) { continue; }
            maxPasses = std::max(maxPasses, shape->Material->pass_count());
        }

        for (auto& shape : _children) {
            if (!shape->is_visible()) { continue; }
            for (isize p {0}; p < maxPasses; ++p) {
                if (p >= shape->Material->pass_count()) { continue; }

                auto const& pass {shape->Material->get_pass(p)};
                _renderer.add_geometry(shape->geometry(p), &pass);
            }
        }
    }

    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

shape::shape()
{
    Pivot.Changed.connect([this](auto const&) { mark_dirty(); });

    Material.Changed.connect([this] { TextureRegion("default"); });
    TextureRegion.Changed.connect([this] { mark_dirty(); });
    Color.Changed.connect([this] { mark_dirty(); });
}

void shape::show()
{
    _visible = true;
}

void shape::hide()
{
    _visible = false;
}

auto shape::is_visible() const -> bool
{
    return _visible && *Material;
}

void shape::on_transform_changed()
{
    mark_dirty();
}

auto shape::is_dirty() const -> bool
{
    return _isDirty;
}

void shape::mark_dirty()
{
    _isDirty = true;
}

void shape::mark_clean()
{
    _isDirty = false;
}

auto shape::get_texture_region(pass const& pass) const -> texture_region
{
    if (pass.Texture && pass.Texture->regions().contains(TextureRegion)) {
        return pass.Texture->regions()[TextureRegion];
    }

    return {.UVRect = {0, 0, 1, 1}, .Level = 0};
}

auto shape::pivot() const -> point_f
{
    if ((*Pivot).has_value()) {
        return **Pivot;
    }

    return center();
}

////////////////////////////////////////////////////////////

rect_shape::rect_shape()
{
    Bounds.Changed.connect([this](auto const&) { mark_transform_dirty(); });
}

auto rect_shape::geometry(isize pass) -> geometry_view
{
    auto it {_quads.find(pass)};
    if (it == _quads.end()) { return {}; }

    return {
        .Vertices = it->second,
        .Indices  = QuadIndicies,
        .Type     = primitive_type::Triangles};
}

auto rect_shape::intersect(ray const& ray) const -> std::vector<ray::result>
{
    return ray.intersect_rect(*Bounds, transform());
}

auto rect_shape::aabb() const -> rect_f
{
    auto const& xform {transform()};
    auto const& rect {*Bounds};

    auto const topLeft {xform * rect.top_left()};
    auto const topRight {xform * rect.top_right()};
    auto const bottomLeft {xform * rect.bottom_left()};
    auto const bottomRight {xform * rect.bottom_right()};

    std::pair<f32, f32> const tb {std::minmax({topLeft.Y, topRight.Y, bottomLeft.Y, bottomRight.Y})};
    std::pair<f32, f32> const lr {std::minmax({topLeft.X, topRight.X, bottomLeft.X, bottomRight.X})};

    return {{lr.first, tb.first}, {lr.second - lr.first, tb.second - tb.first}};
}

void rect_shape::move_by(point_f offset)
{
    Bounds = {Bounds->Position + offset, Bounds->Size};
}

void rect_shape::on_update(milliseconds deltaTime)
{
    if (is_dirty()) {
        update_geometry();
    }

    if (TextureScroll != point_f::Zero) {
        for (auto& kvp : _quads) {
            geometry::scroll_texcoords(kvp.second, *TextureScroll * (deltaTime.count() / 1000.0f));
        }
    }
}

void rect_shape::update_geometry()
{
    mark_clean();

    _quads.clear();

    auto const& xform {transform()};
    for (isize p {0}; p < Material->pass_count(); ++p) {
        auto const& pass {Material->get_pass(p)};

        auto& quad {_quads[p]};
        geometry::set_color(quad, Color);
        geometry::set_texcoords(quad, get_texture_region(pass));
        geometry::set_position(quad, *Bounds, xform);
    }
}

auto rect_shape::center() const -> point_f
{
    return Bounds->center();
}

////////////////////////////////////////////////////////////

circle_shape::circle_shape()
{
    Center.Changed.connect([this](auto const&) { mark_transform_dirty(); });
    Radius.Changed.connect([this](auto const&) { mark_dirty(); });
    Segments.Changed.connect([this](auto const&) { mark_dirty(); });
}

auto circle_shape::geometry(isize pass) -> geometry_view
{
    return {
        .Vertices = _store.get_vertices(pass),
        .Indices  = _store.get_indices(pass),
        .Type     = primitive_type::Triangles};
}

auto circle_shape::aabb() const -> rect_f
{
    auto const worldCenter {transform() * Center};
    return {worldCenter - point_f {Radius, Radius},
            {Radius * 2.0f, Radius * 2.0f}};
}

auto circle_shape::intersect(ray const& ray) const -> std::vector<ray::result>
{
    return ray.intersect_circle(transform() * Center, Radius);
}

void circle_shape::on_update(milliseconds /* deltaTime */)
{
    if (!is_dirty()) { return; }

    update_geometry();
}

void circle_shape::update_geometry()
{
    mark_clean();

    _store.clear();
    auto const& xform {transform()};

    if (Segments < 3 || Radius < 1.0f) { return; }

    for (isize p {0}; p < Material->pass_count(); ++p) {
        auto const& pass {Material->get_pass(p)};

        std::vector<vertex> verts {};
        verts.reserve(Segments + 1);
        std::vector<u32> inds {};
        inds.reserve(Segments * 3);

        // vertices
        f32 const angleStep {TAU_F / Segments};
        f32 const radius {Radius};

        texture_region const texReg {get_texture_region(pass)};

        f32 const texLevel {static_cast<f32>(texReg.Level)};

        auto const [centerX, centerY] {*Center};
        auto const& uvRect {texReg.UVRect};
        verts.push_back({.Position  = (xform * Center),
                         .Color     = Color,
                         .TexCoords = {.U     = (0.5f * uvRect.width()) + uvRect.left(),
                                       .V     = (0.5f * uvRect.height()) + uvRect.top(),
                                       .Level = texLevel}});

        auto const uvSquare {rect_f::FromLTRB(centerX - radius, centerY - radius, centerX + radius, centerY + radius)};

        for (i32 i {0}; i < Segments; ++i) {
            f32 const angle {i * angleStep};
            f32 const x {(radius * std::cos(angle)) + centerX};
            f32 const y {(radius * std::sin(angle)) + centerY};
            verts.push_back({.Position  = (xform * point_f {x, y}),
                             .Color     = Color,
                             .TexCoords = {.U     = (((x - uvSquare.left()) / uvSquare.width()) * uvRect.width()) + uvRect.left(),
                                           .V     = (((y - uvSquare.top()) / uvSquare.height()) * uvRect.height()) + uvRect.top(),
                                           .Level = texLevel}});
        }

        // indices
        for (i32 i {1}; i < Segments; ++i) {
            inds.push_back(0);
            inds.push_back(i + 1);
            inds.push_back(i);
        }

        inds.push_back(0);
        inds.push_back(1);
        inds.push_back(*Segments);

        _store.set(p, verts, inds);
    }
}

auto circle_shape::center() const -> point_f
{
    return Center;
}

////////////////////////////////////////////////////////////

poly_shape::poly_shape()
{
    Polygons.Changed.connect([this](auto const&) { mark_transform_dirty(); });
}

auto poly_shape::geometry(isize pass) -> geometry_view
{
    return {
        .Vertices = _store.get_vertices(pass),
        .Indices  = _store.get_indices(pass),
        .Type     = primitive_type::Triangles};
}

auto poly_shape::aabb() const -> rect_f
{
    auto const& xform {transform()};

    point_f max {std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest()};
    point_f min {std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max()};

    for (auto const& polygon : *Polygons) {
        for (auto const& p : polygon.Outline) {
            auto const t {xform * p};
            min.X = std::min(min.X, t.X);
            min.Y = std::min(min.Y, t.Y);
            max.X = std::max(max.X, t.X);
            max.Y = std::max(max.Y, t.Y);
        }
    }

    return rect_f::FromLTRB(min.X, min.Y, max.X, max.Y);
}

auto poly_shape::intersect(ray const& ray) const -> std::vector<ray::result>
{
    auto const& xform {transform()};

    std::vector<ray::result> retValue;
    for (auto const& polygon : *Polygons) {
        auto points {ray.intersect_polyline(polygon.Outline, xform)};
        retValue.insert(retValue.end(), points.begin(), points.end());

        for (auto const& hole : polygon.Holes) {
            auto holePoints {ray.intersect_polyline(hole, xform)};
            retValue.insert(retValue.end(), holePoints.begin(), holePoints.end());
        }
    }
    return retValue;
}

void poly_shape::clip(poly_shape const& other, clip_mode mode)
{
    Polygons.mutate([&](std::vector<polygon>& polygons) { polygons::clip(polygons, *other.Polygons, mode); });
    mark_dirty();
}

void poly_shape::move_by(point_f offset)
{
    Polygons.mutate([&](std::vector<polygon>& polygons) { polygons::move_by(polygons, offset); });
    mark_dirty();
}

void poly_shape::on_update(milliseconds /* deltaTime */)
{
    if (!is_dirty()) { return; }

    update_geometry();
}

auto poly_shape::center() const -> point_f
{
    return _centroid;
}

void poly_shape::update_geometry()
{
    mark_clean();

    auto const info {polygons::info(*Polygons)};
    _boundingBox = info.BoundingBox;
    _centroid    = info.Centroid;

    _store.clear();
    auto const& xform {transform()};

    for (isize p {0}; p < Material->pass_count(); ++p) {
        u32 indOffset {0};

        auto const&         pass {Material->get_pass(p)};
        std::vector<vertex> verts {};
        std::vector<u32>    inds {};

        // create verts
        texture_region const texReg {get_texture_region(pass)};

        f32 const   texLevel {static_cast<f32>(texReg.Level)};
        auto const& uvRect {texReg.UVRect};

        auto const pushVert {[&](point_f point) {
            auto const& [x, y] {point};
            verts.push_back({.Position  = (xform * point),
                             .Color     = Color,
                             .TexCoords = {.U     = (((x - _boundingBox.left()) / _boundingBox.width()) * uvRect.width()) + uvRect.left(),
                                           .V     = (((y - _boundingBox.top()) / _boundingBox.height()) * uvRect.height()) + uvRect.top(),
                                           .Level = texLevel}});
        }};

        for (auto const& polygon : *Polygons) {
            // outline
            // push verts
            for (auto const& point : polygon.Outline) { pushVert(point); }

            // create inds
            auto const indices {polygon.earcut()};
            for (u32 i {0}; i < indices.size(); i += 3) {
                inds.push_back(indices[i + 2] + indOffset);
                inds.push_back(indices[i + 1] + indOffset);
                inds.push_back(indices[i + 0] + indOffset);
            }
            indOffset += static_cast<u32>(polygon.Outline.size());

            // holes
            for (auto const& hole : polygon.Holes) {
                // push verts
                for (auto const point : hole) { pushVert(point); }

                indOffset += static_cast<u32>(hole.size());
            }
        }

        _store.set(p, verts, inds);
    }
}

////////////////////////////////////////////////////////////

void mesh_shape::set(std::span<vertex const> verts, std::span<u32 const> inds)
{
    _verts = {verts.begin(), verts.end()};
    _inds  = {inds.begin(), inds.end()};

    // Edge counting
    _edgeCount.clear();
    for (usize i {0}; i < _inds.size() - 2; i += 3) {
        u32 const v0 {_inds[i]};
        u32 const v1 {_inds[i + 1]};
        u32 const v2 {_inds[i + 2]};

        _edgeCount[{v0, v1}]++;
        _edgeCount[{v1, v2}]++;
        _edgeCount[{v2, v0}]++;
    }

    mark_transform_dirty();

    if (_verts.empty()) {
        _localBounds = rect_f::Zero;
    } else {
        point_f min {std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max()};
        point_f max {std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest()};

        for (auto const& v : _verts) {
            min.X = std::min(min.X, v.Position.X);
            min.Y = std::min(min.Y, v.Position.Y);
            max.X = std::max(max.X, v.Position.X);
            max.Y = std::max(max.Y, v.Position.Y);
        }
        _localBounds = rect_f::FromLTRB(min.X, min.Y, max.X, max.Y);
    }
}

auto mesh_shape::load(path const& file) noexcept -> bool
{
    io::ifstream fs {file};
    return load(fs, io::get_extension(file));
}

auto mesh_shape::load(io::istream& in, string const& ext) noexcept -> bool
{
    if (!in) { return false; }

    if (auto loader {locate_service<mesh_loader::factory>().create(ext)}) {
        if (auto store {loader->load(in)}) {
            set(store->get_vertices(0), store->get_indices(0));
            return true;
        }
    }

    return false;
}

auto mesh_shape::geometry(isize /* pass */) -> geometry_view
{
    return {.Vertices = _xformVerts, .Indices = _inds, .Type = primitive_type::Triangles};
}

auto mesh_shape::aabb() const -> rect_f
{
    if (_verts.empty()) { return rect_f::Zero; }

    auto const& xform {transform()};

    auto const topLeft {xform * _localBounds.top_left()};
    auto const topRight {xform * _localBounds.top_right()};
    auto const bottomLeft {xform * _localBounds.bottom_left()};
    auto const bottomRight {xform * _localBounds.bottom_right()};

    std::pair<f32, f32> const tb {std::minmax({topLeft.Y, topRight.Y, bottomLeft.Y, bottomRight.Y})};
    std::pair<f32, f32> const lr {std::minmax({topLeft.X, topRight.X, bottomLeft.X, bottomRight.X})};

    return {{lr.first, tb.first}, {lr.second - lr.first, tb.second - tb.first}};
}

auto mesh_shape::intersect(ray const& ray) const -> std::vector<ray::result>
{
    std::vector<ray::result>      results {};
    std::set<std::pair<u32, u32>> tested;

    for (auto const& [edge, count] : _edgeCount) {
        auto const [a, b] {edge};
        auto const reverseEdge {std::make_pair(b, a)};

        if (!_edgeCount.contains(reverseEdge)) {
            if (tested.insert(std::minmax(a, b)).second) {
                auto const& v0 {_xformVerts[a].Position};
                auto const& v1 {_xformVerts[b].Position};

                if (auto hit {ray.intersect_line(v0, v1)}) {
                    results.push_back(*hit);
                }
            }
        }
    }

    return results;
}

void mesh_shape::move_by(point_f offset)
{
    for (auto& v : _verts) {
        v.Position += offset;
    }
    mark_dirty();
    if (!_verts.empty()) {
        _localBounds.Position += offset;
    }
}

void mesh_shape::on_update(milliseconds /* deltaTime */)
{
    if (!is_dirty()) { return; }

    update_geometry();
}

void mesh_shape::update_geometry()
{
    mark_clean();

    auto const xform {transform()};

    _xformVerts.clear();
    _xformVerts.reserve(_verts.size());

    // Pre-calculate color mods
    f32 const rMod {Color->R / 255.f};
    f32 const gMod {Color->G / 255.f};
    f32 const bMod {Color->B / 255.f};
    f32 const aMod {Color->A / 255.f};

    for (auto v : _verts) {
        v.Position = xform * v.Position;
        v.Color.R  = static_cast<u8>(v.Color.R * rMod);
        v.Color.G  = static_cast<u8>(v.Color.G * gMod);
        v.Color.B  = static_cast<u8>(v.Color.B * bMod);
        v.Color.A  = static_cast<u8>(v.Color.A * aMod);
        _xformVerts.push_back(v);
    }
}

auto mesh_shape::center() const -> point_f
{
    if (_verts.empty()) { return {0, 0}; }

    point_f sum {0, 0};
    for (auto const& v : _verts) {
        sum.X += v.Position.X;
        sum.Y += v.Position.Y;
    }

    return {sum.X / static_cast<f32>(_verts.size()), sum.Y / static_cast<f32>(_verts.size())};
}

}
