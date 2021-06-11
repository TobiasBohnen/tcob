// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/drawables/NinePatch.hpp>

namespace tcob {

auto NinePatch::material() const -> ResourcePtr<Material>
{
    return _material;
}

void NinePatch::material(ResourcePtr<Material> material, const std::string& texRegion)
{
    _material = std::move(material);
    _renderer.material(_material.object());
    texture_region(texRegion);
}

void NinePatch::texture_region(const std::string& texRegion)
{
    if (!_material) {
        return;
    }
    if (_material->Texture) {
        auto& regions { _material->Texture->regions() };
        _texRegion = regions[texRegion];
    }

    _isDirty = true;
}

void NinePatch::define_center(const PointF& posTopLeft, const PointF& posBottomRight, const PointF& uvTopLeft, const PointF& uvBottomRight)
{
    _posTopLeft = posTopLeft;
    _posBottomRight = posBottomRight;

    _uvTopLeft = uvTopLeft;
    _uvBottomRight = uvBottomRight;

    _isDirty = true;
}

void NinePatch::update([[maybe_unused]] MilliSeconds deltaTime)
{
    if ((_isDirty || is_transform_dirty()) && _material->Texture) {

        const Transform trans { transform() };
        const RectF rect { bounds() };

        f32 left { rect.Left };
        f32 leftCenter { rect.Left + _posTopLeft.X };
        f32 rightCenter { rect.right() - _posBottomRight.X };
        f32 right { rect.right() };

        f32 top { rect.Top };
        f32 topCenter { rect.Top + _posTopLeft.Y };
        f32 bottomCenter { rect.bottom() - _posBottomRight.Y };
        f32 bottom { rect.bottom() };

        const RectF uvRect { _texRegion.UVRect };
        const u32 level { _texRegion.Level };

        f32 uv_left { uvRect.Left };
        f32 uv_leftCenter { uvRect.Left + _uvTopLeft.X };
        f32 uv_rightCenter { uvRect.right() - _uvBottomRight.X };
        f32 uv_right { uvRect.right() };

        f32 uv_top { uvRect.Top };
        f32 uv_topCenter { uvRect.Top + _uvTopLeft.Y };
        f32 uv_bottomCenter { uvRect.bottom() - _uvBottomRight.Y };
        f32 uv_bottom { uvRect.bottom() };

        Quad* quad { _renderer.map(9) };
        quad->position(RectF::FromLTRB(left, top, leftCenter, topCenter), trans); //top-left
        quad->color(Colors::White); //top-left
        quad->texcoords({ RectF::FromLTRB(uv_left, uv_top, uv_leftCenter, uv_topCenter), level }); //top-left
        quad++;

        quad->position(RectF::FromLTRB(leftCenter, top, rightCenter, topCenter), trans); //top
        quad->color(Colors::White); //top
        quad->texcoords({ RectF::FromLTRB(uv_leftCenter, uv_top, uv_rightCenter, uv_topCenter), level }); //top
        quad++;

        quad->position(RectF::FromLTRB(rightCenter, top, right, topCenter), trans); //top-right
        quad->color(Colors::White); //top-right
        quad->texcoords({ RectF::FromLTRB(uv_rightCenter, uv_top, uv_right, uv_topCenter), level }); //top-right
        quad++;

        quad->position(RectF::FromLTRB(left, topCenter, leftCenter, bottomCenter), trans); //left
        quad->color(Colors::White); //left
        quad->texcoords({ RectF::FromLTRB(uv_left, uv_topCenter, uv_leftCenter, uv_bottomCenter), level }); //left
        quad++;

        quad->position(RectF::FromLTRB(leftCenter, topCenter, rightCenter, bottomCenter), trans); //center
        quad->color(Colors::White); //center
        quad->texcoords({ RectF::FromLTRB(uv_leftCenter, uv_topCenter, uv_rightCenter, uv_bottomCenter), level }); //center
        quad++;

        quad->position(RectF::FromLTRB(rightCenter, topCenter, right, bottomCenter), trans); //right
        quad->color(Colors::White); //right
        quad->texcoords({ RectF::FromLTRB(uv_rightCenter, uv_topCenter, uv_right, uv_bottomCenter), level }); //right
        quad++;

        quad->position(RectF::FromLTRB(left, bottomCenter, leftCenter, bottom), trans); //bottom-left
        quad->color(Colors::White); //bottom-left
        quad->texcoords({ RectF::FromLTRB(uv_left, uv_bottomCenter, uv_leftCenter, uv_bottom), level }); //bottom-left
        quad++;

        quad->position(RectF::FromLTRB(leftCenter, bottomCenter, rightCenter, bottom), trans); //bottom
        quad->color(Colors::White); //bottom
        quad->texcoords({ RectF::FromLTRB(uv_leftCenter, uv_bottomCenter, uv_rightCenter, uv_bottom), level }); //bottom
        quad++;

        quad->position(RectF::FromLTRB(rightCenter, bottomCenter, right, bottom), trans); //bottom-right
        quad->color(Colors::White); //bottom-right
        quad->texcoords({ RectF::FromLTRB(uv_rightCenter, uv_bottomCenter, uv_right, uv_bottom), level }); //bottom-right

        _renderer.unmap(9);

        _isDirty = false;
    }
}

void NinePatch::draw(gl::RenderTarget& target)
{
    if (!is_visible() || !_material) {
        return;
    }

    _renderer.render_to_target(target);
}

}
