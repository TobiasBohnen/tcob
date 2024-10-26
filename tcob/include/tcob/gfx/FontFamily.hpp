// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>
#include <vector>

#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Font.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API font_family final {
public:
    explicit font_family(string name);

    auto get_name() const -> string const&;
    auto get_font(font::style style, u32 size) -> assets::asset_ptr<font>;

    auto get_fallback_style(font::style style) const -> std::optional<font::style>;
    auto has_style(font::style style) const -> bool;

    void clear_assets();

    auto get_font_count() const -> isize;

    void static FindSources(font_family& fam, path const& source);
    void static SingleFont(font_family& fam, std::span<ubyte const> font);

    static inline char const* asset_name {"font_family"};

private:
    void set_source(font::style style, path const& file);

    string _name;

    std::unordered_map<font::style, path>                                                    _fontSources;
    std::unordered_map<font::style, std::vector<ubyte>>                                      _fontData;
    std::unordered_map<font::style, std::unordered_map<u32, assets::manual_asset_ptr<font>>> _fontAssets;
};

////////////////////////////////////////////////////////////
}
