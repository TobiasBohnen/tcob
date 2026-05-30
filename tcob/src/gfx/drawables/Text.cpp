// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Text.hpp"

#include <algorithm>
#include <cstdlib>
#include <utility>
#include <variant>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/TextFormatter.hpp"

namespace tcob::gfx {

enum class command_type : u8 {
    None,
    Color,
    Alpha,
    Effect
};

struct command_definition {
    command_type            Type {command_type::None};
    std::variant<color, u8> Value;
};

struct text_command {
    usize              GlyphIndex {};
    command_definition Command {};
};

static auto parse_commands(utf8_string const& input, utf8_string& stripped) -> std::vector<text_command>
{
    std::vector<text_command> commands;
    stripped.clear();
    stripped.reserve(input.size());

    usize       glyphIndex {0};
    usize       i {0};
    usize const len {input.size()};

    while (i < len) {
        char const ch {input[i]};

        if (ch == '{') {
            if (i + 1 < len && input[i + 1] == '{') {
                stripped += '{';
                ++glyphIndex;
                i += 2;
                continue;
            }

            usize const start {i + 1};
            usize       end {start};
            while (end < len && input[end] != '}') { ++end; }

            if (end >= len) {
                stripped += ch;
                ++glyphIndex;
                ++i;
                continue;
            }

            string_view const inner {input.data() + start, end - start};
            utf8_string const u {utf8::to_upper(inner)};

            command_definition cmd {};
            bool               parsed {true};

            if (u.starts_with("COLOR:")) {
                cmd.Type  = command_type::Color;
                cmd.Value = color::FromString(utf8_string {inner.substr(6)});
            } else if (u.starts_with("ALPHA:")) {
                cmd.Type  = command_type::Alpha;
                cmd.Value = static_cast<u8>(
                    255.0f * std::clamp(std::strtof(utf8_string {inner.substr(6)}.c_str(), nullptr), 0.0f, 1.0f));
            } else if (u.starts_with("EFFECT:")) {
                cmd.Type  = command_type::Effect;
                cmd.Value = static_cast<u8>(
                    std::strtoul(utf8_string {inner.substr(7)}.c_str(), nullptr, 10));
            } else {
                parsed = false;
            }

            if (parsed) {
                commands.push_back({.GlyphIndex = glyphIndex, .Command = cmd});
            } else {
                stripped += '{';
                stripped += utf8_string {inner};
                stripped += '}';
                // count the emitted codepoints
                glyphIndex += 2 + utf8::length(inner);
            }

            i = end + 1;
        } else if ((ch & 0x80) == 0) {
            // ASCII — one codepoint
            stripped += ch;
            if (ch != '\r' && ch != '\n') { ++glyphIndex; }
            ++i;
        } else {
            // multi-byte UTF-8 sequence — copy all bytes, count one codepoint
            stripped += ch;
            ++i;
            while (i < len && (input[i] & 0xC0) == 0x80) {
                stripped += input[i];
                ++i;
            }
            ++glyphIndex;
        }
    }

    return commands;
}

text::text(asset_ptr<font> font)
    : _font {std::move(font)}
{
    _font->TextureResized.connect([this] { _needsFormat = true; });

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
    _quads.clear();
    _inds.clear();
    Effects.clear_quads();

    _needsFormat = false;

    if (Text->empty()) { return; }

    auto const size {Bounds->Size};
    if (size.Width == 0 || size.Height == 0) { return; }

    utf8_string stripped;
    auto const  commands {parse_commands(*Text, stripped)};
    auto const  formatResult {text_formatter::format(stripped, *_font, Style->Alignment, size, 1.0f, Style->KerningEnabled)};
    _quads.reserve(formatResult.QuadCount);

    color col {Style->Color};
    u8    alpha {col.A};
    u8    currentEffectIdx {0};

    auto const [x, y] {Bounds->Position};

    usize       glyphIndex {0};
    usize       cmdIdx {0};
    usize const cmdLen {commands.size()};

    auto const applyCommands {[&]() {
        while (cmdIdx < cmdLen && commands[cmdIdx].GlyphIndex == glyphIndex) {
            switch (commands[cmdIdx].Command.Type) {
            case command_type::Alpha:  alpha = std::get<u8>(commands[cmdIdx].Command.Value); break;
            case command_type::Color:  col = std::get<color>(commands[cmdIdx].Command.Value); break;
            case command_type::Effect: {
                currentEffectIdx = std::get<u8>(commands[cmdIdx].Command.Value);
                if (!Effects.has(currentEffectIdx)) { currentEffectIdx = 0; }
            } break;
            default: break;
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

            if (currentEffectIdx != 0) {
                Effects.add_quad(currentEffectIdx, q);
            }

            ++glyphIndex;
        }
    }

    _inds = geometry::get_indices(_quads.size());
}

}
