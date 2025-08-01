// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Font.hpp"

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "FontEngine.hpp"

#include "tcob/core/Common.hpp"
#include "tcob/core/Logger.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Texture.hpp"

using namespace std::chrono_literals;

namespace tcob::gfx {

////////////////////////////////////////////////////////////

constexpr i32 FONT_TEXTURE_SIZE {2048};
constexpr f32 FONT_TEXTURE_SIZE_F {static_cast<f32>(FONT_TEXTURE_SIZE)};
constexpr u32 FONT_TEXTURE_LAYERS {3};

constexpr i32 GLYPH_PADDING {4};

font::font()
    : _engine {std::make_unique<truetype_font_engine>()}
{
}

font::~font() = default;

auto font::texture() const -> assets::asset_ptr<gfx::texture>
{
    return _texture;
}

auto font::load(path const& file, u32 size) noexcept -> bool
{
    io::ifstream fs {file};
    return load(fs, size);
}

auto font::load(io::istream& stream, u32 size) noexcept -> bool
{
    if (!stream) { return false; }

    _fontData = stream.read_all<byte>();
    return load(_fontData, size);
}

auto font::load(std::span<byte const> fontData, u32 size) noexcept -> bool
{
    if (auto info {_engine->load_data(fontData, size)}) {
        _info = *info;
        _glyphCache.clear();
        _decomposeCache.clear();
        _textureNeedsSetup = true;
        return true;
    }

    return false;
}

void font::setup_texture()
{
    _texture->create({FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE}, FONT_TEXTURE_LAYERS, texture::format::R8);
    _texture->Filtering = texture::filtering::Linear;
    _texture->Wrapping  = texture::wrapping::ClampToBorder;

    _fontTextureCursor = {0, 0};
    _fontTextureLayer  = 0;

    _textureNeedsSetup = false;
}

auto font::render_text(utf8_string_view text, bool kerning) -> std::vector<glyph>
{
    if (_textureNeedsSetup) { setup_texture(); }

    auto const conv {utf8::to_utf32(text)};
    if (!conv) { return {}; }
    usize const len {conv->size()};
    auto const& u32text {*conv};

    std::vector<glyph> retValue;
    retValue.reserve(len);

    for (u32 i {0}; i < len; ++i) {
        u32 const cp0 {u32text[i]};
        if (!cache_render_glyph(cp0)) {
            logger::Error("TrueTypeFont: shaping of text \"{}\" failed.", text);
            return {};
        }

        auto& glyph {retValue.emplace_back(_glyphCache[cp0])};
        if (kerning && i < len - 1) {
            glyph.AdvanceX += _engine->get_kerning(cp0, u32text[i + 1]);
        }
    }

    return retValue;
}

void font::decompose_text(utf8_string_view text, bool kerning, decompose_callbacks& funcs)
{
    auto const conv {utf8::to_utf32(text)};
    if (!conv) { return; }
    usize const len {conv->size()};
    auto const& u32text {*conv};

    funcs.Offset.Y += _info.Ascender;

    std::vector<decompose_commands> cbCommands;
    decompose_callbacks             cb {};
    cb.MoveTo  = [&cbCommands](point_f p) { cbCommands.emplace_back(decompose_move {p}); };
    cb.LineTo  = [&cbCommands](point_f p) { cbCommands.emplace_back(decompose_line {p}); };
    cb.ConicTo = [&cbCommands](point_f p0, point_f p1) { cbCommands.emplace_back(decompose_conic {p0, p1}); };
    cb.CubicTo = [&cbCommands](point_f p0, point_f p1, point_f p2) { cbCommands.emplace_back(decompose_cubic {p0, p1, p2}); };

    for (usize i {0}; i < len; ++i) {
        auto const cp0 {u32text[i]};

        if (!_decomposeCache.contains(cp0)) {
            cbCommands.clear();
            auto const gl {_engine->decompose_glyph(cp0, cb)};
            _glyphCache[cp0]     = gl;
            _decomposeCache[cp0] = {.CodePoint = cp0, .Commands = cbCommands};
        }

        auto const& result {_decomposeCache[cp0]};
        for (auto const& command : result.Commands) {
            std::visit(
                overloaded {[&](decompose_move const& cmd) { funcs.MoveTo(cmd.Point); },
                            [&](decompose_line const& cmd) { funcs.LineTo(cmd.Point); },
                            [&](decompose_conic const& cmd) { funcs.ConicTo(cmd.Point0, cmd.Point1); },
                            [&](decompose_cubic const& cmd) { funcs.CubicTo(cmd.Point0, cmd.Point1, cmd.Point2); }},
                command);
        }

        funcs.Offset.X += _glyphCache[result.CodePoint].AdvanceX;
        if (kerning && i < len - 1) {
            funcs.Offset.X += _engine->get_kerning(result.CodePoint, u32text[i + 1]);
        }
    }
}

auto font::get_glyphs(utf8_string_view text, bool kerning) -> std::vector<glyph>
{
    auto const conv {utf8::to_utf32(text)};
    if (!conv) { return {}; }
    usize const len {conv->size()};
    auto const& u32text {*conv};

    std::vector<glyph> retValue;
    retValue.reserve(len);

    for (u32 i {0}; i < len; ++i) {
        u32 const cp0 {u32text[i]};
        if (_glyphCache.contains(cp0)) {
            retValue.push_back(_glyphCache[cp0]);
        } else {
            retValue.push_back(_engine->load_glyph(cp0));
        }

        if (kerning && i < len - 1) {
            retValue.back().AdvanceX += _engine->get_kerning(cp0, u32text[i + 1]);
        }
    }

    return retValue;
}

auto font::Init() -> bool
{
    return truetype_font_engine::Init();
}

void font::Done()
{
    truetype_font_engine::Done();
}

auto font::info() const -> font::information const&
{
    return _info;
}

auto font::cache_render_glyph(u32 cp) -> bool
{
    if (!_glyphCache.contains(cp) || !_glyphCache[cp].TextureRegion) {
        auto       gb {_engine->render_glyph(cp)};
        auto const bitmapSize {gb.second.BitmapSize};
        if (bitmapSize.Width < 0 || bitmapSize.Height < 0) { return false; }

        // check font texture space
        if (_fontTextureCursor.X + bitmapSize.Width >= FONT_TEXTURE_SIZE) { // new line
            _fontTextureCursor.X = 0;
            _fontTextureCursor.Y += static_cast<i32>(info().LineHeight) + GLYPH_PADDING;
        }

        if (_fontTextureCursor.Y + bitmapSize.Height >= FONT_TEXTURE_SIZE) { // new level
            _fontTextureLayer++;
            if (_fontTextureLayer >= FONT_TEXTURE_LAYERS) {
                logger::Error("TrueTypeFont: font texture layer {} exceeds maximum.", _fontTextureLayer);
            }
            _fontTextureCursor = point_i::Zero;
        }

        // write to texture
        _texture->update_data(_fontTextureCursor, bitmapSize, gb.second.Bitmap.data(), _fontTextureLayer, bitmapSize.Width, 1);

        // create glyph
        gb.first.TextureRegion = {.UVRect = {static_cast<f32>(_fontTextureCursor.X) / FONT_TEXTURE_SIZE_F,
                                             static_cast<f32>(_fontTextureCursor.Y) / FONT_TEXTURE_SIZE_F,
                                             static_cast<f32>(bitmapSize.Width) / FONT_TEXTURE_SIZE_F,
                                             static_cast<f32>(bitmapSize.Height) / FONT_TEXTURE_SIZE_F},
                                  .Level  = _fontTextureLayer};
        _glyphCache[cp]        = gb.first;

        // advance cursor
        _fontTextureCursor.X += bitmapSize.Width + GLYPH_PADDING;
    }
    return true;
}

}
