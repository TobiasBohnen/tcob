// Copyright (c) 2024 Tobias Bohnen
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
    Bounds.Changed.connect([&](auto const&) { mark_transform_dirty(); });
    Pivot.Changed.connect([&](auto const&) { mark_transform_dirty(); });

    _material->Texture = _font->get_texture();
    _renderer.set_material(_material.get_ptr());
    Shader.Changed.connect([&](auto const& value) { _material->Shader = value; });
    Text.Changed.connect([&](auto const&) { _needsReshape = true; });
    Style.Changed.connect([&](auto const&) { _needsReshape = true; });
}

void text::on_update(milliseconds deltaTime)
{
    if (!_font.is_ready()) { return; }

    if (_needsReshape) { reshape(); }

    if (_needsFormat) { format(); }

    if (is_visible()) {
        Effects.update(deltaTime);
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

auto text::get_pivot() const -> point_f
{
    if (Pivot().has_value()) {
        return Bounds->top_left() + *Pivot();
    }

    return Bounds->get_center();
}

void text::on_transform_changed()
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
    Effects.clear_quads();

    _needsFormat = false;

    if (Text->empty()) { return; }

    // format text
    auto const size {Bounds->Size};
    auto const formatResult {text_formatter::format(Text(), *_font, Style->Alignment, size, 1.0f, Style->KerningEnabled)};
    _quads.reserve(formatResult.QuadCount);

    color c {Style->Color};
    u8    alpha {c.A};
    u8    currentEffectIdx {0};

    auto const [x, y] {Bounds->Position};
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
                if (!Effects.has(currentEffectIdx)) {
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
            rect_f const quadRect {x + posRect.left(), y + posRect.top(), posRect.width(), posRect.height()};
            geometry::set_position(q, quadRect, xform);

            if (currentEffectIdx != 0) {
                Effects.add_quad(currentEffectIdx, q);
            }
        }
    }
}

void text::reshape()
{
    _needsFormat  = true;
    _needsReshape = false;
}
}
