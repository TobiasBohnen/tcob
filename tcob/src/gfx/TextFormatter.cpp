// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/TextFormatter.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx::text_formatter {

enum class token_type : u8 {
    None,
    Text,
    Whitespace,
    Newline,
};

struct token {
    token_type         Type {token_type::None};
    string             Text {};
    f32                Width {0};
    std::vector<glyph> Glyphs {};
};

struct line_definition {
    std::vector<token const*> Tokens {};
    f32                       RemainingWidth {0};
    usize                     LineIndex {0};
};

static auto Tokenize(utf8_string_view text) -> std::vector<token>
{
    auto static finish_token {[](std::vector<token>& vec, token& token) {
        if (token.Type != token_type::None) {
            vec.push_back(token);
        }
        token.Type = token_type::None;
        token.Text.clear();
    }};

    std::vector<token> retValue;
    if (text.empty()) { return retValue; }

    token currentToken {};
    for (char const ch : text) {
        switch (ch) {
        case ' ':
        case '\t':
        case '\n': {
            if (ch == '\n') {
                finish_token(retValue, currentToken);
                currentToken.Type = token_type::Newline;
                finish_token(retValue, currentToken);
            } else if (currentToken.Type != token_type::Whitespace) {
                finish_token(retValue, currentToken);
                currentToken.Type = token_type::Whitespace;
                currentToken.Text += ch;
            } else {
                currentToken.Text += ch;
            }
        } break;
        case '\r':
            continue;
        default: {
            if (currentToken.Type != token_type::Text) {
                finish_token(retValue, currentToken);
                currentToken.Type = token_type::Text;
            }
            currentToken.Text += ch;
        } break;
        }
    }

    finish_token(retValue, currentToken);
    return retValue;
}

static auto Shape(utf8_string_view text, font& font, bool kerning, bool measure) -> std::vector<token>
{
    auto retValue {Tokenize(text)};

    for (auto& token : retValue) {
        switch (token.Type) {
        case token_type::Text:
        case token_type::Whitespace: {
            token.Glyphs = measure
                ? font.load_glyphs(token.Text, kerning)
                : font.shape_text(token.Text, kerning);
            for (auto const& glyph : token.Glyphs) { token.Width += glyph.AdvanceX; }
        } break;
        default:
            break;
        }
    }

    return retValue;
}

static auto Wrap(std::vector<token> const& tokens, f32 lineWidth, f32 scale) -> std::vector<line_definition>
{
    std::vector<line_definition> retValue {};
    line_definition              currentLine {};
    currentLine.RemainingWidth = lineWidth < 0 ? std::numeric_limits<f32>::max() : lineWidth;
    currentLine.Tokens.reserve(tokens.size());

    usize lineIndex {0};

    for (auto const& currentToken : tokens) {
        if (std::floor(currentToken.Width * scale) > currentLine.RemainingWidth || currentToken.Type == token_type::Newline) {
            currentLine.LineIndex = lineIndex++;
            retValue.push_back(currentLine);

            // reset currentLine
            currentLine.Tokens.clear();
            currentLine.RemainingWidth = lineWidth < 0 ? std::numeric_limits<f32>::max() : lineWidth;

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
        }
    }
    if (!currentLine.Tokens.empty()) {
        currentLine.LineIndex = lineIndex;
        retValue.push_back(currentLine);
    }

    return retValue;
}

static auto Layout(std::vector<line_definition> const& lines, font& font, alignment align, f32 availableHeight, f32 scale) -> result
{
    availableHeight = availableHeight < 0 ? std::numeric_limits<f32>::max() : availableHeight;

    auto const& fontInfo {font.info()};

    f32 const baseline {fontInfo.Ascender * scale};
    f32       x {0}, y {baseline};
    result    retValue {};
    retValue.Font = &font;

    for (auto const& line : lines) {
        f32 remainingWidth {line.RemainingWidth};
        if (!line.Tokens.empty() && line.Tokens.back()->Type == token_type::Whitespace) {
            remainingWidth += line.Tokens.back()->Width * scale;
        }

        switch (align.Horizontal) {
        case horizontal_alignment::Left:   x = 0; break;
        case horizontal_alignment::Right:  x = remainingWidth; break;
        case horizontal_alignment::Center: x = remainingWidth / 2; break;
        }

        f32 const lineStartX {x};

        for (auto const* shapeToken : line.Tokens) {
            auto& formatToken {retValue.Tokens.emplace_back()};
            formatToken.LineIndex = line.LineIndex;

            for (auto const& glyph : shapeToken->Glyphs) {
                auto&     quadDef {formatToken.Quads.emplace_back()};
                f32 const offsetX {x + (glyph.Offset.X * scale)};
                f32 const offsetY {y + (glyph.Offset.Y * scale)};
                if (shapeToken->Type == token_type::Whitespace) {
                    quadDef.Rect = {{offsetX, offsetY}, size_f {glyph.AdvanceX * scale, 0.0f}};
                } else {
                    quadDef.Rect = {{offsetX, offsetY}, size_f {glyph.Size} * scale};
                }
                quadDef.TextureRegion = *glyph.TextureRegion;

                retValue.QuadCount++;
                x += glyph.AdvanceX * scale;
            }
        }

        y += fontInfo.LineHeight * scale;
        f32 const lineContentWidth {std::ceil(x - lineStartX)};
        retValue.UsedSize.Width = std::max(lineContentWidth, retValue.UsedSize.Width);

        if (y + (fontInfo.LineHeight * scale) > availableHeight + baseline) { break; }
    }

    retValue.UsedSize.Height = y - (fontInfo.LineHeight * scale) - (fontInfo.Descender * scale);

    if (align.Vertical != vertical_alignment::Top) {
        f32 offset {0.0f};
        if (align.Vertical == vertical_alignment::Middle) {
            offset = (availableHeight - retValue.UsedSize.Height) / 2;
        } else if (align.Vertical == vertical_alignment::Bottom) {
            offset = availableHeight - retValue.UsedSize.Height;
        }

        for (auto& token : retValue.Tokens) {
            for (auto& quad : token.Quads) {
                quad.Rect.Position.Y += offset;
            }
        }
    }

    return retValue;
}

auto format(utf8_string_view text, font& font, alignment align, size_f availableSize, f32 scale, bool kerning) -> result
{
    if (scale == 0) { return {}; }

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

auto result::get_quad(isize idx) const -> std::optional<quad_definition>
{
    for (auto const& token : Tokens) {
        for (auto const& quad : token.Quads) {
            if (idx == 0) { return quad; }
            --idx;
        }
    }

    return std::nullopt;
}

}
