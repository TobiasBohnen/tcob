// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <initializer_list>
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

void Serialize(color_stop const& v, auto&& s)
{
    s["pos"]   = v.Position;
    s["value"] = v.Value;
}

auto Deserialize(color_stop& v, auto&& s) -> bool
{
    return s.try_get(v.Position, "pos") && s.try_get(v.Value, "value");
}

////////////////////////////////////////////////////////////

class TCOB_API color_gradient final {
public:
    static constexpr u32 Size {256};

    color_gradient();
    color_gradient(color startColor, color endColor, bool preMulAlpha = true);
    explicit color_gradient(std::initializer_list<color_stop const> colorStops, bool preMulAlpha = true);
    explicit color_gradient(std::span<color_stop const> colorStops, bool preMulAlpha = true);

    auto colors() const -> std::array<color, Size>;

    auto is_single_color() const -> bool;
    auto first_color() const -> color;
    auto get_color_at(u32 key) const -> color;

    auto operator==(color_gradient const& other) const -> bool = default;

    auto static Lerp(color_gradient const& left, color_gradient const& right, f64 step) -> color_gradient;

private:
    bool                 _premulAlpha {true};
    std::map<u32, color> _colorStops;
};

}
