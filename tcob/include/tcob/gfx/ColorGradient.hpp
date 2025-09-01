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
#include <tuple>

#include "tcob/core/Color.hpp"
#include "tcob/core/Serialization.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API color_stop final {
public:
    color_stop() = default;
    color_stop(f32 pos, color c);

    f32   Position {0};
    color Value {};

    auto operator==(color_stop const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&color_stop::Position> {"pos"},
            member<&color_stop::Value> {"value"}};
    }
};

////////////////////////////////////////////////////////////

class TCOB_API color_gradient final {
public:
    static constexpr u32 Size {256};

    color_gradient();
    color_gradient(std::initializer_list<color const> colors);
    color_gradient(std::initializer_list<color_stop const> colorStops);
    color_gradient(std::span<color_stop const> colorStops);

    auto colors(bool preMulAlpha = true) const -> std::array<color, Size>;

    auto is_single_color() const -> bool;
    auto first_color() const -> color;
    auto get_color_at(u32 key) const -> color;

    auto operator==(color_gradient const& other) const -> bool = default;

    static auto Lerp(color_gradient const& from, color_gradient const& to, f64 step) -> color_gradient;

private:
    std::map<u32, color> _colorStops;
};

}
