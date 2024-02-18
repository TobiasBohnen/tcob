// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/Font.hpp"

namespace tcob::detail {
////////////////////////////////////////////////////////////

class ini_raster_font_loader : public gfx::raster_font::loader {
public:
    auto load(gfx::raster_font& font, path const& file, string const& textureFolder) -> std::optional<gfx::font::info> override;
};

////////////////////////////////////////////////////////////

class fnt_raster_font_loader : public gfx::raster_font::loader {
public:
    auto load(gfx::raster_font& font, path const& file, string const& textureFolder) -> std::optional<gfx::font::info> override;
};

}
