// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <map>
#include <vector>

#include <tcob/assets/Resource.hpp>
#include <tcob/gfx/Font.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/QuadEffect.hpp>
#include <tcob/gfx/TextFormatter.hpp>
#include <tcob/gfx/Transformable.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderTarget.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>
#include <tcob/gfx/gl/GLUniformBuffer.hpp>
#include <tcob/thirdparty/sigslot/signal.hpp>

namespace tcob {

class Text final : public RectTransformable, public Drawable {
public:
    auto font() -> ResourcePtr<Font>;
    void font(ResourcePtr<Font> font);

    auto text() const -> std::string;
    void text(const std::string& text);

    auto color() const -> Color;
    void color(const Color& color);

    auto background_color() const -> Color;
    void background_color(const Color& color);

    auto outline_thickness() const -> f32;
    void outline_thickness(f32 outline);
    auto outline_color() const -> Color;
    void outline_color(const Color& color);

    auto horizontal_alignment() const -> TextAlignment;
    void horizontal_alignment(TextAlignment align);

    void update(MilliSeconds deltaTime) override;

    void draw(gl::RenderTarget& target) override;

    void register_effect(u8 id, std::shared_ptr<QuadEffectBase> effect);
    void start_all_effects(bool looped = false);
    void stop_all_effects();
    auto get_effect(u8 id) const -> std::shared_ptr<QuadEffectBase>;

private:
    void reshape();
    void format();
    void setup_ubo();

    ResourcePtr<Font> _font;

    std::string _text {};
    Color _color { Colors::White };
    TextAlignment _horiAlignment { TextAlignment::Left };

    bool _needsReshape { true };
    bool _needsFormat { true };

    Color _backgroundColor { 0, 0, 0, 0 };
    Quad _backgroundQuad {};

    std::vector<Quad> _quads {};
    std::vector<TextFormatter::ShaperToken> _tokens {};

    gl::UniformBuffer _uniformBuffer { sizeof(f32) + sizeof(vec4) };
    f32 _outlineThickness { 0 };
    Color _outlineColor { Colors::Black };

    gl::DynamicQuadRenderer _renderer {};
    SizeU _targetSize { SizeU::Zero };

    std::unordered_map<u8, std::shared_ptr<QuadEffectBase>> _textEffects {};

    sigslot::scoped_connection _fontConnection;
    std::vector<sigslot::scoped_connection> _effectConnections;
};
}
