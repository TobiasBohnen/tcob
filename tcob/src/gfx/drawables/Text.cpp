// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Text.hpp"

#include <utility>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/TextFormatter.hpp"

namespace tcob::gfx {

text::text(asset_ptr<font> font)
    : _font {std::move(font)}
{
    _font->TextureResized.connect([this]() { _needsFormat = true; });

    Bounds.Changed.connect([this](auto const&) { mark_transform_dirty(); });
    Pivot.Changed.connect([this](auto const&) { mark_transform_dirty(); });

    _material->first_pass().Texture = _font->texture();
    Shader.Changed.connect([this](auto const& value) { _material->first_pass().Shader = value; });
    Text.Changed.connect([this](auto const&) { _needsFormat = true; });
    Style.Changed.connect([this](auto const&) { _needsFormat = true; });
}

void text::on_update(milliseconds deltaTime)
{
    if (!_font.is_ready()) { return; }
    if (_needsFormat) { format(); }
    if (is_visible()) { Effects.update(deltaTime); }
}

auto text::can_draw() const -> bool
{
    return _font.is_ready() && !_quads.empty();
}

void text::on_draw_to(render_target& target, transform& xform)
{
    _renderer.set_geometry(
        {.Vertices = geometry::flatten(_quads), .Indices = _inds, .Type = primitive_type::Triangles},
        &_material->first_pass());
    _renderer.render_to_target(target, xform * get_transform());
}

auto text::pivot() const -> point_f
{
    if ((*Pivot).has_value()) {
        return Bounds->Position + **Pivot;
    }

    return Bounds->center();
}

void text::on_transform_changed()
{
    _needsFormat = true;
}

void text::force_reshape()
{
    _needsFormat = true;
}

void text::format()
{
    _quads.clear();
    _inds.clear();
    Effects.clear_quads();

    _needsFormat = false;

    if (Text->empty()) { return; }

    auto const size {Bounds->Size};
    auto const formatResult {text_formatter::format(*Text, *_font, Style->Alignment, size, 1.0f, Style->KerningEnabled, true)};
    _quads.reserve(formatResult.QuadCount);

    color col {Style->Color};
    u8    alpha {col.A};
    u8    currentEffectIdx {0};

    auto const [x, y] {Bounds->Position};

    for (auto const& token : formatResult.Tokens) {
        if (token.Command.Type != text_formatter::command_type::None) { // handle text commands
            switch (token.Command.Type) {
            case text_formatter::command_type::Alpha:  alpha = std::get<u8>(token.Command.Value); break;
            case text_formatter::command_type::Color:  col = std::get<color>(token.Command.Value); break;
            case text_formatter::command_type::Effect: {
                currentEffectIdx = std::get<u8>(token.Command.Value);
                if (!Effects.has(currentEffectIdx)) { currentEffectIdx = 0; }
            } break;
            default:
                break;
            }
        }

        // setup quads
        for (usize i {0}; i < token.Quads.size(); ++i) {
            quad& q {_quads.emplace_back()};

            col.A = alpha;
            geometry::set_color(q, col);

            geometry::set_texcoords(q, token.Quads[i].TextureRegion);

            auto const&  posRect {token.Quads[i].Rect};
            rect_f const quadRect {x + posRect.left(), y + posRect.top(), posRect.width(), posRect.height()};
            geometry::set_position(q, quadRect);

            if (currentEffectIdx != 0) {
                Effects.add_quad(currentEffectIdx, q);
            }
        }
    }

    _inds = geometry::get_indices(_quads.size());
}

}
