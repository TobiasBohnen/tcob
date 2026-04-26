// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/FontFamily.hpp"

#include <cstddef>
#include <format>
#include <optional>
#include <span>
#include <utility>

#include "tcob/core/Logger.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/gfx/Font.hpp"

namespace tcob::gfx {

font_family::font_family(string name)
    : _name {std::move(name)}
{
}

void font_family::add_style(font::style style, path const& file)
{
    _styles[style].Source = file;
}

auto font_family::get_style(font::style want) const -> std::optional<font::style>
{
    if (has_style(want)) { return want; }

    font::style retValue {want};

    auto const ascend {[&] {
        while (static_cast<i32>(retValue.Weight) <= static_cast<i32>(font::weight::Heavy)) {
            retValue.Weight = static_cast<font::weight>(static_cast<i32>(retValue.Weight) + 100);
            if (has_style(retValue)) { return true; }
        }
        return false;
    }};

    auto const descend {[&] {
        while (static_cast<i32>(retValue.Weight) > 0) {
            retValue.Weight = static_cast<font::weight>(static_cast<i32>(retValue.Weight) - 100);
            if (has_style(retValue)) { return true; }
        }
        return false;
    }};

    // rule from https://developer.mozilla.org/en-US/docs/Web/CSS/font-weight#fallback_weights
    // If the target weight given is between 400 and 500 inclusive:
    if (want.Weight == font::weight::Normal || want.Weight == font::weight::Medium) {
        // Look for available weights between the target and 500, in ascending order.
        retValue.Weight = font::weight::Medium;
        if (has_style(retValue)) { return retValue; }

        // If no match is found, look for available weights less than the target, in descending order.
        retValue.Weight = font::weight::Normal;
        if (has_style(retValue)) { return retValue; }
        if (descend()) { return retValue; }
        // If no match is found, look for available weights greater than 500, in ascending order.
        retValue.Weight = font::weight::Medium;
        if (ascend()) { return retValue; }

        return std::nullopt;
    }

    // If a weight less than 400 is given,
    if (static_cast<i32>(retValue.Weight) < 400) {
        // look for available weights less than the target, in descending order.
        if (descend()) { return retValue; }
        // If no match is found, look for available weights greater than the target, in ascending order.
        retValue.Weight = want.Weight;
        if (ascend()) { return retValue; }
    }

    // If a weight greater than 500 is given,
    if (static_cast<i32>(retValue.Weight) > 500) {
        // look for available weights greater than the target, in ascending order.
        if (ascend()) { return retValue; }
        // If no match is found, look for available weights less than the target, in descending order.
        retValue.Weight = want.Weight;
        if (descend()) { return retValue; }
    }

    return std::nullopt;
}

auto font_family::has_style(font::style style) const -> bool
{
    return _styles.contains(style);
}

auto font_family::name() const -> string const&
{
    return _name;
}

auto font_family::get_font(font::style style, u32 size) -> asset_ptr<font>
{
    auto fallbackStyle {get_style(style)};
    if (!fallbackStyle) {
        logger::Error("font_family {}: no sources for font style: style {}, size {}.", _name, style, size);
        return {};
    }

    auto const fontStyle {*fallbackStyle};

    auto const styleIt {_styles.find(fontStyle)};
    if (styleIt == _styles.end()) { return {}; }

    auto& entry {styleIt->second};
    // check if font already exists
    if (auto const sizeIt {entry.Fonts.find(size)}; sizeIt != entry.Fonts.end()) {
        return sizeIt->second;
    }

    // load font data
    if (entry.Data.empty() && io::is_file(entry.Source)) {
        io::ifstream fs {entry.Source};
        entry.Data = fs.read_all<std::byte>();
    }

    logger::Info("font_family {}: created new font: style {}, size {}.", _name, fontStyle, size);

    // load font
    auto const            fontName {std::format("{}-{}-{}", _name, fontStyle, size)};
    asset_owner_ptr<font> newFont {fontName, fontName};
    entry.Fonts[size] = newFont;
    if (newFont->load(entry.Data, size)) {
        newFont->TextureResized.connect([this](font const& f) { TextureResized(f); });
        return newFont;
    }

    return {};
}

void font_family::FindSources(font_family& fam, path const& source)
{
    fam._styles.clear();

    auto const files {io::enumerate(io::get_parent_folder(source), {.String = source + "*.ttf", .MatchWholePath = true}, false)};
    for (auto const& file : files) {
        font::style       style;
        string_view const name {string_view {file}.substr(source.size())};

        style.IsItalic = name.contains("Italic");

        if (name.contains("Thin") || name.contains("Hairline")) {   // Thin
            style.Weight = font::weight::Thin;
        } else if (name.contains("Light")) {
            if (name.contains("Extra") || name.contains("Ultra")) { // Light or ExtraLight
                style.Weight = font::weight::ExtraLight;
            } else {
                style.Weight = font::weight::Light;
            }
        } else if (name.contains("Medium")) { // Medium
            style.Weight = font::weight::Medium;
        } else if (name.contains("Bold")) {   // Bold or SemiBold or ExtraBold
            if (name.contains("Semi") || name.contains("Demi")) {
                style.Weight = font::weight::SemiBold;
            } else if (name.contains("Extra") || name.contains("Ultra")) {
                style.Weight = font::weight::ExtraBold;
            } else {
                style.Weight = font::weight::Bold;
            }
        } else if (name.contains("Heavy") || name.contains("Black")) { // Heavy
            style.Weight = font::weight::Heavy;
        } else {                                                       // Normal
            style.Weight = font::weight::Normal;
        }

        if (!fam.has_style(style)) {
            fam.add_style(style, file);
        }
    }
}

void font_family::SingleFont(font_family& fam, std::span<std::byte const> font)
{
    fam._styles.clear();

    fam._styles[{}].Data.assign(font.begin(), font.end());
    fam._styles[{}].Source = "";
}

}
