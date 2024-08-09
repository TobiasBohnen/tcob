// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Sprite.hpp"

#include <algorithm>

#include "tcob/gfx/Renderer.hpp"

namespace tcob::gfx {

static_mesh_batch::static_mesh_batch(std::span<std::shared_ptr<mesh>> meshes)
{
    for (auto& mesh : meshes) {
        mesh->update(milliseconds {0});
        if (mesh->is_visible()) {
            auto const gd {mesh->get_geometry()};
            _renderer.add_geometry(gd.Vertices, gd.Indices, mesh->Material());
        }
    }
}

void static_mesh_batch::on_update(milliseconds)
{
    // nothing to do
}

auto static_mesh_batch::can_draw() const -> bool
{
    return true;
}

void static_mesh_batch::on_draw_to(render_target& target)
{
    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////

mesh_batch::mesh_batch()
{
    _children.reserve(32);
}

void mesh_batch::remove_mesh(mesh const& mesh)
{
    _children.erase(std::find_if(_children.begin(), _children.end(), [&mesh](auto const& val) {
        return val.get() == &mesh;
    }));
}

void mesh_batch::clear()
{
    _children.clear();
}

void mesh_batch::move_to_front(mesh const& mesh)
{
    auto it {std::find_if(_children.begin(), _children.end(), [&mesh](auto const& val) {
        return val.get() == &mesh;
    })};
    if (it != _children.end()) {
        std::rotate(it, it + 1, _children.end());
    }
}

void mesh_batch::send_to_back(mesh const& mesh)
{
    auto it {std::find_if(_children.begin(), _children.end(), [&mesh](auto const& val) {
        return val.get() == &mesh;
    })};
    if (it != _children.end()) {
        std::rotate(_children.begin(), it, it + 1);
    }
}

auto mesh_batch::get_mesh_count() const -> isize
{
    return std::ssize(_children);
}

auto mesh_batch::is_empty() const -> bool
{
    return _children.empty();
}

auto mesh_batch::get_mesh_at(usize index) const -> std::shared_ptr<mesh>
{
    return _children.at(index);
}

void mesh_batch::on_update(milliseconds deltaTime)
{
    for (auto& child : _children) {
        child->update(deltaTime);
    }
}

auto mesh_batch::can_draw() const -> bool
{
    return !_children.empty();
}

void mesh_batch::on_draw_to(render_target& target)
{
    _renderer.reset_geometry();

    for (auto& mesh : _children) {
        if (mesh->is_visible()) {
            auto const gd {mesh->get_geometry()};
            _renderer.add_geometry(gd.Vertices, gd.Indices, mesh->Material());
        }
    }

    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////

void mesh::show()
{
    _visible = true;
}

void mesh::hide()
{
    _visible = false;
}

auto mesh::is_visible() const -> bool
{
    return _visible && Material();
}

////////////////////////////////////////////////////////////

sprite::sprite()
    : Transparency {{[&]() -> f32 { return static_cast<f32>(Color->A) / 255.0f; },
                     [&](f32 value) {
                         color c {Color()};
                         c.A   = 255 - static_cast<u8>(255 * std::clamp(value, 0.0f, 1.0f));
                         Color = c;
                     }}}
{
    Material.Changed.connect([&]() { TextureRegion("default"); });
    Color.Changed.connect([&](auto const& color) { geometry::set_color(_quad, color); });
    Color(colors::White);

    TextureRegion.Changed.connect(
        [&](string const& texRegion) {
            if (Material() && Material->Texture && Material->Texture->has_region(texRegion)) {
                geometry::set_texcoords(_quad, Material->Texture->get_region(texRegion));
            } else {
                geometry::set_texcoords(_quad, {{0, 0, 1, 1}, 1});
            }
        });

    TextureRegion("default");
}

auto sprite::get_geometry() -> geometry_data
{
    static std::array<u32, 6> Inds {0, 1, 3, 1, 2, 3};
    return {
        .Vertices = _quad,
        .Indices  = Inds,
    };
}

void sprite::on_transform_dirty()
{
    _isDirty = true;
}

auto sprite::get_AABB() const -> rect_f
{
    return _aabb;
}

void sprite::update_aabb()
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

void sprite::on_update(milliseconds delta)
{
    if (_isDirty) {
        geometry::set_position(_quad, Bounds(), get_transform());
        update_aabb();
        _isDirty = false;
    }

    if (TextureScroll() != point_f::Zero) {
        geometry::scroll_texcoords(_quad, TextureScroll() * (delta.count() / 1000.0f));
    }
}
}
