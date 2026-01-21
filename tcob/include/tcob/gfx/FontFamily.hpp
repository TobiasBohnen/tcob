// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Font.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API font_family final {
public:
    explicit font_family(string name);

    auto name() const -> string const&;
    auto get_font(font::style style, u32 size) -> asset_ptr<font>;

    auto get_style(font::style want) const -> std::optional<font::style>;
    auto has_style(font::style style) const -> bool;

    static void FindSources(font_family& fam, path const& source);
    static void SingleFont(font_family& fam, std::span<byte const> font);

    static inline char const* AssetName {"font_family"};

private:
    void add_style(font::style style, path const& file);

    string _name;

    struct style_entry {
        path                                           Source;
        std::vector<byte>                              Data;
        std::unordered_map<u32, asset_owner_ptr<font>> Fonts;
    };

    std::unordered_map<font::style, style_entry> _styles;
};

////////////////////////////////////////////////////////////
}
