// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Background.hpp"

namespace tcob::gfx {

background::background()
{
    Material.Changed.connect([&](auto const& value) {
        _renderer.set_material(value.get_ptr());
        TextureRegion("default");
    });
}

void background::on_update(milliseconds /* deltaTime */)
{
}

auto background::can_draw() const -> bool
{
    return !Material().is_expired();
}

void background::on_draw_to(render_target& target)
{
    geometry::set_position(_quad, {point_f::Zero, size_f {target.Size()}});
    geometry::set_color(_quad, colors::White);
    if (Material() && Material->Texture && Material->Texture->has_region(TextureRegion)) {
        geometry::set_texcoords(_quad, Material->Texture->get_region(TextureRegion));
    } else {
        geometry::set_texcoords(_quad, {{0, 0, 1, 1}, 1});
    }

    _renderer.set_geometry(_quad);

    target.get_camera().push_state();
    _renderer.render_to_target(target);
    target.get_camera().pop_state();
}

////////////////////////////////////////////////////////////

parallax_background::parallax_background()
{
    Material.Changed.connect([&](auto const& value) { _renderer.set_material(value.get_ptr()); });
}

void parallax_background::add_layer(string const& textureRegion, f32 factor)
{
    _layers.emplace_back(textureRegion, factor);
    _quads.push_back({});
}

void parallax_background::on_update(milliseconds /* deltaTime */)
{
}

auto parallax_background::can_draw() const -> bool
{
    return !Material().is_expired();
}

void parallax_background::on_draw_to(render_target& target)
{
    auto const& camera {target.get_camera()};

    auto const targetSize {size_f {target.Size()}};
    auto const texSize {size_f {Material->Texture->get_size()} * TextureScale};

    for (usize i {0}; i < _layers.size(); ++i) {
        auto const& layer {_layers[i]};
        auto&       quad {_quads[i]};

        geometry::set_position(quad, {point_f::Zero, size_f {target.Size()}});
        geometry::set_color(quad, colors::White);

        if (Material() && Material->Texture && Material->Texture->has_region(layer.TextureRegion)) {
            auto  texReg {Material->Texture->get_region(layer.TextureRegion)};
            auto& uvRect {texReg.UVRect};
            uvRect.Size *= (targetSize / texSize);

            uvRect.Position.X = (camera.Position.X / texSize.Width * layer.Factor);
            uvRect.Position.Y = (camera.Position.Y / texSize.Height * layer.Factor);

            geometry::set_texcoords(quad, texReg);
        } else {
            geometry::set_texcoords(quad, {{0, 0, 1, 1}, 1});
        }
    }

    _renderer.set_geometry(_quads);

    target.get_camera().push_state();
    _renderer.render_to_target(target);
    target.get_camera().pop_state();
}

}
