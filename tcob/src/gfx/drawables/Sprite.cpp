// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/drawables/Sprite.hpp>

#include <algorithm>

#include <tcob/gfx/gl/GLRenderer.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>

namespace tcob {

void SpriteBatch::draw(gl::RenderTarget& target)
{
    if (!is_visible()) {
        return;
    }

    _renderer.prepare(_children.size());

    const RectF frustum { target.camera().frustum() };
    for (auto& sprite : _children) {
        if (sprite.is_visible() && sprite.aabb().intersects(frustum)) {
            _renderer.add_quads(&sprite._quad, 1, sprite._material.object());
        }
    }

    _renderer.render_to_target(target);
}

auto SpriteBatch::create_sprite() -> Sprite&
{
    return _children.emplace_back();
}

void SpriteBatch::add_sprite(const Sprite& sprite)
{
    return _children.push_back(sprite);
}

auto SpriteBatch::at(isize idx) -> Sprite&
{
    return _children.at(idx);
}

auto SpriteBatch::find_by_id(u64 id) -> Sprite*
{
    return find_if([id](const auto& s) { return s.id() == id; });
}

auto SpriteBatch::sprite_count() const -> isize
{
    return _children.size();
}

void SpriteBatch::update(MilliSeconds deltaTime)
{
    for (auto& child : _children) {
        child.update(deltaTime);
    }
}

////////////////////////////////////////////////////////////

Sprite::Sprite()
    : _id { ++ID }
{
    color(Colors::White);
}

auto Sprite::material() const -> ResourcePtr<Material>
{
    return _material;
}

void Sprite::material(ResourcePtr<Material> material, const std::string& texRegion)
{
    _material = std::move(material);
    texture_region(texRegion);
}

void Sprite::texture_region(const std::string& texRegion)
{
    if (!_material) {
        return;
    }

    if (_material->Texture) {
        auto& regions { _material->Texture->regions() };
        _quad.texcoords(regions[texRegion]);
    }
}

void Sprite::color(const Color& color)
{
    _color = color;
    _quad.color(_color);
}

void Sprite::transparency(f32 trans)
{
    Color c { _color };
    c.A = 255 - static_cast<u8>(255 * std::clamp(trans, 0.f, 1.f));
    color(c);
}

auto Sprite::id() const -> u64
{
    return _id;
}

void Sprite::texture_scroll(const PointF& scroll)
{
    _textureScroll = scroll;
}

auto Sprite::aabb() const -> RectF
{
    return _aabb;
}

void Sprite::update_aabb()
{
    auto& trans { transform() };
    auto& rect { bounds() };

    const PointF topLeft { trans * rect.top_left() };
    const PointF topRight { trans * rect.top_right() };
    const PointF bottomLeft { trans * rect.bottom_left() };
    const PointF bottomRight { trans * rect.bottom_right() };

    const std::pair<f32, f32> tb { std::minmax({ topLeft.Y, topRight.Y, bottomLeft.Y, bottomRight.Y }) };
    const std::pair<f32, f32> lr { std::minmax({ topLeft.X, topRight.X, bottomLeft.X, bottomRight.X }) };

    _aabb.position({ lr.first, tb.first });
    _aabb.size({ lr.second - lr.first, tb.second - tb.first });
}

void Sprite::update(MilliSeconds delta)
{
    if (is_transform_dirty()) {
        _quad.position(bounds(), transform());
        update_aabb();
    }

    if (_textureScroll != PointF::Zero) {
        _quad.scroll_texcoords(_textureScroll * (static_cast<f32>(delta.count()) / 1000));
    }
}

auto Sprite::is_visible() const -> bool
{
    return _visible && _material;
}

void Sprite::show()
{
    _visible = true;
}

void Sprite::hide()
{
    _visible = false;
}

}