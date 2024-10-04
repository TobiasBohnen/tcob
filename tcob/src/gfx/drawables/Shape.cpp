// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Shape.hpp"

#include <algorithm>

#include "tcob/gfx/Renderer.hpp"

namespace tcob::gfx {

static_shape_batch::static_shape_batch(std::span<std::shared_ptr<shape>> shapes)
{
    for (auto& shape : shapes) {
        shape->update(milliseconds {0});
        if (shape->is_visible()) {
            _renderer.add_geometry(shape->get_geometry(), shape->Material());
        }
    }
}

void static_shape_batch::on_update(milliseconds)
{
    // nothing to do
}

auto static_shape_batch::can_draw() const -> bool
{
    return true;
}

void static_shape_batch::on_draw_to(render_target& target)
{
    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////

shape_batch::shape_batch()
{
    _children.reserve(32);
}

void shape_batch::remove_shape(shape const& shape)
{
    _children.erase(std::ranges::find_if(_children, [&shape](auto const& val) {
        return val.get() == &shape;
    }));
}

void shape_batch::clear()
{
    _children.clear();
}

void shape_batch::bring_to_front(shape const& shape)
{
    auto it {std::ranges::find_if(_children, [&shape](auto const& val) {
        return val.get() == &shape;
    })};
    if (it != _children.end()) {
        std::rotate(it, it + 1, _children.end());
    }
}

void shape_batch::send_to_back(shape const& shape)
{
    auto it {std::ranges::find_if(_children, [&shape](auto const& val) {
        return val.get() == &shape;
    })};
    if (it != _children.end()) {
        std::rotate(_children.begin(), it, it + 1);
    }
}

auto shape_batch::get_shape_count() const -> isize
{
    return std::ssize(_children);
}

auto shape_batch::is_empty() const -> bool
{
    return _children.empty();
}

auto shape_batch::get_shape_at(usize index) const -> std::shared_ptr<shape>
{
    return _children.at(index);
}

auto shape_batch::intersect(ray const& ray, u32 mask) -> std::unordered_map<shape*, std::vector<ray::result>>
{
    std::unordered_map<shape*, std::vector<ray::result>> retValue;
    for (auto& child : _children) {
        if (child->RayCastMask & mask) {
            auto const points {child->intersect(ray)};
            if (points.empty()) { continue; }
            retValue[child.get()] = points;
        }
    }
    return retValue;
}

void shape_batch::on_update(milliseconds deltaTime)
{
    _isDirty = false;
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

        for (auto& shape : _children) {
            if (shape->is_visible()) {
                _renderer.add_geometry(shape->get_geometry(), shape->Material());
            }
        }
    }

    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

shape::shape()
    : TextureRegion("default")
    , Color(colors::White)
    , Transparency {{[&]() -> f32 { return static_cast<f32>(Color->A) / 255.0f; }, [&](f32 value) {
                         color c {Color()};
                         c.A   = 255 - static_cast<u8>(255 * std::clamp(value, 0.0f, 1.0f));
                         Color = c; }}}

{
    Pivot.Changed.connect([&](auto const&) { mark_dirty(); });

    Material.Changed.connect([&](auto const&) { TextureRegion("default"); });
    TextureRegion.Changed.connect([&](string const& texRegion) {
        on_texture_region_changed(texRegion);
        mark_dirty();
    });
    Color.Changed.connect([&](auto const& color) {
        on_color_changed(color);
        mark_dirty();
    });
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
    return _visible && Material();
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

auto shape::get_pivot() const -> point_f
{
    if (Pivot().has_value()) {
        return *Pivot();
    }

    return get_center();
}

////////////////////////////////////////////////////////////

circle_shape::circle_shape()
{
    Center.Changed.connect([&](auto const&) { mark_dirty(); });
    Radius.Changed.connect([&](auto const&) { mark_dirty(); });
    Segments.Changed.connect([&](auto const&) { mark_dirty(); });
    Segments(90);
}

auto circle_shape::get_geometry() -> geometry_data
{
    return {
        .Vertices = _verts,
        .Indices  = _inds,
        .Type     = primitive_type::Triangles};
}

auto circle_shape::intersect(ray const& ray) -> std::vector<ray::result>
{
    return ray.intersect_circle(get_transform() * Center(), Radius());
}

void circle_shape::on_update(milliseconds /* deltaTime */)
{
    if (!is_dirty()) { return; }
    mark_clean();

    create();
}

void circle_shape::on_color_changed(color c)
{
    geometry::set_color(_verts, c);
}

void circle_shape::on_texture_region_changed(string const& /* texRegion */)
{
    // TODO
}

void circle_shape::create()
{
    _verts.clear();
    _inds.clear();

    if (Segments < 3 || Radius < 1) { return; }

    // vertices
    auto const& xform {get_transform()};
    f32 const   angleStep {TAU_F / Segments};
    f32 const   radius {Radius()};

    texture_region texReg {};
    if (Material() && Material->Texture && Material->Texture->has_region(TextureRegion)) {
        texReg = Material->Texture->get_region(TextureRegion);
    } else {
        texReg = {.UVRect = {0, 0, 1, 1}, .Level = 0};
    }
    f32 const texLevel {static_cast<f32>(texReg.Level)};

    auto const [centerX, centerY] {Center()};
    auto const& uvRect {texReg.UVRect};
    _verts.push_back({.Position  = (xform * Center()).as_array(),
                      .Color     = Color->as_array(),
                      .TexCoords = {(0.5f * uvRect.Width) + uvRect.X,
                                    (0.5f * uvRect.Height) + uvRect.Y,
                                    texLevel}});

    auto const uvSquare {rect_f::FromLTRB(centerX - radius, centerY - radius, centerX + radius, centerY + radius)};

    for (i32 i {0}; i < Segments; ++i) {
        f32 const angle {i * angleStep};
        f32 const x {(radius * std::cos(angle)) + centerX};
        f32 const y {(radius * std::sin(angle)) + centerY};
        _verts.push_back({.Position  = (xform * point_f {x, y}).as_array(),
                          .Color     = Color->as_array(),
                          .TexCoords = {(((x - uvSquare.X) / uvSquare.Width) * uvRect.Width) + uvRect.X,
                                        (((y - uvSquare.Y) / uvSquare.Height) * uvRect.Height) + uvRect.Y,
                                        texLevel}});
    }

    // indices
    for (i32 i {1}; i < Segments; ++i) {
        _inds.push_back(0);
        _inds.push_back(i + 1);
        _inds.push_back(i);
    }

    _inds.push_back(0);
    _inds.push_back(1);
    _inds.push_back(Segments);
}

auto circle_shape::get_center() const -> point_f
{
    return Center();
}

////////////////////////////////////////////////////////////

rect_shape::rect_shape()
{
    Bounds.Changed.connect([&](auto const&) { mark_dirty(); });

    geometry::set_color(_quad, colors::White);
    geometry::set_texcoords(_quad, {{0, 0, 1, 1}, 1});
}

auto rect_shape::get_geometry() -> geometry_data
{
    static std::array<u32, 6> Inds {3, 1, 0, 3, 2, 1};
    return {
        .Vertices = _quad,
        .Indices  = Inds,
        .Type     = primitive_type::Triangles};
}

auto rect_shape::intersect(ray const& ray) -> std::vector<ray::result>
{
    return ray.intersect_rect(Bounds(), get_transform());
}

auto rect_shape::get_AABB() const -> rect_f
{
    return _aabb;
}

void rect_shape::move_by(point_f offset)
{
    Bounds = {Bounds->get_position() + offset, Bounds->get_size()};
}

void rect_shape::update_aabb()
{
    auto const& xform {get_transform()};
    auto const& rect {Bounds()};

    auto const topLeft {xform * rect.top_left()};
    auto const topRight {xform * rect.top_right()};
    auto const bottomLeft {xform * rect.bottom_left()};
    auto const bottomRight {xform * rect.bottom_right()};

    std::pair<f32, f32> const tb {std::minmax({topLeft.Y, topRight.Y, bottomLeft.Y, bottomRight.Y})};
    std::pair<f32, f32> const lr {std::minmax({topLeft.X, topRight.X, bottomLeft.X, bottomRight.X})};

    _aabb = {{lr.first, tb.first}, {lr.second - lr.first, tb.second - tb.first}};
}

void rect_shape::on_update(milliseconds deltaTime)
{
    if (is_dirty()) {
        geometry::set_position(_quad, Bounds(), get_transform());
        update_aabb();
        mark_clean();
    }

    if (TextureScroll() != point_f::Zero) {
        geometry::scroll_texcoords(_quad, TextureScroll() * (deltaTime.count() / 1000.0f));
    }
}

void rect_shape::on_color_changed(color c)
{
    geometry::set_color(_quad, c);
}

void rect_shape::on_texture_region_changed(string const& texRegion)
{
    if (Material() && Material->Texture && Material->Texture->has_region(texRegion)) {
        geometry::set_texcoords(_quad, Material->Texture->get_region(texRegion));
    } else {
        geometry::set_texcoords(_quad, {{0, 0, 1, 1}, 1});
    }
}

auto rect_shape::get_center() const -> point_f
{
    return Bounds->get_center();
}

////////////////////////////////////////////////////////////

poly_shape::poly_shape()
{
    Polygons.Changed.connect([&](auto const&) { mark_dirty(); });
}

auto poly_shape::get_geometry() -> geometry_data
{
    return {
        .Vertices = _verts,
        .Indices  = _inds,
        .Type     = primitive_type::Triangles};
}

auto poly_shape::intersect(ray const& ray) -> std::vector<ray::result>
{
    std::vector<ray::result> retValue;
    for (auto const& polygon : Polygons()) {
        auto points {ray.intersect_polyline(polygon.Outline, get_transform())};
        retValue.insert(retValue.end(), points.begin(), points.end());

        for (auto const& hole : polygon.Holes) {
            auto holePoints {ray.intersect_polyline(hole, get_transform())};
            retValue.insert(retValue.end(), holePoints.begin(), holePoints.end());
        }
    }
    return retValue;
}

void poly_shape::clip(poly_shape const& other, clip_mode mode)
{
    polygons::clip(*Polygons, *other.Polygons, mode);
    mark_dirty();
}

void poly_shape::move_by(point_f offset)
{
    polygons::move_by(*Polygons, offset);
    mark_dirty();
}

void poly_shape::on_update(milliseconds /* deltaTime */)
{
    if (!is_dirty()) { return; }
    mark_clean();

    auto const info {polygons::get_info(*Polygons)};
    _boundingBox = info.BoundingBox;
    _centroid    = info.Centroid;

    create();
}

void poly_shape::on_color_changed(color c)
{
    auto const carr {c.as_array()};
    for (auto& vert : _verts) {
        vert.Color = carr;
    }
}

void poly_shape::on_texture_region_changed(string const& /* texRegion */)
{
    // TODO
}

auto poly_shape::get_center() const -> point_f
{
    return _centroid;
}

void poly_shape::create()
{
    _verts.clear();
    _inds.clear();
    u32 indOffset {0};

    // create verts
    auto const&    xform {get_transform()};
    texture_region texReg {};
    if (Material() && Material->Texture && Material->Texture->has_region(TextureRegion)) {
        texReg = Material->Texture->get_region(TextureRegion);
    } else {
        texReg = {.UVRect = {0, 0, 1, 1}, .Level = 0};
    }
    f32 const   texLevel {static_cast<f32>(texReg.Level)};
    auto const& uvRect {texReg.UVRect};

    auto const pushVert {[&](point_f point) {
        auto const& [x, y] {point};
        _verts.push_back({.Position  = (xform * point).as_array(),
                          .Color     = Color->as_array(),
                          .TexCoords = {(((x - _boundingBox.X) / _boundingBox.Width) * uvRect.Width) + uvRect.X,
                                        (((y - _boundingBox.Y) / _boundingBox.Height) * uvRect.Height) + uvRect.Y,
                                        texLevel}});
    }};

    for (auto const& polygon : Polygons()) {
        // outline
        // push verts
        for (auto const& p : polygon.Outline) { pushVert(p); }

        // create inds
        auto const indices {polygon.earcut()};
        for (u32 i {0}; i < indices.size(); i += 3) {
            _inds.push_back(indices[i + 2] + indOffset);
            _inds.push_back(indices[i + 1] + indOffset);
            _inds.push_back(indices[i + 0] + indOffset);
        }
        indOffset += polygon.Outline.size();

        // holes
        for (auto const& hole : polygon.Holes) {
            // push verts
            for (auto const p : hole) { pushVert(p); }

            indOffset += hole.size();
        }
    }
}

////////////////////////////////////////////////////////////
}
