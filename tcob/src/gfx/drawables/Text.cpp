// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Text.hpp"

#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/TextFormatter.hpp"

namespace tcob::gfx {

text::text(assets::asset_ptr<font> font)
    : _font {std::move(font)}
{
    _material->Texture = _font->get_texture();
    _renderer.set_material(_material);
    Shader.Changed.connect([&](auto const& value) { _material->Shader = value; });
    Text.Changed.connect([&](auto const&) { _needsReshape = true; });
    Style.Changed.connect([&](auto const&) { _needsReshape = true; });
}

auto text::get_effects() -> quad_tweens&
{
    return _textEffects;
}

void text::on_update(milliseconds deltaTime)
{
    if (!_font.is_ready()) {
        return;
    }

    if (_needsReshape) {
        reshape();
    }

    if (_needsFormat) {
        format();
    }

    if (is_visible()) {
        _textEffects.update(deltaTime);
    }
}

auto text::can_draw() const -> bool
{
    return _font.is_ready() && !_quads.empty();
}

void text::on_draw_to(render_target& target)
{
    _renderer.set_geometry(_quads);
    _renderer.render_to_target(target);
}

void text::on_transform_dirty()
{
    _needsFormat = true;
}

void text::force_reshape()
{
    _needsReshape = true;
}

void text::format()
{
    // clear quads
    _quads.clear();
    _textEffects.clear_quads();

    _needsFormat = false;

    // abort when we have no tokens or target size hasn't been set
    if (_shaperTokens.empty()) {
        return;
    }

    // format text
    auto const size {Bounds->get_size()};
    auto const lines {text_formatter::wrap(_shaperTokens, size.Width)};
    auto const formatResult {text_formatter::format(lines, *_font, Style->Alignment, size.Height)};
    _quads.reserve(formatResult.QuadCount);

    color c {Style->Color};
    u8    alpha {c.A};
    u8    currentEffectIdx {0};

    auto const [x, y] {Bounds->get_position()};
    auto const& xform {get_transform()};
    for (auto const& token : formatResult.Tokens) {
        // handle text commands
        if (token.Command.Type != text_formatter::command_type::None) {
            switch (token.Command.Type) {
            case text_formatter::command_type::Alpha:
                alpha = std::get<u8>(token.Command.Value);
                break;
            case text_formatter::command_type::Color:
                c = std::get<color>(token.Command.Value);
                break;
            case text_formatter::command_type::Effect: {
                currentEffectIdx = std::get<u8>(token.Command.Value);
                if (!_textEffects.has(currentEffectIdx)) {
                    currentEffectIdx = 0;
                }
            } break;
            default:
                break;
            }
        }

        // fill quads
        for (usize i {0}; i < token.Quads.size(); ++i) {
            quad& q {_quads.emplace_back()};

            c.A = alpha;
            geometry::set_color(q, c);

            geometry::set_texcoords(q, token.Quads[i].TexRegion);

            auto const&  posRect {token.Quads[i].Rect};
            rect_f const quadRect {x + posRect.left(), y + posRect.top(), posRect.Width, posRect.Height};
            geometry::set_position(q, quadRect, xform);

            if (currentEffectIdx != 0) {
                _textEffects.add_quad(currentEffectIdx, q);
            }
        }
    }
}

void text::reshape()
{
    _shaperTokens = text_formatter::shape(Text(), *_font, Style->KerningEnabled, false);

    _needsFormat  = true;
    _needsReshape = false;
}
}
