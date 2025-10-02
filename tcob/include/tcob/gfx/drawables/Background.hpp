// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API background : public drawable {
public:
    background();

    prop<asset_ptr<material>> Material;
    prop<string>              TextureRegion {"default"};

protected:
    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) final;

private:
    quad          _quad;
    quad_renderer _renderer {buffer_usage_hint::StreamDraw};
};

////////////////////////////////////////////////////////////

class TCOB_API parallax_background_layer final : public non_copyable {
public:
    parallax_background_layer() = default;

    string TextureRegion;
    size_f ScrollScale {size_f::One};
    size_f Offset {size_f::Zero};
    bool   Visible {true};
};

////////////////////////////////////////////////////////////

class TCOB_API parallax_background final : public drawable {
public:
    parallax_background();

    prop<asset_ptr<material>> Material;
    size_f                    TextureScale {size_f::One};

    auto create_layer() -> parallax_background_layer&;
    void remove_layer(parallax_background_layer const& layer);
    void clear();

protected:
    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) final;

private:
    std::vector<std::unique_ptr<parallax_background_layer>> _layers {};

    std::vector<quad> _quads;
    quad_renderer     _renderer {buffer_usage_hint::StreamDraw};
};

}
