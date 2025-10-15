// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Background.hpp"

#include <memory>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {

background::background()
{
    Material.Changed.connect([this](auto const&) {
        TextureRegion("default");
    });
}

auto background::can_draw() const -> bool
{
    return !(*Material).is_expired();
}

void background::on_draw_to(render_target& target)
{
    target.camera().push_state();

    geometry::set_position(_quad, {point_f::Zero, size_f {*target.Size}});
    geometry::set_color(_quad, colors::White);

    for (isize i {0}; i < Material->pass_count(); ++i) {
        auto const& pass {Material->get_pass(i)};

        geometry::set_texcoords(_quad, pass, TextureRegion);

        _renderer.set_geometry(_quad, &pass);
        _renderer.render_to_target(target);
    }

    target.camera().pop_state();
}

////////////////////////////////////////////////////////////

parallax_background::parallax_background() = default;

auto parallax_background::create_layer() -> parallax_background_layer&
{
    return *_layers.emplace_back(std::make_unique<parallax_background_layer>());
}

void parallax_background::remove_layer(parallax_background_layer const& layer)
{
    helper::erase_first(_layers, [&layer](auto const& val) { return val.get() == &layer; });
}

void parallax_background::clear()
{
    _layers.clear();
}

auto parallax_background::can_draw() const -> bool
{
    return !(*Material).is_expired();
}

void parallax_background::on_draw_to(render_target& target)
{
    auto const cameraPos {target.camera().Position};
    target.camera().push_state();

    _quads.resize(_layers.size());
    auto const targetSize {size_f {*target.Size}};

    for (isize i {0}; i < Material->pass_count(); ++i) {
        auto const& pass {Material->get_pass(i)};

        auto const texSize {size_f {pass.Texture->info().Size} * TextureScale};

        for (usize i {0}; i < _layers.size(); ++i) {
            auto const& layer {*_layers[i]};
            if (!layer.Visible) { continue; }
            auto& quad {_quads[i]};

            geometry::set_position(quad, {point_f::Zero, size_f {*target.Size}});
            geometry::set_color(quad, colors::White);

            if (pass.Texture && pass.Texture->regions().contains(layer.TextureRegion)) {
                auto  texReg {pass.Texture->regions()[layer.TextureRegion]};
                auto& uvRect {texReg.UVRect};
                uvRect.Size *= (targetSize / texSize);

                uvRect.Position.X = ((cameraPos.X / texSize.Width) * layer.ScrollScale.Width) + layer.Offset.Width;
                uvRect.Position.Y = ((cameraPos.Y / texSize.Height) * layer.ScrollScale.Height) + layer.Offset.Height;
                geometry::set_texcoords(quad, texReg);
            } else {
                geometry::set_texcoords(quad, {.UVRect = {0, 0, 1, 1}, .Level = 0});
            }
        }

        _renderer.set_geometry(_quads, &pass);
        _renderer.render_to_target(target);
    }

    target.camera().pop_state();
}

}
