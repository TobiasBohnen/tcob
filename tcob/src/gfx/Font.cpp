// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Font.hpp"

#include <optional>
#include <utility>

#include "tcob/core/Logger.hpp"
#include "tcob/core/Semaphore.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

#include "FontEngines.hpp"

using namespace std::chrono_literals;

namespace tcob::gfx {

// based on: https://github.com/brofield/simpleini/blob/master/ConvertUTF.c
auto static convert_UTF8_to_UTF32(string_view text) -> std::u32string
{
    static constexpr std::array<ubyte, 256> trailingBytesForUTF8 {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5};

    static constexpr std::array<u32, 6> offsetsFromUTF8 {0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL};

    usize const    texSize {text.size()};
    std::u32string retValue {};
    retValue.reserve(texSize);

    for (usize i {0}; i < texSize;) {
        ubyte const source {static_cast<ubyte>(text[i])};
        u32         ch {0};

        ubyte const extraBytesToRead {trailingBytesForUTF8[source]};
        if (i + extraBytesToRead >= texSize) {
            return {};
        }

        for (ubyte j {extraBytesToRead}; j > 0; --j) {
            ch += static_cast<ubyte>(text[i++]);
            ch <<= 6;
        }
        ch += static_cast<ubyte>(text[i++]);
        ch -= offsetsFromUTF8[extraBytesToRead];

        if (ch <= 0x0010FFFF) {
            retValue.append(1, static_cast<char32_t>(ch));
        } else {
            return {};
        }
    }

    return retValue;
}

font::font() = default;

auto font::get_texture() const -> assets::asset_ptr<texture>
{
    return _texture;
}

////////////////////////////////////////////////////////////
raster_font::raster_font() = default;

auto raster_font::get_info() const -> font::info const&
{
    return _info;
}

auto raster_font::load(path const& file, string const& textureFolder) noexcept -> load_status
{
    if (!io::is_file(file)) { return load_status::FileNotFound; }

    if (auto dec {locate_service<loader::factory>().create(io::get_extension(file))}) {
        if (auto info {dec->load(*this, file, textureFolder)}) {
            _info = *info;
            return load_status::Ok;
        }
    }

    return load_status::Error;
}

auto raster_font::load_async(path const& filename, string const& textureFolder) noexcept -> std::future<load_status>
{
    return std::async(std::launch::async, [&, filename, textureFolder]() {
        auto& sema {locate_service<semaphore>()};
        sema.acquire();
        auto const retValue {load(filename, textureFolder)};
        sema.release();
        return retValue;
    });
}

void raster_font::setup_texture()
{
    if (_fontImages.empty()) {
        return;
    }

    u32 const  pages {static_cast<u32>(_fontImages.size())};
    auto const texSize {_fontImages[0].get_info().Size};
    auto       texture {get_texture()};
    texture->create(texSize, pages, texture::format::RGBA8);
    texture->Filtering = texture::filtering::NearestNeighbor;

    for (u32 i {0}; i < pages; ++i) {
        texture->update_data(_fontImages[i].get_data(), i);
    }

    _fontImages.clear();

    _textureNeedsUpdate = false;
}

auto raster_font::shape_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<glyph>
{
    if (_textureNeedsUpdate && !readOnlyCache) {
        setup_texture();
    }

    auto const         utf32text {convert_UTF8_to_UTF32(text)};
    usize const        len {utf32text.size()};
    std::vector<glyph> retValue {};
    retValue.reserve(len);

    for (u32 i {0}; i < len; ++i) {
        u32 const first {utf32text[i]};

        auto it {_glyphs.find(first)};
        if (it == _glyphs.end()) {
            logger::Warning("RasterFont: glyph not found: {}", first);
        }

        auto& glyph {retValue.emplace_back(it->second)};
        if (kerning && i < len - 1) {
            u32 const second {utf32text[i + 1]};
            if (_kerning.contains(first) && _kerning[first].contains(second)) {
                glyph.AdvanceX += _kerning[first][second];
            }
        }
    }

    return retValue;
}

void raster_font::add_image(image const& img)
{
    _fontImages.push_back(img);
    _textureNeedsUpdate = true;
}

void raster_font::add_glyph(u32 idx, glyph const& gl)
{
    _glyphs[idx] = gl;
}

void raster_font::add_kerning_pair(u32 first, u32 second, i16 amount)
{
    _kerning[first][second] = amount;
}

////////////////////////////////////////////////////////////

constexpr i32 FONT_TEXTURE_SIZE {512};
constexpr f32 FONT_TEXTURE_SIZE_F {static_cast<f32>(FONT_TEXTURE_SIZE)};
constexpr u32 FONT_TEXTURE_LAYERS {3};

constexpr i32      GLYPH_PADDING {4};
static char const* FONT_WARMUP {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,;:'!\"%&()=?<>"};

truetype_font::truetype_font()
    : _engine {std::make_unique<detail::ft_ttf_font_engine>()}
{
}

auto truetype_font::load(path const& file, u32 size) noexcept -> load_status
{
    if (auto fs {io::ifstream::Open(file)}) { return load(*fs, size); }

    return load_status::FileNotFound;
}

auto truetype_font::load(istream& stream, u32 size) noexcept -> load_status
{
    _fontData = stream.read_all<ubyte>();
    return load(_fontData, size);
}

auto truetype_font::load(std::span<ubyte const> fontData, u32 size) noexcept -> load_status
{
    if (auto info {_engine->load_data(fontData, size)}) {
        _info = *info;
        _glyphs.clear();
        _textureNeedsSetup = true;
        return load_status::Ok;
    }

    return load_status::Error;
}

void truetype_font::setup_texture()
{
    auto texture {get_texture()};
    texture->create({FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE}, FONT_TEXTURE_LAYERS, texture::format::R8);
    texture->Filtering = texture::filtering::Linear;
    texture->Wrapping  = texture::wrapping::ClampToBorder;

    _fontTextureCursor = {0, 0};
    _fontTextureLayer  = 0;

    _textureNeedsSetup = false;
}

auto truetype_font::shape_text(utf8_string_view text, bool kerning, bool readOnlyCache) -> std::vector<glyph>
{
    if (_textureNeedsSetup && !readOnlyCache) {
        setup_texture();
        shape_text(FONT_WARMUP, false, true);
    }

    auto const         utf32text {convert_UTF8_to_UTF32(text)};
    usize const        len {utf32text.size()};
    std::vector<glyph> retValue;
    retValue.reserve(len);

    for (u32 i {0}; i < len; ++i) {
        u32 const cp0 {utf32text[i]};
        if (!readOnlyCache) {
            if (!cache_glyph(cp0)) {
                logger::Error("TrueTypeFont: shaping of text \"{}\" failed.", text);
                return {};
            }

            auto& glyph {retValue.emplace_back(_glyphs[cp0])};
            if (kerning && i < len - 1) {
                glyph.AdvanceX += _engine->get_kerning(cp0, utf32text[i + 1]);
            }
        } else {
            if (_glyphs.contains(cp0)) {
                retValue.push_back(_glyphs[cp0]);
            } else {
                retValue.push_back(_engine->get_glyph(cp0).Glyph);
            }
        }
    }

    return retValue;
}

auto truetype_font::get_info() const -> font::info const&
{
    return _info;
}

auto truetype_font::cache_glyph(u32 cp) -> bool
{
    if (!_glyphs.contains(cp)) {
        auto        gb {_engine->get_glyph(cp)};
        auto const& glyphSize {gb.Glyph.Size};
        if (glyphSize.Width < 0 || glyphSize.Height < 0) {
            return false;
        }

        // check font texture space
        if (_fontTextureCursor.X + glyphSize.Width >= FONT_TEXTURE_SIZE) { // new line
            _fontTextureCursor.X = 0;
            _fontTextureCursor.Y += static_cast<i32>(get_info().LineHeight) + GLYPH_PADDING;
        }

        if (_fontTextureCursor.Y + glyphSize.Height >= FONT_TEXTURE_SIZE) { // new level
            _fontTextureLayer++;
            if (_fontTextureLayer >= FONT_TEXTURE_LAYERS) {
                logger::Error("TrueTypeFont: font texture layer {} exceeds maximum.", _fontTextureLayer);
            }
            _fontTextureCursor = point_i::Zero;
        }

        // write to texture
        get_texture()->update_data(_fontTextureCursor, glyphSize, gb.Bitmap.data(), _fontTextureLayer, glyphSize.Width, 1);

        // create glyph
        gb.Glyph.TexRegion = {.UVRect = {_fontTextureCursor.X / FONT_TEXTURE_SIZE_F, _fontTextureCursor.Y / FONT_TEXTURE_SIZE_F,
                                         glyphSize.Width / FONT_TEXTURE_SIZE_F, glyphSize.Height / FONT_TEXTURE_SIZE_F},
                              .Level  = _fontTextureLayer};

        _glyphs[cp] = gb.Glyph;

        // advance cursor
        _fontTextureCursor.X += glyphSize.Width + GLYPH_PADDING;
    }
    return true;
}

////////////////////////////////////////////////////////////

font_family::font_family(string name)
    : _name {std::move(name)}
{
}

void font_family::set_source(font::style style, path const& file)
{
    _fontSources[style] = file;
}

auto font_family::get_fallback_style(font::style style) const -> std::optional<font::style>
{
    font::style retValue {style};

    if (has_style(retValue)) { return retValue; }

    auto const ascendWhile {[&]() {
        while (static_cast<i32>(retValue.Weight) <= static_cast<i32>(font::weight::Heavy)) {
            retValue.Weight = static_cast<font::weight>(static_cast<i32>(retValue.Weight) + 100);
            if (has_style(retValue)) { return true; }
        }
        return false;
    }};

    auto const descendWhile {[&]() {
        while (static_cast<i32>(retValue.Weight) > 0) {
            retValue.Weight = static_cast<font::weight>(static_cast<i32>(retValue.Weight) - 100);
            if (has_style(retValue)) { return true; }
        }
        return false;
    }};

    // rule from https://developer.mozilla.org/en-US/docs/Web/CSS/font-weight#fallback_weights
    // If the target weight given is between 400 and 500 inclusive:
    if (style.Weight == font::weight::Normal || style.Weight == font::weight::Medium) {
        // Look for available weights between the target and 500, in ascending order.
        retValue.Weight = font::weight::Medium;
        if (has_style(retValue)) { return retValue; }
        retValue.Weight = font::weight::Normal;
        if (has_style(retValue)) { return retValue; }

        // If no match is found, look for available weights less than the target, in descending order.
        if (descendWhile()) { return retValue; }
        // If no match is found, look for available weights greater than 500, in ascending order.
        retValue.Weight = font::weight::Medium;
        if (ascendWhile()) { return retValue; }

        return std::nullopt;
    }

    // If a weight less than 400 is given,
    if (static_cast<i32>(retValue.Weight) < 400) {
        // look for available weights less than the target, in descending order.
        if (descendWhile()) { return retValue; }
        // If no match is found, look for available weights greater than the target, in ascending order.
        retValue.Weight = style.Weight;
        if (ascendWhile()) { return retValue; }
    }

    // If a weight greater than 500 is given,
    if (static_cast<i32>(retValue.Weight) > 500) {
        // look for available weights greater than the target, in ascending order.
        if (ascendWhile()) { return retValue; }
        // If no match is found, look for available weights less than the target, in descending order.
        retValue.Weight = style.Weight;
        if (descendWhile()) { return retValue; }
    }

    return std::nullopt;
}

auto font_family::has_style(font::style style) const -> bool
{
    return _fontSources.contains(style);
}

auto font_family::get_name() const -> string const&
{
    return _name;
}

auto font_family::get_font(font::style style, u32 size) -> assets::asset_ptr<font>
{
    auto fallbackStyle {get_fallback_style(style)};
    if (!fallbackStyle) {
        logger::Error("FontFamily {}: no sources for font style: style {}, size {}.", _name, style, size);
        return {};
    }

    auto const fontStyle {*fallbackStyle};

    // check if asset already exists
    if (auto const styleIt {_fontAssets.find(fontStyle)}; styleIt != _fontAssets.end()) {
        if (auto const sizeIt {styleIt->second.find(size)}; sizeIt != styleIt->second.end()) {
            return sizeIt->second;
        }
    }

    logger::Info("FontFamily {}: created new font: style {}, size {}.", _name, fontStyle, size);

    // check if font data has already been loaded
    if (!_fontData.contains(fontStyle)) {
        auto fs {io::ifstream::Open(_fontSources[fontStyle])};
        assert(_fontData.capacity() > _fontData.size()); // _fontData shall not reallocate
        _fontData[fontStyle] = fs->read_all<ubyte>();
    }

    // load font
    auto const& asset {_fontAssets[fontStyle][size]};
    if (asset.get_obj()->load(_fontData[fontStyle], size) == load_status::Ok) {
        return asset;
    }

    return {};
}

void font_family::clear_assets()
{
    _fontAssets.clear();
}

auto font_family::get_font_count() const -> isize
{
    isize retValue {0};
    for (auto const& a : _fontAssets) {
        retValue += std::ssize(a.second);
    }
    return retValue;
}

void font_family::FindSources(font_family& fam, path const& source)
{
    fam._fontSources.clear();
    fam._fontData.clear();
    fam._fontAssets.clear();

    auto files {io::enumerate(io::get_parent_folder(source), source + "*.ttf", false)};
    for (auto const& file : files) {
        font::style       style;
        string_view const name {string_view {file}.substr(source.size())};

        style.IsItalic = name.find("Italic") != string::npos;

        if (name.find("Thin") != string::npos || name.find("Hairline") != string::npos) {   // Thin
            style.Weight = font::weight::Thin;
        } else if (name.find("Light") != string::npos) {
            if (name.find("Extra") != string::npos || name.find("Ultra") != string::npos) { // Light or ExtraLight
                style.Weight = font::weight::ExtraLight;
            } else {
                style.Weight = font::weight::Light;
            }
        } else if (name.find("Medium") != string::npos) { // Medium
            style.Weight = font::weight::Medium;
        } else if (name.find("Bold") != string::npos) {   // Bold or SemiBold or ExtraBold
            if (name.find("Semi") != string::npos || name.find("Demi") != string::npos) {
                style.Weight = font::weight::SemiBold;
            } else if (name.find("Extra") != string::npos || name.find("Ultra") != string::npos) {
                style.Weight = font::weight::ExtraBold;
            } else {
                style.Weight = font::weight::Bold;
            }
        } else if (name.find("Heavy") != string::npos || name.find("Black") != string::npos) { // Heavy
            style.Weight = font::weight::Heavy;
        } else {                                                                               // Normal
            style.Weight = font::weight::Normal;
        }

        fam.set_source(style, file);
    }

    fam._fontData.reserve(fam._fontSources.size());
}
}
