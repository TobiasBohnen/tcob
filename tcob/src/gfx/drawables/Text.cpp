// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Text.hpp"

#include <algorithm>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
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

    if (_isRunning && is_visible()) {
        _elapsedTime += deltaTime;

        for (auto& [_, effect] : _effects) {
            effect->update(deltaTime);
        }
    }
}

auto text::can_draw() const -> bool
{
    return _font.is_ready() && !_quads.empty();
}

void text::on_draw_to(render_target& target, transform const& xform)
{
    std::span<u32 const> inds;
    if (!_hasWait) {
        inds = _inds;
    } else {
        if (!_isRunning) { return; }

        milliseconds timeStamp {};
        usize        gi {0};
        for (auto const& wp : _commands) {
            if (wp.Type == text_command_type::Effect) {
                auto const id {std::get<u8>(wp.Value)};
                if (_effects.contains(id) && !_startedEffects.contains(id)) {
                    _startedEffects.insert(id);
                    _effects.at(id)->start(_playbackMode);
                }
                continue;
            }
            if (wp.Type != text_command_type::Wait) { continue; }
            gi = wp.GlyphIndex;
            timeStamp += std::get<milliseconds>(wp.Value);
            if (_elapsedTime < timeStamp) { break; }
        }

        inds = {_inds.begin(), _inds.begin() + (6 * gi)};
    }

    if (inds.empty()) { return; }

    _renderer.set_geometry(
        {.Vertices = geometry::flatten(_quads), .Indices = inds, .Type = primitive_type::Triangles},
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

void text::start(playback_mode mode)
{
    stop();

    _elapsedTime  = milliseconds::zero();
    _playbackMode = mode;
    _isRunning    = true;
    _startedEffects.clear();

    if (!_hasWait) {
        for (auto& [_, effect] : _effects) {
            effect->start(mode);
        }
    }
}

void text::stop()
{
    _isRunning = false;
    for (auto& [_, effect] : _effects) {
        effect->stop();
    }
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
    _startedEffects.clear();
    for (auto& [_, effect] : _effects) {
        effect->clear_groups();
    }

    if (Text->empty()) { return; }

    auto const size {Bounds->Size};
    if (size.Width == 0 || size.Height == 0) { return; }

    auto const currentFont {_font->get_font(Style->FontStyle, Style->FontSize)};
    auto const formatResult {text_formatter::format(_strippedText, *currentFont, Style->Alignment, size, 1.0f, Style->KerningEnabled)};
    _quads.reserve(formatResult.QuadCount);
    _material->first_pass().Texture = currentFont->texture();

    color             currentColor {Style->Color};
    u8                currentAlpha {currentColor.A};
    std::optional<u8> currentEffectIdx {};

    usize       glyphIndex {0};
    usize       cmdIdx {0};
    usize const cmdLen {_commands.size()};

    auto const applyCommands {[&]() {
        while (cmdIdx < cmdLen && _commands[cmdIdx].GlyphIndex == glyphIndex) {
            switch (_commands[cmdIdx].Type) {
            case text_command_type::Alpha:    currentAlpha = std::get<u8>(_commands[cmdIdx].Value); break;
            case text_command_type::AlphaOff: currentAlpha = 255; break;
            case text_command_type::Color:    currentColor = std::get<color>(_commands[cmdIdx].Value); break;
            case text_command_type::ColorOff: currentColor = Style->Color; break;
            case text_command_type::Effect:   {
                currentEffectIdx = std::get<u8>(_commands[cmdIdx].Value);
                if (!_effects.contains(*currentEffectIdx)) { currentEffectIdx = std::nullopt; }
            } break;
            case text_command_type::EffectOff: currentEffectIdx = std::nullopt; break;
            case text_command_type::Wait:      break;
            case text_command_type::None:      break;
            }
            ++cmdIdx;
        }
    }};

    // setup quads
    auto const [x, y] {Bounds->Position};
    for (auto const& token : formatResult.Tokens) {
        for (usize i {0}; i < token.Quads.size(); ++i) {
            applyCommands();
            quad& q {_quads.emplace_back()};

            currentColor.A = currentAlpha;
            geometry::set_color(q, currentColor);

            geometry::set_texcoords(q, token.Quads[i].TextureRegion);

            auto const&  posRect {token.Quads[i].Rect};
            rect_f const quadRect {x + posRect.left(), y + posRect.top(), posRect.width(), posRect.height()};
            geometry::set_position(q, quadRect);

            if (currentEffectIdx) {
                _effects.at(*currentEffectIdx)->add_group(q);
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
    _hasWait = false;
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
                    cmd.Value = static_cast<u8>(255.0f * std::clamp(helper::to_number<f32>(inner.substr(6)).value_or(1.0f), 0.0f, 1.0f));
                } else if (u.starts_with("EFFECT:")) {
                    cmd.Type  = text_command_type::Effect;
                    cmd.Value = helper::to_number<u8>(inner.substr(7)).value_or(0);
                } else if (u.starts_with("WAIT:")) {
                    _hasWait  = true;
                    cmd.Type  = text_command_type::Wait;
                    cmd.Value = milliseconds {helper::to_number<f32>(inner.substr(5)).value_or(0)};
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

    if (_hasWait) {
        _commands.emplace_back(text_command_type::Wait, glyphIndex, milliseconds::zero());
    }
}

}
