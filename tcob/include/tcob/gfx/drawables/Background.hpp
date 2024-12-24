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
    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) final;

private:
    quad          _quad;
    quad_renderer _renderer {buffer_usage_hint::StreamDraw};
};

////////////////////////////////////////////////////////////

struct parallax_background_layer {
    string TextureRegion;
    size_f ScrollScale {size_f::One};
    size_f Offset {size_f::Zero};
};

////////////////////////////////////////////////////////////

class TCOB_API parallax_background final : public drawable {
public:
    parallax_background();

    prop<assets::asset_ptr<material>> Material;
    size_f                            TextureScale {size_f::One};

    auto add_layer(parallax_background_layer const& layer) -> uid;

    auto is_layer_visible(uid id) const -> bool;
    void set_layer_visible(uid id, bool visible);
    void set_layer_texture(uid id, string const& texture);

protected:
    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) final;

private:
    struct layer {
        uid    ID {INVALID_ID};
        string TextureRegion;
        size_f ScrollScale;
        size_f Offset;
        bool   Visible {true};
    };

    auto get_layer(uid id) -> layer*;
    auto get_layer(uid id) const -> layer const*;

    std::vector<layer> _layers;
    std::vector<quad>  _quads;
    quad_renderer      _renderer {buffer_usage_hint::StreamDraw};
};

}
