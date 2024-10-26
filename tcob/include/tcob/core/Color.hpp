// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ostream>

#include "tcob/core/AngleUnits.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

struct hsx {
    degree_f Hue;
    f32      Saturation {};
    f32      X {};
};

auto constexpr operator==(hsx left, hsx right) -> bool;

inline auto operator<<(std::ostream& os, hsx m) -> std::ostream&;

////////////////////////////////////////////////////////////

class [[nodiscard]] TCOB_API color final {
public:
    constexpr color() = default;
    constexpr color(u8 r, u8 g, u8 b, u8 a = 255);

    auto static constexpr FromRGBA(u32 value) -> color;
    auto static constexpr FromRGB(u32 value) -> color;
    auto static constexpr FromHSLA(hsx const& hsl, u8 a = 255) -> color;
    auto static constexpr FromHSVA(hsx const& hsv, u8 a = 255) -> color;

    auto constexpr as_grayscale [[nodiscard]] (f32 redFactor = 0.299f, f32 greenFactor = 0.587f, f32 blueFactor = 0.114f) const -> color;
    auto constexpr as_alpha_premultiplied [[nodiscard]] () const -> color;
    auto constexpr as_array [[nodiscard]] () const -> std::array<u8, 4>;
    auto constexpr as_float_array [[nodiscard]] () const -> std::array<f32, 4>;

    auto constexpr value() const -> u32;

    auto constexpr to_hsl() const -> hsx;
    auto constexpr to_hsv() const -> hsx;

    auto static constexpr Lerp(color left, color right, f64 step) -> color;

    auto static FromString(string_view name) -> color;

    u8 R {0};
    u8 G {0};
    u8 B {0};
    u8 A {0};
};

auto constexpr operator==(color left, color right) -> bool;

inline auto operator<<(std::ostream& os, color m) -> std::ostream&;

void Serialize(color v, auto&& s)
{
    s["r"] = v.R;
    s["g"] = v.G;
    s["b"] = v.B;
    s["a"] = v.A;
}

auto Deserialize(color& v, auto&& s) -> bool
{
    if (s.try_get(v.R, "r") && s.try_get(v.G, "g") && s.try_get(v.B, "b")) {
        if (!s.try_get(v.A, "a")) {
            v.A = u8 {255};
        }
        return true;
    }
    return false;
}

}

#include "Color.inl"

template <>
struct std::formatter<tcob::color> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(tcob::color val, format_context& ctx) const
    {
        return format_to(ctx.out(), "(r:{},g:{},b:{},a:{})", val.R, val.G, val.B, val.A);
    }
};
