// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once
#include <tcob/tcob_config.hpp>

#include <variant>

#include <tcob/assets/Resource.hpp>
#include <tcob/gfx/Font.hpp>
#include <tcob/gfx/Transformable.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderTarget.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>
#include <tcob/gfx/gl/GLUniformBuffer.hpp>

namespace tcob {
enum class TextAlignment : u8 {
    Left,
    Right,
    Centered,
    Justified
};

namespace TextFormatter {
    enum class ShaperTokenType {
        None,
        Text,
        Whitespace,
        Newline,
        Command
    };

    enum class CommandType {
        None,
        Color,
        Alpha,
        Effect
    };

    struct CommandDefinition {
        CommandType Type { CommandType::None };
        std::variant<Color, u8> Value;
    };

    struct ShaperToken {
        ShaperTokenType Type { ShaperTokenType::None };
        std::string Text {};
        CommandDefinition Command {};
        std::vector<Glyph> Glyphs {};
        f32 Width { 0 };
    };

    struct QuadDefinition {
        RectF Rect { RectF::Zero };
        TextureRegion TexRegion {};
    };

    struct FormatterToken {
        CommandDefinition Command {};
        std::vector<QuadDefinition> Quads {};
    };

    struct Result {
        std::vector<FormatterToken> Tokens {};
        isize GlyphCount { 0 };
    };

    auto shape(const std::string& text, ResourcePtr<Font>& font) -> std::vector<ShaperToken>;

    auto format(std::span<ShaperToken> tokens, const FontInfo& font, TextAlignment align, const SizeF& size) -> Result;
    auto format(const std::string& text, ResourcePtr<Font>& font, TextAlignment align, const SizeF& size) -> Result;
}
}