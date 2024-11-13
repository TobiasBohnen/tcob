// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Background.hpp"

#include "tcob/core/random/Random.hpp"

namespace tcob::gfx {

background::background()
{
    Material.Changed.connect([&](auto const& value) {
        _renderer.set_material(value.ptr());
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
    if (Material() && Material->Texture && Material->Texture->has_region(TextureRegion())) {
        geometry::set_texcoords(_quad, Material->Texture->get_region(TextureRegion()));
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
    Material.Changed.connect([&](auto const& value) { _renderer.set_material(value.ptr()); });
}

auto parallax_background::add_layer(parallax_background_layer const& layer) -> uid
{
    uid const id {GetRandomID()};

    _layers.push_back({
        .ID            = id,
        .TextureRegion = layer.TextureRegion,
        .ScrollScale   = layer.ScrollScale,
        .Offset        = layer.Offset,
        .Visible       = true,
    });
    _quads.push_back({});

    return id;
}

auto parallax_background::is_layer_visible(uid id) const -> bool
{
    return get_layer(id)->Visible;
}

void parallax_background::set_layer_visible(uid id, bool visible)
{
    get_layer(id)->Visible = visible;
}

void parallax_background::set_layer_texture(uid id, string const& texture)
{
    get_layer(id)->TextureRegion = texture;
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

            uvRect.Position.X = ((camera.Position.X / texSize.Width) * layer.ScrollScale.Width) + layer.Offset.Width;
            uvRect.Position.Y = ((camera.Position.Y / texSize.Height) * layer.ScrollScale.Height) + layer.Offset.Height;

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

auto parallax_background::get_layer(uid id) -> layer*
{
    for (auto& layer : _layers) {
        if (layer.ID == id) { return &layer; }
    }

    return nullptr;
}

auto parallax_background::get_layer(uid id) const -> layer const*
{
    for (auto const& layer : _layers) {
        if (layer.ID == id) { return &layer; }
    }

    return nullptr;
}

}
