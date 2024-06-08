// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Sprite.hpp"

#include <algorithm>

#include "tcob/gfx/Renderer.hpp"

namespace tcob::gfx {

static_sprite_batch::static_sprite_batch(std::span<std::shared_ptr<sprite>> sprites)
{
    _renderer.prepare(sprites.size());
    for (auto& sprite : sprites) {
        sprite->update(milliseconds {0});
        if (sprite->is_visible()) {
            _renderer.add_geometry(sprite->_quad, sprite->Material());
        }
    }
}

void static_sprite_batch::on_update(milliseconds)
{
    // nothing to do
}

auto static_sprite_batch::can_draw() const -> bool
{
    return true;
}

void static_sprite_batch::on_draw_to(render_target& target)
{
    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////

sprite_batch::sprite_batch()
{
    _children.reserve(32);
}

auto sprite_batch::create_sprite() -> std::shared_ptr<sprite>
{
    return _children.emplace_back(std::make_shared<sprite>());
}

void sprite_batch::remove_sprite(std::shared_ptr<sprite> const& sprite)
{
    _children.erase(std::find(_children.begin(), _children.end(), sprite));
}

void sprite_batch::clear()
{
    _children.clear();
}

void sprite_batch::move_to_front(std::shared_ptr<sprite> const& sprite)
{
    auto it {std::find(_children.begin(), _children.end(), sprite)};
    if (it != _children.end()) {
        std::rotate(it, it + 1, _children.end());
    }
}

void sprite_batch::send_to_back(std::shared_ptr<sprite> const& sprite)
{
    auto it {std::find(_children.begin(), _children.end(), sprite)};
    if (it != _children.end()) {
        std::rotate(_children.begin(), it, it + 1);
    }
}

auto sprite_batch::get_sprite_count() const -> isize
{
    return std::ssize(_children);
}

auto sprite_batch::get_sprite_at(usize index) const -> std::shared_ptr<sprite>
{
    return _children.at(index);
}

void sprite_batch::on_update(milliseconds deltaTime)
{
    for (auto& child : _children) {
        child->update(deltaTime);
    }
}

auto sprite_batch::can_draw() const -> bool
{
    return !_children.empty();
}

void sprite_batch::on_draw_to(render_target& target)
{
    _renderer.prepare(_children.size());

    for (auto& child : _children) {
        if (child->is_visible()) {
            _renderer.add_geometry(child->_quad, child->Material());
        }
    }

    _renderer.render_to_target(target);
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

auto sprite::is_visible() const -> bool
{
    return _visible && Material();
}

void sprite::show()
{
    _visible = true;
}

void sprite::hide()
{
    _visible = false;
}

}
