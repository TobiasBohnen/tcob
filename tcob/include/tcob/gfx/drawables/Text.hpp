// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Property.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Transformable.hpp"
#include "tcob/gfx/animation/QuadTween.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API text final : public transformable, public drawable {
public:
    struct style {
        color      Color {colors::White};
        alignments Alignment {};
        bool       KerningEnabled {true};

        auto operator==(style const& other) const -> bool = default;
    };

    explicit text(assets::asset_ptr<font> font);

    prop<rect_f>                 Bounds;
    prop<std::optional<point_f>> Pivot;

    prop<utf8_string>               Text;
    prop<style>                     Style;
    prop<assets::asset_ptr<shader>> Shader;
    quad_tweens                     Effects;

    void force_reshape();

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

    auto get_pivot() const -> point_f override;
    void on_transform_changed() override;

private:
    void reshape();
    void format();

    bool _needsReshape {true};
    bool _needsFormat {true};

    std::vector<quad> _quads {};

    quad_renderer                      _renderer {buffer_usage_hint::DynamicDraw};
    assets::manual_asset_ptr<material> _material {};

    assets::asset_ptr<font> _font;
};

}
