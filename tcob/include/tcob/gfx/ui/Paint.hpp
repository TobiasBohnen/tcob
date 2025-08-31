// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <variant>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

class TCOB_API linear_gradient {
public:
    degree_f            Angle {0};
    gfx::color_gradient Colors;

    auto operator==(linear_gradient const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API radial_gradient {
public:
    length              InnerRadius {0.0f, length::type::Relative};
    length              OuterRadius {1.0f, length::type::Relative};
    size_f              Scale {size_f::One};
    gfx::color_gradient Colors;

    auto operator==(radial_gradient const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API box_gradient {
public:
    length              Radius {0.25f, length::type::Relative};
    length              Feather {0.50f, length::type::Relative};
    gfx::color_gradient Colors;

    auto operator==(box_gradient const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API nine_patch {
public:
    assets::asset_ptr<gfx::texture> Texture;
    string                          TextureRegion {"default"};
    rect_f                          UV;

    auto operator==(nine_patch const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

using paint = std::variant<color, linear_gradient, radial_gradient, box_gradient, nine_patch>;

TCOB_API void paint_lerp(paint& target, paint const& from, paint const& to, f64 step);

}
