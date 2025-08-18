// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API icon {
public:
    assets::asset_ptr<gfx::texture> Texture {};
    string                          TextureRegion {"default"};
    color                           Color {colors::White};

    auto operator==(icon const& other) const -> bool = default;
};

}
