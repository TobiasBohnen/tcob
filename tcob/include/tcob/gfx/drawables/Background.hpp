// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Property.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API background : public drawable {
public:
    background();

    prop<assets::asset_ptr<material>> Material;
    prop<string>                      TextureRegion {"default"};

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) final;

private:
    quad          _quad;
    quad_renderer _renderer {buffer_usage_hint::StreamDraw};
};

////////////////////////////////////////////////////////////

class TCOB_API parallax_background final : public drawable {
public:
    parallax_background();

    prop<assets::asset_ptr<material>> Material;
    size_f                            TextureScale {size_f::One};

    void add_layer(string const& textureRegion, f32 factor);

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) final;

private:
    struct layer {
        string TextureRegion;
        f32    Factor {};
    };

    std::vector<layer> _layers;
    std::vector<quad>  _quads;
    quad_renderer      _renderer {buffer_usage_hint::StreamDraw};
};

}
