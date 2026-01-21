// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/FontFamily.hpp"

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

void font_family::set_source(font::style style, path const& file)
{
    _styles[style].Source = file;
}

auto font_family::get_fallback_style(font::style style) const -> std::optional<font::style>
{
    font::style retValue {style};

    if (has_style(retValue)) { return retValue; }

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
    if (style.Weight == font::weight::Normal || style.Weight == font::weight::Medium) {
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
        retValue.Weight = style.Weight;
        if (ascend()) { return retValue; }
    }

    // If a weight greater than 500 is given,
    if (static_cast<i32>(retValue.Weight) > 500) {
        // look for available weights greater than the target, in ascending order.
        if (ascend()) { return retValue; }
        // If no match is found, look for available weights less than the target, in descending order.
        retValue.Weight = style.Weight;
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
    auto fallbackStyle {get_fallback_style(style)};
    if (!fallbackStyle) {
#if !defined(__EMSCRIPTEN__) // TODO: fixed in llvm 19
        logger::Error("FontFamily {}: no sources for font style: style {}, size {}.", _name, style, size);
#endif
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
        entry.Data = fs.read_all<byte>();
    }

#if !defined(__EMSCRIPTEN__) // TODO: fixed in llvm 19
    logger::Info("FontFamily {}: created new font: style {}, size {}.", _name, fontStyle, size);
#endif

    // load font
    auto const& asset {entry.Fonts[size]};
    if (asset->load(entry.Data, size)) {
        return asset;
    }

    return {};
}

void font_family::FindSources(font_family& fam, path const& source)
{
    fam._styles.clear();

    auto const files {io::enumerate(io::get_parent_folder(source), {source + "*.ttf", true}, false)};
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

        if (!fam.has_style(style)) {
            fam.set_source(style, file);
        }
    }
}

void font_family::SingleFont(font_family& fam, std::span<byte const> font)
{
    fam._styles.clear();

    fam._styles[{}].Data.assign(font.begin(), font.end());
    fam._styles[{}].Source = "";
}

}
