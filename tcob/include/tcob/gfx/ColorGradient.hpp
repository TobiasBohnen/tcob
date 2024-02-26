// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <map>
#include <span>

#include "tcob/core/Color.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API color_stop final {
public:
    color_stop() = default;
    color_stop(f32 pos, color c);

    f32   Position {0};
    color Value {};

    void static Serialize(color_stop const& v, auto&& s);
    auto static Deserialize(color_stop& v, auto&& s) -> bool;
};

////////////////////////////////////////////////////////////

class TCOB_API color_gradient final {
    static constexpr u32 Size {256};

public:
    color_gradient();
    color_gradient(color startColor, color endColor, bool preMulAlpha = true);
    explicit color_gradient(std::span<color_stop> colorStops, bool preMulAlpha = true);

    auto get_color_array(f32 multAlpha) const -> std::array<vec4, Size>;
    auto get_colors() const -> std::array<color, Size>;

    friend auto operator==(color_gradient const& left, color_gradient const& right) -> bool;

private:
    bool                 _premulAlpha {true};
    std::map<u32, color> _colorStops;
};

}

#include "ColorGradient.inl"
