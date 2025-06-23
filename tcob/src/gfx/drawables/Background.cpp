// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Background.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {

background::background()
{
    Material.Changed.connect([this](auto const& value) {
        _renderer.set_material(value.ptr());
        TextureRegion("default");
    });
}

auto background::can_draw() const -> bool
{
    return !(*Material).is_expired();
}

void background::on_draw_to(render_target& target)
{
    geometry::set_position(_quad, {point_f::Zero, size_f {*target.Size}});
    geometry::set_color(_quad, colors::White);
    if (*Material && Material->Texture && Material->Texture->has_region(TextureRegion)) {
        geometry::set_texcoords(_quad, Material->Texture->get_region(TextureRegion));
    } else {
        geometry::set_texcoords(_quad, {.UVRect = {0, 0, 1, 1}, .Level = 1});
    }

    _renderer.set_geometry(_quad);

    target.camera().push_state();
    _renderer.render_to_target(target);
    target.camera().pop_state();
}

////////////////////////////////////////////////////////////

parallax_background::parallax_background()
{
    Material.Changed.connect([this](auto const& value) { _renderer.set_material(value.ptr()); });
}

auto parallax_background::add_layer(parallax_background_layer const& layer) -> uid
{
    uid const id {get_random_ID()};

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

auto parallax_background::is_layer_visible(uid layerId) const -> bool
{
    if (auto const* layer {get_layer(layerId)}) {
        return layer->Visible;
    }
    return false;
}

void parallax_background::show_layer(uid layerId)
{
    if (auto* layer {get_layer(layerId)}) {
        if (!layer->Visible) {
            layer->Visible = true;
        }
    }
}

void parallax_background::hide_layer(uid layerId)
{
    if (auto* layer {get_layer(layerId)}) {
        if (layer->Visible) {
            layer->Visible = false;
        }
    }
}

void parallax_background::set_layer_texture(uid layerId, string const& texture)
{
    if (auto* layer {get_layer(layerId)}) {
        layer->TextureRegion = texture;
    }
}

auto parallax_background::can_draw() const -> bool
{
    return !(*Material).is_expired();
}

void parallax_background::on_draw_to(render_target& target)
{
    auto const& camera {target.camera()};

    auto const targetSize {size_f {*target.Size}};
    auto const texSize {size_f {Material->Texture->info().Size} * TextureScale};

    for (usize i {0}; i < _layers.size(); ++i) {
        auto const& layer {_layers[i]};
        auto&       quad {_quads[i]};

        geometry::set_position(quad, {point_f::Zero, size_f {*target.Size}});
        geometry::set_color(quad, colors::White);

        if (*Material && Material->Texture && Material->Texture->has_region(layer.TextureRegion)) {
            auto  texReg {Material->Texture->get_region(layer.TextureRegion)};
            auto& uvRect {texReg.UVRect};
            uvRect.Size *= (targetSize / texSize);

            uvRect.Position.X = ((camera.Position.X / texSize.Width) * layer.ScrollScale.Width) + layer.Offset.Width;
            uvRect.Position.Y = ((camera.Position.Y / texSize.Height) * layer.ScrollScale.Height) + layer.Offset.Height;

            geometry::set_texcoords(quad, texReg);
        } else {
            geometry::set_texcoords(quad, {.UVRect = {0, 0, 1, 1}, .Level = 1});
        }
    }

    _renderer.set_geometry(_quads);

    target.camera().push_state();
    _renderer.render_to_target(target);
    target.camera().pop_state();
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
