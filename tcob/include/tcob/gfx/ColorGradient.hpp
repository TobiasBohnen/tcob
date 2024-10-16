// Copyright (c) 2024 Tobias Bohnen
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
};

////////////////////////////////////////////////////////////

class TCOB_API color_gradient final {
public:
    static constexpr u32 Size {256};

    color_gradient();
    color_gradient(color startColor, color endColor, bool preMulAlpha = true);
    explicit color_gradient(std::initializer_list<color_stop const> colorStops, bool preMulAlpha = true);
    explicit color_gradient(std::span<color_stop const> colorStops, bool preMulAlpha = true);

    auto get_colors() const -> std::array<color, Size>;
    auto get_single_color() const -> std::optional<color>; // meh

    auto operator==(color_gradient const& other) const -> bool = default;

private:
    bool                 _premulAlpha {true};
    std::map<u32, color> _colorStops;
};

}
