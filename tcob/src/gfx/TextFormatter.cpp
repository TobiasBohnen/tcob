// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/TextFormatter.hpp>

#include <locale>

namespace tcob {
std::locale loc = std::locale("en_US.UTF8");

void handle_commands(TextFormatter::ShaperToken& token)
{
    std::string hay { token.Text };
    for (auto& c : hay)
        c = std::toupper(c, std::locale("en_US.utf8"));
    if (hay.rfind("COLOR:", 0) == 0) {
        token.Command = {
            .Type = TextFormatter::CommandType::Color,
            .Value = Colors::FromString(token.Text.substr(6))
        };
    } else if (hay.rfind("ALPHA:", 0) == 0) {
        token.Command = {
            .Type = TextFormatter::CommandType::Alpha,
            .Value = static_cast<u8>(255 * std::clamp(std::strtof(token.Text.substr(6).c_str(), nullptr), 0.f, 1.f))
        };
    }
}

auto TextFormatter::shape(const std::string& text, ResourcePtr<Font>& font) -> std::vector<ShaperToken>
{
    std::vector<ShaperToken> retValue {};

    ShaperToken currentToken {};
    for (auto& ch : text) {
        if (currentToken.Type == ShaperTokenType::Command) {
            if (currentToken.Text.empty() && ch == '{') { // escaped {
                currentToken.Text += ch;
                currentToken.Type = ShaperTokenType::Text;
                continue;
            }

            if (ch == '}') { // finish command
                retValue.push_back(currentToken);
                currentToken = {};
            } else if (!std::isspace(ch, loc)) { // ignore spaces
                currentToken.Text += ch;
            }
        } else if (std::isspace(ch, loc)) { // handle whitespace
            if (ch == '\n') { // handle newline
                if (currentToken.Type != ShaperTokenType::None)
                    retValue.push_back(currentToken);

                retValue.push_back({ .Type = ShaperTokenType::Newline });
                currentToken = {};
            } else if (currentToken.Type != ShaperTokenType::Whitespace) { // start new whitespace token
                if (currentToken.Type != ShaperTokenType::None)
                    retValue.push_back(currentToken);

                currentToken = { .Type = ShaperTokenType::Whitespace };
                currentToken.Text += ch;
            } else { // already whitespace -> add char
                currentToken.Text += ch;
            }
        } else {
            if (std::iscntrl(ch, loc)) { // ignore control chars
                continue;
            }
            if (ch == '{') { // start new command token
                if (currentToken.Type != ShaperTokenType::None)
                    retValue.push_back(currentToken);

                currentToken = { .Type = ShaperTokenType::Command };
            } else { // handle text
                if (currentToken.Type != ShaperTokenType::Text) { // start new text token
                    if (currentToken.Type != ShaperTokenType::None)
                        retValue.push_back(currentToken);

                    currentToken = { .Type = ShaperTokenType::Text };
                    currentToken.Text += ch;
                } else { // add text to text token
                    currentToken.Text += ch;
                }
            }
        }
    }
    if (currentToken.Type != ShaperTokenType::None)
        retValue.push_back(currentToken);

    if (font)
        for (auto& token : retValue) {
            if (token.Type == ShaperTokenType::Text || token.Type == ShaperTokenType::Whitespace) {
                token.Glyphs = font->shape_text(token.Text);
                for (const auto& glyph : token.Glyphs) {
                    token.Width += glyph.Advance;
                }
            } else if (token.Type == ShaperTokenType::Command) {
                handle_commands(token);
            }
        }

    return retValue;
}

auto TextFormatter::format(const std::vector<ShaperToken>& tokens, const FontInfo& font, TextAlignment align, const SizeF& sizeInPixels) -> Result
{
    // fit words in lines
    struct Line {
        std::vector<ShaperToken> Tokens {};
        f32 RemainingWidth { 0 };
        f32 WhiteSpaceCount { 0 };
    };

    std::vector<Line> lines {};
    Line currentLine { .RemainingWidth = sizeInPixels.Width };

    for (auto& token : tokens) {
        if (token.Type == ShaperTokenType::Command) {
            currentLine.Tokens.push_back(token);
            continue;
        }

        f32 testWidth { currentLine.RemainingWidth };
        if (align == TextAlignment::Justified) {
            testWidth -= currentLine.Tokens.size() * 4;
        }
        if (token.Width > testWidth || token.Type == ShaperTokenType::Newline) {
            if (!currentLine.Tokens.empty()) {
                if (currentLine.Tokens.back().Type == ShaperTokenType::Whitespace) { // remove whitespace if last word of line
                    currentLine.WhiteSpaceCount--;
                    if (align != TextAlignment::Justified) {
                        currentLine.RemainingWidth += currentLine.Tokens.back().Width;
                    }
                    currentLine.Tokens.pop_back();
                }

                lines.push_back(currentLine);
            }

            // reset currentLine
            currentLine.Tokens.clear();
            currentLine.WhiteSpaceCount = 0;
            currentLine.RemainingWidth = sizeInPixels.Width;

            // add word to new line
            if (token.Type == ShaperTokenType::Text) {
                currentLine.Tokens.push_back(token);
                currentLine.RemainingWidth -= token.Width;
            }
        } else {
            // add word to line
            if (token.Type == ShaperTokenType::Text || token.Type == ShaperTokenType::Whitespace)
                currentLine.Tokens.push_back(token);

            if (token.Type == ShaperTokenType::Whitespace) {
                currentLine.WhiteSpaceCount++;
                if (align != TextAlignment::Justified) {
                    currentLine.RemainingWidth -= token.Width;
                }
            } else {
                currentLine.RemainingWidth -= token.Width;
            }
        }
    }
    if (!currentLine.Tokens.empty()) {
        lines.push_back(currentLine);
    }

    // format lines
    f32 x { 0 }, y { 0 };
    Result retValue {};
    for (auto& line : lines) {
        switch (align) {
        case TextAlignment::Left:
        case TextAlignment::Justified:
            x = 0;
            break;
        case TextAlignment::Right:
            x = line.RemainingWidth;
            break;
        case TextAlignment::Centered:
            x = line.RemainingWidth / 2;
            break;
        }

        for (auto& token : line.Tokens) {
            FormatterToken form { .Command = token.Command };

            if (token.Type == ShaperTokenType::Whitespace) {
                if (align == TextAlignment::Justified) {
                    x += line.RemainingWidth / line.WhiteSpaceCount;
                } else {
                    x += token.Width;
                }
            } else {
                for (const auto& glyph : token.Glyphs) {
                    form.Quads.push_back(QuadDefinition {
                        .Rect = { { x + glyph.Offset, y + glyph.Bearing.Y + font.Ascender }, glyph.Size },
                        .TexRegion = { glyph.UVRect, 0 } });

                    retValue.GlyphCount++;

                    x += glyph.Advance;
                }
            }

            retValue.Tokens.push_back(form);
        }
        y += font.Height;

        if (y + font.Height > sizeInPixels.Height) {
            break;
        }
    }

    return retValue;
}

auto TextFormatter::format(const std::string& text, ResourcePtr<Font>& font, TextAlignment align, const SizeF& size) -> Result
{
    return format(shape(text, font), font->info(), align, size);
}
}