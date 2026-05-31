// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/FontFamily.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Transformable.hpp"
#include "tcob/gfx/animation/VertexTween.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API text final : public transformable, public drawable, public updatable {
public:
    struct style {
        color       Color {colors::White};
        alignment   Alignment {};
        bool        KerningEnabled {true};
        font::style FontStyle {};
        u32         FontSize {32};

        auto operator==(style const& other) const -> bool = default;
    };

    explicit text(asset_ptr<font_family> font);

    prop<rect_f>                 Bounds;
    prop<std::optional<point_f>> Pivot;

    prop<utf8_string>       Text;
    prop<style>             Style;
    prop<asset_ptr<shader>> Shader;

    vertex_tweens Effects;

    void force_reshape();

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target, transform const& xform) override;

    auto pivot() const -> point_f override;
    void on_transform_changed() override;

private:
    void format();

    bool _needsFormat {true};

    std::vector<quad> _quads {};
    std::vector<u32>  _inds {};

    renderer                  _renderer {buffer_usage_hint::DynamicDraw};
    asset_owner_ptr<material> _material {};

    asset_ptr<font_family> _font;
};

}
