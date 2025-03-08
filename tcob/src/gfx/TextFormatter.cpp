// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/TextFormatter.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <locale>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx::text_formatter {

enum class token_type : u8 {
    None,
    Text,
    Whitespace,
    Newline,
    Command
};

struct token {
    token_type         Type {token_type::None}; // shape
    string             Text {};                 // shape
    command_definition Command {};              // shape
    f32                Width {0};               // shape
    std::vector<glyph> Glyphs {};               // shape
};

struct line_definition {
    std::vector<token const*> Tokens {};
    f32                       RemainingWidth {0};
    f32                       WhiteSpaceCount {0};
};

void static HandleCommands(token& token)
{
    static std::locale loc {"en_US.UTF8"};

    auto hay {token.Text};
    for (auto& c : hay) {
        c = std::toupper(c, loc);
    }

    if (hay.rfind("COLOR:", 0) == 0) {
        token.Command = {
            .Type  = command_type::Color,
            .Value = color::FromString(token.Text.substr(6))};
    } else if (hay.rfind("ALPHA:", 0) == 0) {
        token.Command = {
            .Type  = command_type::Alpha,
            .Value = static_cast<u8>(255 * std::clamp(std::strtof(token.Text.substr(6).c_str(), nullptr), 0.0f, 1.0f))};
    } else if (hay.rfind("EFFECT:", 0) == 0) {
        token.Command = {
            .Type  = command_type::Effect,
            .Value = static_cast<u8>(std::strtoul(token.Text.substr(7).c_str(), nullptr, 10))};
    }
}

void static FinishToken(std::vector<token>& vec, token& token)
{
    if (token.Type == token_type::Command) {
        HandleCommands(token);
    }
    if (token.Type != token_type::None) {
        vec.push_back(token);
    }

    token.Type = token_type::None;
    token.Text.clear();
    token.Command = {};
}

auto static Tokenize(utf8_string_view text) -> std::vector<token>
{
    std::vector<token> retValue {};

    if (text.empty()) { return retValue; }

    token currentToken {};
    for (char const ch : text) {
        switch (ch) {
        case ' ':
        case '\t':
        case '\n': {
            if (currentToken.Type == token_type::Command) { // ignore spaces in command
                continue;
            }

            if (ch == '\n') { // handle newline
                FinishToken(retValue, currentToken);
                currentToken.Type = token_type::Newline;
                FinishToken(retValue, currentToken);
            } else if (currentToken.Type != token_type::Whitespace) { // start new whitespace token
                FinishToken(retValue, currentToken);
                currentToken.Type = token_type::Whitespace;
                currentToken.Text += ch;
            } else { // already whitespace -> add char
                currentToken.Text += ch;
            }
        } break;
        case '\r':
            continue;
        case '{': {
            if (currentToken.Type == token_type::Command) {
                if (currentToken.Text.empty()) { // escaped {
                    currentToken.Text += ch;
                    currentToken.Type = token_type::Text;
                }
            } else {
                FinishToken(retValue, currentToken);
                currentToken.Type = token_type::Command;
            }
        } break;
        case '}': {
            if (currentToken.Type == token_type::Command) {  // close command token
                FinishToken(retValue, currentToken);
            } else {
                if (currentToken.Type != token_type::Text) { // start new text token
                    FinishToken(retValue, currentToken);
                    currentToken.Type = token_type::Text;
                }
                currentToken.Text += ch;
            }
        } break;
        default: {
            // handle text
            if (currentToken.Type != token_type::Text && currentToken.Type != token_type::Command) { // start new text token
                FinishToken(retValue, currentToken);
                currentToken.Type = token_type::Text;
            }

            currentToken.Text += ch;
        } break;
        }
    }

    if (currentToken.Type == token_type::Command) { // treat unclosed command as text
        currentToken.Type = token_type::Text;
    }

    FinishToken(retValue, currentToken);

    return retValue;
}

auto static Shape(utf8_string_view text, font& font, bool kerning, bool measure) -> std::vector<token>
{
    auto retValue {Tokenize(text)};

    for (auto& token : retValue) {
        switch (token.Type) {
        case token_type::Text:
        case token_type::Whitespace:
            if (measure) {
                token.Glyphs = font.get_glyphs(token.Text, kerning);
            } else {
                token.Glyphs = font.render_text(token.Text, kerning);
            }

            for (auto const& glyph : token.Glyphs) { token.Width += glyph.AdvanceX; }
            break;
        default:
            break;
        }
    }

    return retValue;
}

auto static Wrap(std::vector<token> const& tokens, f32 lineWidth, f32 scale) -> std::vector<line_definition>
{
    std::vector<line_definition> retValue {};
    line_definition              currentLine {};
    currentLine.RemainingWidth = lineWidth < 0 ? std::numeric_limits<f32>::max() : lineWidth;
    currentLine.Tokens.reserve(tokens.size());

    for (auto const& currentToken : tokens) {
        if (currentToken.Type == token_type::Command) {
            currentLine.Tokens.push_back(&currentToken);
            continue;
        }

        f32 const testWidth {currentLine.RemainingWidth};
        if (std::floor(currentToken.Width * scale) > testWidth || currentToken.Type == token_type::Newline) {
            if (!currentLine.Tokens.empty()) {
                if (currentLine.Tokens.back()->Type == token_type::Whitespace) { // remove whitespace if last word of line
                    currentLine.WhiteSpaceCount--;
                    currentLine.RemainingWidth += currentLine.Tokens.back()->Width * scale;
                    currentLine.Tokens.pop_back();
                }
            }
            retValue.push_back(currentLine);

            // reset currentLine
            currentLine.Tokens.clear();
            currentLine.WhiteSpaceCount = 0;
            currentLine.RemainingWidth  = lineWidth < 0 ? std::numeric_limits<f32>::max() : lineWidth;

            // add word to new line
            if (currentToken.Type == token_type::Text) {
                currentLine.Tokens.push_back(&currentToken);
                currentLine.RemainingWidth -= currentToken.Width * scale;
            }
        } else {
            // add word to line
            if (currentToken.Type == token_type::Text || currentToken.Type == token_type::Whitespace) {
                currentLine.Tokens.push_back(&currentToken);
            }

            currentLine.RemainingWidth -= currentToken.Width * scale;
            if (currentToken.Type == token_type::Whitespace) {
                currentLine.WhiteSpaceCount++;
            }
        }
    }
    if (!currentLine.Tokens.empty()) {
        retValue.push_back(currentLine);
    }

    return retValue;
}

auto static Layout(std::vector<line_definition> const& lines, font& font, alignments align, f32 availableHeight, f32 scale) -> result
{
    availableHeight = availableHeight < 0 ? std::numeric_limits<f32>::max() : availableHeight;

    auto const& fontInfo {font.info()};

    f32    x {0}, y {0};
    result retValue {};
    retValue.Font = &font;

    for (auto const& line : lines) {
        switch (align.Horizontal) {
        case horizontal_alignment::Left: x = 0; break;
        case horizontal_alignment::Right: x = line.RemainingWidth; break;
        case horizontal_alignment::Centered: x = line.RemainingWidth / 2; break;
        }

        for (auto const* shapeToken : line.Tokens) {
            auto& formatToken {retValue.Tokens.emplace_back()};
            formatToken.Command = shapeToken->Command;

            for (auto const& glyph : shapeToken->Glyphs) {
                auto&     quadDef {formatToken.Quads.emplace_back()};
                f32 const offsetX {x + (glyph.Offset.X * scale)};
                f32 const offsetY {y + (glyph.Offset.Y * scale)};
                if (shapeToken->Type == token_type::Whitespace) {
                    quadDef.Rect = {{offsetX, offsetY}, size_f {glyph.AdvanceX * scale, 0.f}};
                } else {
                    quadDef.Rect = {{offsetX, offsetY}, size_f {glyph.Size} * scale};
                }
                quadDef.TextureRegion = *glyph.TextureRegion;

                retValue.QuadCount++;
                x += glyph.AdvanceX * scale;
            }
        }

        y += fontInfo.LineHeight * scale;
        retValue.UsedSize.Width = std::max(x, retValue.UsedSize.Width);

        if (y + fontInfo.LineHeight * scale > availableHeight) { break; }
    }

    retValue.UsedSize.Height = y;

    if (align.Vertical != vertical_alignment::Top) {
        f32 offset {0.0f};
        if (align.Vertical == vertical_alignment::Middle) {
            offset = (availableHeight - y) / 2;
        } else if (align.Vertical == vertical_alignment::Bottom) {
            offset = (availableHeight - y);
        }

        for (auto& token : retValue.Tokens) {
            for (auto& quad : token.Quads) {
                quad.Rect.Position.Y += offset;
            }
        }
    }

    return retValue;
}

auto format(utf8_string_view text, font& font, alignments align, size_f availableSize, f32 scale, bool kerning) -> result
{
    auto shaperTokens {Shape(text, font, kerning, false)};
    auto lines {Wrap(shaperTokens, availableSize.Width, scale)};
    return Layout(lines, font, align, availableSize.Height, scale);
}

auto measure(utf8_string_view text, font& font, f32 availableHeight, bool kerning) -> size_f
{
    auto shaperTokens {Shape(text, font, kerning, true)};
    auto lines {Wrap(shaperTokens, -1, 1.0f)};
    return Layout(lines, font, {}, availableHeight, 1.0f).UsedSize;
}

////////////////////////////////////////////////////////////

auto result::get_quad(isize idx) const -> quad_definition
{
    for (auto const& token : Tokens) {
        for (auto const& quad : token.Quads) {
            if (idx == 0) { return quad; }
            --idx;
        }
    }

    return {};
}

////////////////////////////////////////////////////////////

}
