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

class Text final : public Transformable, public Drawable {
public:
    void font(ResourcePtr<Font> font);

    auto text() const -> std::string;
    void text(const std::string& text);

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material);

    auto color() const -> Color; //move to Material
    void color(const Color& color); //move to Material

    auto outline_thickness() const -> f32; //move to Material
    void outline_thickness(f32 outline); //move to Material
    auto outline_color() const -> Color; //move to Material
    void outline_color(const Color& color); //move to Material

    auto horizontal_alignment() const -> TextAlignment;
    void horizontal_alignment(TextAlignment align);

    static inline ResourcePtr<gl::ShaderProgram> DefaultShader;

    void update(f64 deltaTime) override;

    void draw(gl::RenderTarget& target) override;

private:
    void reshape();
    void format(const SizeU& newTargetSize);
    void setup_ubo();

    ResourcePtr<Font> _font;
    ResourcePtr<Material> _material;

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
