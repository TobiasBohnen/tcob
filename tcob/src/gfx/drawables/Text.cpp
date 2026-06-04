// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Text.hpp"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/FontFamily.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/TextFormatter.hpp"

namespace tcob::gfx {

text::text(asset_ptr<font_family> font)
    : _font {std::move(font)}
{
    _font->TextureResized.connect([this] { _needsFormat = true; });

    Bounds.Changed.connect([this](auto const&) { mark_transform_dirty(); });
    Pivot.Changed.connect([this](auto const&) { mark_transform_dirty(); });

    Shader.Changed.connect([this](auto const& value) { _material->first_pass().Shader = value; });
    Text.Changed.connect([this](auto const&) {
        parse_commands();
        _needsFormat = true;
    });
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

void text::on_draw_to(render_target& target, transform const& xform)
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
    _needsFormat = false;

    _quads.clear();
    _inds.clear();
    Effects.clear_groups();

    if (Text->empty()) { return; }

    auto const size {Bounds->Size};
    if (size.Width == 0 || size.Height == 0) { return; }

    auto const currentFont {_font->get_font(Style->FontStyle, Style->FontSize)};
    auto const formatResult {text_formatter::format(_strippedText, *currentFont, Style->Alignment, size, 1.0f, Style->KerningEnabled)};
    _quads.reserve(formatResult.QuadCount);
    _material->first_pass().Texture = currentFont->texture();

    color             col {Style->Color};
    u8                alpha {col.A};
    std::optional<u8> currentEffectIdx {};

    auto const [x, y] {Bounds->Position};

    usize       glyphIndex {0};
    usize       cmdIdx {0};
    usize const cmdLen {_commands.size()};

    auto const applyCommands {[&]() {
        while (cmdIdx < cmdLen && _commands[cmdIdx].GlyphIndex == glyphIndex) {
            switch (_commands[cmdIdx].Type) {
            case text_command_type::Alpha:    alpha = std::get<u8>(_commands[cmdIdx].Value); break;
            case text_command_type::AlphaOff: alpha = 255; break;
            case text_command_type::Color:    col = std::get<color>(_commands[cmdIdx].Value); break;
            case text_command_type::ColorOff: col = Style->Color; break;
            case text_command_type::Effect:   {
                currentEffectIdx = std::get<u8>(_commands[cmdIdx].Value);
                if (!Effects.has(*currentEffectIdx)) { currentEffectIdx = std::nullopt; }
            } break;
            case text_command_type::EffectOff: currentEffectIdx = std::nullopt; break;
            case text_command_type::None:      break;
            }
            ++cmdIdx;
        }
    }};

    // setup quads
    for (auto const& token : formatResult.Tokens) {
        for (usize i {0}; i < token.Quads.size(); ++i) {
            applyCommands();
            quad& q {_quads.emplace_back()};

            col.A = alpha;
            geometry::set_color(q, col);

            geometry::set_texcoords(q, token.Quads[i].TextureRegion);

            auto const&  posRect {token.Quads[i].Rect};
            rect_f const quadRect {x + posRect.left(), y + posRect.top(), posRect.width(), posRect.height()};
            geometry::set_position(q, quadRect);

            if (currentEffectIdx) {
                Effects.add_group(*currentEffectIdx, q);
            }

            ++glyphIndex;
        }
    }

    _inds = geometry::get_indices(_quads.size());
}

void text::parse_commands()
{
    auto const& input {*Text};
    _commands.clear();
    _strippedText.clear();
    _strippedText.reserve(input.size());

    usize       glyphIndex {0};
    usize       i {0};
    usize const len {input.size()};

    while (i < len) {
        char const ch {input[i]};

        if (ch == '{') {
            if (i + 1 < len && input[i + 1] == '{') {
                _strippedText += '{';
                ++glyphIndex;
                i += 2;
                continue;
            }

            usize const start {i + 1};
            usize       end {start};
            while (end < len && input[end] != '}') { ++end; }

            if (end >= len) {
                _strippedText += ch;
                ++glyphIndex;
                ++i;
                continue;
            }

            string_view const inner {input.data() + start, end - start};
            utf8_string const u {utf8::to_upper(inner)};

            text_command cmd {};
            bool         parsed {true};

            if (u.starts_with("/")) {
                if (u.starts_with("/COLOR")) {
                    cmd.Type = text_command_type::ColorOff;
                } else if (u.starts_with("/ALPHA")) {
                    cmd.Type = text_command_type::AlphaOff;
                } else if (u.starts_with("/EFFECT")) {
                    cmd.Type = text_command_type::EffectOff;
                } else {
                    parsed = false;
                }
            } else {
                if (u.starts_with("COLOR:")) {
                    cmd.Type  = text_command_type::Color;
                    cmd.Value = color::FromString(utf8_string {inner.substr(6)});
                } else if (u.starts_with("ALPHA:")) {
                    cmd.Type  = text_command_type::Alpha;
                    cmd.Value = static_cast<u8>(255.0f * std::clamp(std::strtof(utf8_string {inner.substr(6)}.c_str(), nullptr), 0.0f, 1.0f));
                } else if (u.starts_with("EFFECT:")) {
                    cmd.Type  = text_command_type::Effect;
                    cmd.Value = static_cast<u8>(std::strtoul(utf8_string {inner.substr(7)}.c_str(), nullptr, 10));
                } else {
                    parsed = false;
                }
            }

            if (parsed) {
                cmd.GlyphIndex = glyphIndex;
                _commands.push_back(cmd);
            } else {
                _strippedText += '{';
                _strippedText += utf8_string {inner};
                _strippedText += '}';
                // count the emitted codepoints
                glyphIndex += 2 + utf8::length(inner);
            }

            i = end + 1;
        } else if ((ch & 0x80) == 0) {
            // ASCII — one codepoint
            _strippedText += ch;
            if (ch != '\r' && ch != '\n') { ++glyphIndex; }
            ++i;
        } else {
            // multi-byte UTF-8 sequence — copy all bytes, count one codepoint
            _strippedText += ch;
            ++i;
            while (i < len && (input[i] & 0xC0) == 0x80) {
                _strippedText += input[i];
                ++i;
            }
            ++glyphIndex;
        }
    }
}

}
