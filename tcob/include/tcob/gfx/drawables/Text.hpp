// Copyright (c) 2023 Tobias Bohnen
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
#include "tcob/gfx/QuadTween.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Transformable.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API text final : public rect_transformable, public drawable {
public:
    struct style {
        color      Color {colors::White};
        alignments Alignment {};
        bool       KerningEnabled {true};
    };

    explicit text(assets::asset_ptr<font> font);

    prop<utf8_string>               Text;
    prop<style>                     Style;
    prop<assets::asset_ptr<shader>> Shader;

    auto get_effects() -> quad_tweens&;

    void force_reshape();

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

    void on_transform_dirty() override;

private:
    void reshape();
    void format();

    bool _needsReshape {true};
    bool _needsFormat {true};

    std::vector<quad> _quads {};

    quad_renderer                      _renderer {buffer_usage_hint::DynamicDraw};
    assets::manual_asset_ptr<material> _material {};

    quad_tweens             _textEffects {};
    assets::asset_ptr<font> _font;
};

inline auto operator==(text::style const& left, text::style const& right) -> bool
{
    return (left.Color == right.Color)
        && (left.Alignment == right.Alignment)
        && (left.KerningEnabled == right.KerningEnabled);
}

}
