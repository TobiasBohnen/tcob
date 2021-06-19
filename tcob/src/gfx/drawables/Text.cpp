// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/drawables/Text.hpp>

#include <cstring>

#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/gl/GLShaderProgram.hpp>

namespace tcob {

void TextEffect::add_quad(isize idx, Quad q)
{
    _quads[idx] = q;
}

////////////////////////////////////////////////////////////

auto Text::font() -> ResourcePtr<Font>
{
    return _font;
}

void Text::font(ResourcePtr<Font> font)
{
    _font = std::move(font);
    _needsReshape = true;

    if (_font)
        _connection = _font->Changed.connect([=]() { _needsReshape = true; });
}

auto Text::text() const -> std::string
{
    return _text;
}

void Text::text(const std::string& text)
{
    _text = text;
    _needsReshape = true;
}

auto Text::color() const -> Color
{
    return _color;
}

void Text::color(const Color& color)
{
    _color = color;
    for (auto& quad : _quads) {
        quad.color(color);
    }
}

auto Text::outline_thickness() const -> f32
{
    return _outlineThickness;
}

void Text::outline_thickness(f32 outline)
{
    _outlineThickness = std::clamp(outline, 0.0f, 1.0f);
    setup_ubo();
}

auto Text::outline_color() const -> Color
{
    return _outlineColor;
}

void Text::outline_color(const Color& color)
{
    _outlineColor = color;
    setup_ubo();
}

auto Text::horizontal_alignment() const -> TextAlignment
{
    return _horiAlignment;
}

void Text::horizontal_alignment(TextAlignment align)
{
    if (_horiAlignment != align) {
        _horiAlignment = align;
        _needsFormat = true;
    }
}

void Text::update([[maybe_unused]] MilliSeconds deltaTime)
{
    if (!_font && Font::Default) {
        font(Font::Default);
    }

    if (_font) {
        if (_needsReshape) {
            reshape();
        }

        if (is_transform_dirty()) {
            _needsFormat = true;
        }
    } else {
        _needsReshape = false;
        _needsFormat = false;
    }
}

void Text::draw(gl::RenderTarget& target)
{
    if (!is_visible() || !_font)
        return;

    if (_needsFormat) {
        format(target.size());

        const isize size { _quads.size() };
        Quad* quad { _renderer.map(size) };
        std::memcpy(quad, _quads.data(), size * sizeof(Quad));
        _renderer.unmap(size);
    }

    _uniformBuffer.bind_base(1);
    _renderer.material(_font->material().object());
    _renderer.render_to_target(target);
}

void Text::register_event(u8 id, std::shared_ptr<TextEffect> effect)
{
    if (id == 0) {
        //TODO: log error
        return;
    }

    _textEffects[id] = effect;
}

void Text::format(const SizeU& newTargetSize)
{
    if (!_tokens.empty() || _oldTargetSize != newTargetSize) {
        const f32 ty { static_cast<f32>(newTargetSize.Height) };

        const auto formatResult { TextFormatter::format(_tokens, _font->info(), _horiAlignment, { size().Width * ty, size().Height * ty }) };

        _quads.clear();

        Color color { _color };
        u8 alpha { color.A };
        std::shared_ptr<TextEffect> currentEffect;

        const auto [x, y] { position() };
        for (const auto& token : formatResult.Tokens) {
            if (token.Command.Type != TextFormatter::CommandType::None) {
                switch (token.Command.Type) {
                case TextFormatter::CommandType::Alpha:
                    alpha = std::get<u8>(token.Command.Value);
                    break;
                case TextFormatter::CommandType::Color:
                    color = std::get<Color>(token.Command.Value);
                    break;
                case TextFormatter::CommandType::Effect: {
                    u8 idx { std::get<u8>(token.Command.Value) };
                    if (idx != 0)
                        currentEffect = _textEffects[idx];
                    else
                        currentEffect = nullptr;
                } break;
                default:
                    break;
                }
            }

            for (isize i { 0 }; i < token.Quads.size(); ++i) {
                Quad& q { _quads.emplace_back() };

                color.A = alpha;
                q.color(color);

                q.texcoords(token.Quads[i].TexRegion);

                auto& posRect { token.Quads[i].Rect };
                RectF quadRect { x + posRect.Left / ty, y + posRect.Top / ty, posRect.Width / ty, posRect.Height / ty };
                q.position(quadRect, transform());

                if (currentEffect) {
                    currentEffect->add_quad(_quads.size() - 1, q);
                }
            }
        }

        _oldTargetSize = newTargetSize;
        _needsFormat = false;

        setup_ubo();
    }
}

void Text::reshape()
{
    if (_font && !_text.empty()) {
        _tokens = TextFormatter::shape(_text, _font);
    } else {
        _tokens.clear();
    }

    _needsFormat = true;
    _needsReshape = false;
}

void Text::setup_ubo()
{
    const std::array<f32, 4> outlineColor { _outlineColor.R / 255.f, _outlineColor.G / 255.f, _outlineColor.B / 255.f, _outlineColor.A / 255.f };
    _uniformBuffer.update(outlineColor.data(), sizeof(outlineColor), 0);
    const f32 outline = (1 - _outlineThickness) * 0.5f;
    _uniformBuffer.update(&outline, sizeof(outline), sizeof(outlineColor));
}

}