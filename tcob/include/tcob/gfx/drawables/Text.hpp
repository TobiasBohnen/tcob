// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/Resource.hpp>
#include <tcob/gfx/Font.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/TextFormatter.hpp>
#include <tcob/gfx/Transformable.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderTarget.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>
#include <tcob/gfx/gl/GLUniformBuffer.hpp>

namespace tcob {
struct TextEffect {
};

////////////////////////////////////////////////////////////

class Text final : public RectTransformable, public Drawable {
public:
    auto font() -> ResourcePtr<Font>;
    void font(ResourcePtr<Font> font);

    auto text() const -> std::string;
    void text(const std::string& text);

    auto color() const -> Color;
    void color(const Color& color);

    auto outline_thickness() const -> f32;
    void outline_thickness(f32 outline);
    auto outline_color() const -> Color;
    void outline_color(const Color& color);

    auto horizontal_alignment() const -> TextAlignment;
    void horizontal_alignment(TextAlignment align);

    void update(MilliSeconds deltaTime) override;

    void draw(gl::RenderTarget& target) override;

    void reshape();

private:
    void format(const SizeU& newTargetSize);
    void setup_ubo();

    ResourcePtr<Font> _font;

    std::string _text;
    Color _color { Colors::White };
    TextAlignment _horiAlignment { TextAlignment::Left };

    bool _needsReshape { true };
    bool _needsFormat { true };

    std::vector<Quad> _quads;
    std::vector<TextFormatter::ShaperToken> _tokens;

    gl::UniformBuffer _uniformBuffer { sizeof(f32) + sizeof(vec4) };
    f32 _outlineThickness { 0 };
    Color _outlineColor { Colors::Black };

    gl::DynamicQuadRenderer _renderer {};
    SizeU _oldTargetSize { SizeU::Zero };
};
}
