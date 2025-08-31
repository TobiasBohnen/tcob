// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <format>

#include "tcob/core/AngleUnits.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

struct hsx {
    degree_f Hue;
    f32      Saturation {};
    f32      X {};
};

auto constexpr operator==(hsx left, hsx right) -> bool;

////////////////////////////////////////////////////////////

class [[nodiscard]] TCOB_API color final {
public:
    constexpr color() = default;
    constexpr color(u8 r, u8 g, u8 b, u8 a = 255);

    auto static constexpr FromRGBA(u32 value) -> color;
    auto static constexpr FromRGB(u32 value) -> color;
    auto static constexpr FromABGR(u32 value) -> color;

    auto static constexpr FromHSLA(hsx const& hsl, u8 a = 255) -> color;
    auto static constexpr FromHSVA(hsx const& hsv, u8 a = 255) -> color;

    auto constexpr as_grayscale [[nodiscard]] (f32 redFactor = 0.299f, f32 greenFactor = 0.587f, f32 blueFactor = 0.114f) const -> color;
    auto constexpr as_alpha_premultiplied [[nodiscard]] () const -> color;

    auto constexpr value() const -> u32;

    auto constexpr to_array [[nodiscard]] () const -> std::array<u8, 4>;
    auto constexpr to_float_array [[nodiscard]] () const -> std::array<f32, 4>;
    auto constexpr to_hsl() const -> hsx;
    auto constexpr to_hsv() const -> hsx;

    auto static constexpr Lerp(color from, color to, f64 step) -> color;

    auto static FromString(string_view name) -> color;

    u8 R {0};
    u8 G {0};
    u8 B {0};
    u8 A {0};

    auto static constexpr Members();
};

auto constexpr operator==(color left, color right) -> bool;

}

#include "Color.inl"

template <>
struct std::formatter<tcob::color> {
    auto constexpr parse(format_parse_context& ctx) { return ctx.begin(); }
    auto format(tcob::color val, format_context& ctx) const { return format_to(ctx.out(), "(r:{},g:{},b:{},a:{})", val.R, val.G, val.B, val.A); }
};
