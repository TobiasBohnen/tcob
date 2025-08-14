// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Color.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <tuple>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Serialization.hpp"

namespace tcob {

constexpr color::color(u8 r, u8 g, u8 b, u8 a)
    : R {r}
    , G {g}
    , B {b}
    , A {a}
{
}

auto constexpr color::FromRGBA(u32 value) -> color
{
    return {static_cast<u8>((value >> 24) & 0xFF),
            static_cast<u8>((value >> 16) & 0xFF),
            static_cast<u8>((value >> 8) & 0xFF),
            static_cast<u8>((value >> 0) & 0xFF)};
}

auto constexpr color::FromABGR(u32 value) -> color
{
    return {static_cast<u8>((value >> 0) & 0xFF),
            static_cast<u8>((value >> 8) & 0xFF),
            static_cast<u8>((value >> 16) & 0xFF),
            static_cast<u8>((value >> 24) & 0xFF)};
}

auto constexpr color::FromRGB(u32 value) -> color
{
    return {static_cast<u8>((value >> 16) & 0xFF),
            static_cast<u8>((value >> 8) & 0xFF),
            static_cast<u8>((value >> 0) & 0xFF)};
}

auto constexpr color::FromHSLA(hsx const& hsl, u8 a) -> color
{
    auto constexpr HueToRGB {[](f32 v1, f32 v2, f32 vH) {
        if (vH < 0) { vH += 1; }
        if (vH > 1) { vH -= 1; }
        if ((vH * 6) < 1) { return (v1 + ((v2 - v1) * 6 * vH)); }
        if ((vH * 2) < 1) { return v2; }
        if ((vH * 3) < 2) { return (v1 + ((v2 - v1) * ((2.0f / 3) - vH) * 6)); }
        return v1;
    }};

    color retValue;
    if (hsl.Saturation == 0) {
        retValue.R = retValue.G = retValue.B = static_cast<u8>(hsl.X * 255);
    } else {
        f32 const hue {hsl.Hue.Value / 360};
        f32 const v2 {(hsl.X < 0.5f) ? (hsl.X * (1 + hsl.Saturation)) : ((hsl.X + hsl.Saturation) - (hsl.X * hsl.Saturation))};
        f32 const v1 {(2 * hsl.X) - v2};

        retValue.R = static_cast<u8>(std::round(HueToRGB(v1, v2, hue + (1.0f / 3)) * 255.0f));
        retValue.G = static_cast<u8>(std::round(HueToRGB(v1, v2, hue) * 255.0f));
        retValue.B = static_cast<u8>(std::round(HueToRGB(v1, v2, hue - (1.0f / 3)) * 255.0f));
    }

    retValue.A = a;
    return retValue;
}

auto constexpr color::FromHSVA(hsx const& hsv, u8 a) -> color
{
    color retValue;
    if (hsv.Saturation == 0.0f) {
        retValue.R = retValue.G = retValue.B = static_cast<u8>(std::round(hsv.X * 255.0f));
    } else {
        i32 const hi {static_cast<i32>(hsv.Hue.Value / 60) % 6};
        f32 const f {(hsv.Hue.Value / 60) - hi};
        f32 const p {hsv.X * (1 - hsv.Saturation)};
        f32 const q {hsv.X * (1 - (hsv.Saturation * f))};
        f32 const t {hsv.X * (1 - (hsv.Saturation * (1 - f)))};

        switch (hi) {
        case 0:
            retValue.R = static_cast<u8>(std::round(hsv.X * 255.0f));
            retValue.G = static_cast<u8>(std::round(t * 255.0f));
            retValue.B = static_cast<u8>(std::round(p * 255.0f));
            break;
        case 1:
            retValue.R = static_cast<u8>(std::round(q * 255.0f));
            retValue.G = static_cast<u8>(std::round(hsv.X * 255.0f));
            retValue.B = static_cast<u8>(std::round(p * 255.0f));
            break;
        case 2:
            retValue.R = static_cast<u8>(std::round(p * 255.0f));
            retValue.G = static_cast<u8>(std::round(hsv.X * 255.0f));
            retValue.B = static_cast<u8>(std::round(t * 255.0f));
            break;
        case 3:
            retValue.R = static_cast<u8>(std::round(p * 255.0f));
            retValue.G = static_cast<u8>(std::round(q * 255.0f));
            retValue.B = static_cast<u8>(std::round(hsv.X * 255.0f));
            break;
        case 4:
            retValue.R = static_cast<u8>(std::round(t * 255.0f));
            retValue.G = static_cast<u8>(std::round(p * 255.0f));
            retValue.B = static_cast<u8>(std::round(hsv.X * 255.0f));
            break;
        case 5:
            retValue.R = static_cast<u8>(std::round(hsv.X * 255.0f));
            retValue.G = static_cast<u8>(std::round(p * 255.0f));
            retValue.B = static_cast<u8>(std::round(q * 255.0f));
            break;
        default:
            break;
        }
    }

    retValue.A = a;
    return retValue;
}

auto constexpr color::as_grayscale(f32 redFactor, f32 greenFactor, f32 blueFactor) const -> color
{
    u8 const value {static_cast<u8>((R * redFactor) + (G * greenFactor) + (B * blueFactor))};
    return {value, value, value};
}

auto constexpr color::Lerp(color left, color right, f64 step) -> color
{
    u8 const nr {static_cast<u8>(left.R + ((right.R - left.R) * step))};
    u8 const ng {static_cast<u8>(left.G + ((right.G - left.G) * step))};
    u8 const nb {static_cast<u8>(left.B + ((right.B - left.B) * step))};
    u8 const na {static_cast<u8>(left.A + ((right.A - left.A) * step))};
    return {nr, ng, nb, na};
}

auto constexpr color::as_alpha_premultiplied() const -> color
{
    f32 const factor {static_cast<f32>(A) / 255.0f};
    u8 const  nr {static_cast<u8>(static_cast<f32>(R) * factor)};
    u8 const  ng {static_cast<u8>(static_cast<f32>(G) * factor)};
    u8 const  nb {static_cast<u8>(static_cast<f32>(B) * factor)};
    return {nr, ng, nb, A};
}

auto constexpr color::value() const -> u32
{
    return static_cast<u32>(R << 24 | G << 16 | B << 8 | A);
}

auto constexpr color::to_array() const -> std::array<u8, 4>
{
    return {R, G, B, A};
}

auto constexpr color::to_float_array [[nodiscard]] () const -> std::array<f32, 4>
{
    return {R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f};
}

auto constexpr color::to_hsl() const -> hsx
{
    f32 const r {R / 255.0f};
    f32 const g {G / 255.0f};
    f32 const b {B / 255.0f};

    f32 const max_val {std::max({r, g, b})};
    f32 const min_val {std::min({r, g, b})};

    f32       h {0}, s {0};
    f32 const l {(max_val + min_val) / 2.0f};

    if (max_val == min_val) {
        h = s = 0; // achromatic
    } else {
        f32 const d {max_val - min_val};
        s = l > 0.5f ? d / (2.0f - max_val - min_val) : d / (max_val + min_val);

        if (max_val == r) {
            h = (g - b) / d + (g < b ? 6.0f : 0.0f);
        } else if (max_val == g) {
            h = (b - r) / d + 2.0f;
        } else {
            h = (r - g) / d + 4.0f;
        }

        h /= 6.0f;
    }

    return {.Hue = degree_f {h * 360.0f}, .Saturation = s, .X = l};
}

auto constexpr color::to_hsv() const -> hsx
{
    f32 const r {R / 255.0f};
    f32 const g {G / 255.0f};
    f32 const b {B / 255.0f};

    f32 const max_val {std::max({r, g, b})};
    f32 const min_val {std::min({r, g, b})};

    f32       h {0};
    f32 const v {max_val};

    f32 const d {max_val - min_val};
    f32 const s {max_val == 0 ? 0 : d / max_val};

    if (max_val == min_val) {
        h = 0; // achromatic
    } else {
        if (max_val == r) {
            h = (g - b) / d + (g < b ? 6.0f : 0.0f);
        } else if (max_val == g) {
            h = (b - r) / d + 2.0f;
        } else {
            h = (r - g) / d + 4.0f;
        }

        h /= 6.0f;
    }

    return {.Hue = degree_f {h * 360.0f}, .Saturation = s, .X = v};
}

auto constexpr operator==(color left, color right) -> bool
{
    return (left.R == right.R) && (left.G == right.G) && (left.B == right.B) && (left.A == right.A);
}

auto constexpr operator==(hsx left, hsx right) -> bool
{
    return (left.Hue == right.Hue) && (left.Saturation == right.Saturation) && (left.X == right.X);
}

auto constexpr color::Members()
{
    return std::tuple {
        member<&color::R> {"r"},
        member<&color::G> {"g"},
        member<&color::B> {"b"},
        optional_member<&color::A, u8 {255}> {"a"},
    };
}

}

////////////////////////////////////////////////////////////

namespace tcob::literals {
inline auto operator""_color(char const* str, usize) -> color
{
    return color::FromString(str);
}
}

////////////////////////////////////////////////////////////

namespace tcob::colors {
inline constexpr color AliceBlue {color::FromRGBA(0xF0F8FFFF)};
inline constexpr color AntiqueWhite {color::FromRGBA(0xFAEBD7FF)};
inline constexpr color Aqua {color::FromRGBA(0x00FFFFFF)};
inline constexpr color Aquamarine {color::FromRGBA(0x7FFFD4FF)};
inline constexpr color Azure {color::FromRGBA(0xF0FFFFFF)};
inline constexpr color Beige {color::FromRGBA(0xF5F5DCFF)};
inline constexpr color Bisque {color::FromRGBA(0xFFE4C4FF)};
inline constexpr color Black {color::FromRGBA(0x000000FF)};
inline constexpr color BlanchedAlmond {color::FromRGBA(0xFFEBCDFF)};
inline constexpr color Blue {color::FromRGBA(0x0000FFFF)};
inline constexpr color BlueViolet {color::FromRGBA(0x8A2BE2FF)};
inline constexpr color Brown {color::FromRGBA(0xA52A2AFF)};
inline constexpr color BurlyWood {color::FromRGBA(0xDEB887FF)};
inline constexpr color CadetBlue {color::FromRGBA(0x5F9EA0FF)};
inline constexpr color Chartreuse {color::FromRGBA(0x7FFF00FF)};
inline constexpr color Chocolate {color::FromRGBA(0xD2691EFF)};
inline constexpr color Coral {color::FromRGBA(0xFF7F50FF)};
inline constexpr color CornflowerBlue {color::FromRGBA(0x6495EDFF)};
inline constexpr color Cornsilk {color::FromRGBA(0xFFF8DCFF)};
inline constexpr color Crimson {color::FromRGBA(0xDC143CFF)};
inline constexpr color Cyan {color::FromRGBA(0x00FFFFFF)};
inline constexpr color DarkBlue {color::FromRGBA(0x00008BFF)};
inline constexpr color DarkCyan {color::FromRGBA(0x008B8BFF)};
inline constexpr color DarkGoldenRod {color::FromRGBA(0xB8860BFF)};
inline constexpr color DarkGray {color::FromRGBA(0xA9A9A9FF)};
inline constexpr color DarkGreen {color::FromRGBA(0x006400FF)};
inline constexpr color DarkKhaki {color::FromRGBA(0xBDB76BFF)};
inline constexpr color DarkMagenta {color::FromRGBA(0x8B008BFF)};
inline constexpr color DarkOliveGreen {color::FromRGBA(0x556B2FFF)};
inline constexpr color DarkOrange {color::FromRGBA(0xFF8C00FF)};
inline constexpr color DarkOrchid {color::FromRGBA(0x9932CCFF)};
inline constexpr color DarkRed {color::FromRGBA(0x8B0000FF)};
inline constexpr color DarkSalmon {color::FromRGBA(0xE9967AFF)};
inline constexpr color DarkSeaGreen {color::FromRGBA(0x8FBC8FFF)};
inline constexpr color DarkSlateBlue {color::FromRGBA(0x483D8BFF)};
inline constexpr color DarkSlateGray {color::FromRGBA(0x2F4F4FFF)};
inline constexpr color DarkTurquoise {color::FromRGBA(0x00CED1FF)};
inline constexpr color DarkViolet {color::FromRGBA(0x9400D3FF)};
inline constexpr color DeepPink {color::FromRGBA(0xFF1493FF)};
inline constexpr color DeepSkyBlue {color::FromRGBA(0x00BFFFFF)};
inline constexpr color DimGray {color::FromRGBA(0x696969FF)};
inline constexpr color DodgerBlue {color::FromRGBA(0x1E90FFFF)};
inline constexpr color FireBrick {color::FromRGBA(0xB22222FF)};
inline constexpr color FloralWhite {color::FromRGBA(0xFFFAF0FF)};
inline constexpr color ForestGreen {color::FromRGBA(0x228B22FF)};
inline constexpr color Fuchsia {color::FromRGBA(0xFF00FFFF)};
inline constexpr color Gainsboro {color::FromRGBA(0xDCDCDCFF)};
inline constexpr color GhostWhite {color::FromRGBA(0xF8F8FFFF)};
inline constexpr color Gold {color::FromRGBA(0xFFD700FF)};
inline constexpr color GoldenRod {color::FromRGBA(0xDAA520FF)};
inline constexpr color Gray {color::FromRGBA(0x808080FF)};
inline constexpr color Green {color::FromRGBA(0x008000FF)};
inline constexpr color GreenYellow {color::FromRGBA(0xADFF2FFF)};
inline constexpr color HoneyDew {color::FromRGBA(0xF0FFF0FF)};
inline constexpr color HotPink {color::FromRGBA(0xFF69B4FF)};
inline constexpr color IndianRed {color::FromRGBA(0xCD5C5CFF)};
inline constexpr color Indigo {color::FromRGBA(0x4B0082FF)};
inline constexpr color Ivory {color::FromRGBA(0xFFFFF0FF)};
inline constexpr color Khaki {color::FromRGBA(0xF0E68CFF)};
inline constexpr color Lavender {color::FromRGBA(0xE6E6FAFF)};
inline constexpr color LavenderBlush {color::FromRGBA(0xFFF0F5FF)};
inline constexpr color LawnGreen {color::FromRGBA(0x7CFC00FF)};
inline constexpr color LemonChiffon {color::FromRGBA(0xFFFACDFF)};
inline constexpr color LightBlue {color::FromRGBA(0xADD8E6FF)};
inline constexpr color LightCoral {color::FromRGBA(0xF08080FF)};
inline constexpr color LightCyan {color::FromRGBA(0xE0FFFFFF)};
inline constexpr color LightGoldenRodYellow {color::FromRGBA(0xFAFAD2FF)};
inline constexpr color LightGray {color::FromRGBA(0xD3D3D3FF)};
inline constexpr color LightGreen {color::FromRGBA(0x90EE90FF)};
inline constexpr color LightPink {color::FromRGBA(0xFFB6C1FF)};
inline constexpr color LightSalmon {color::FromRGBA(0xFFA07AFF)};
inline constexpr color LightSeaGreen {color::FromRGBA(0x20B2AAFF)};
inline constexpr color LightSkyBlue {color::FromRGBA(0x87CEFAFF)};
inline constexpr color LightSlateGray {color::FromRGBA(0x778899FF)};
inline constexpr color LightSteelBlue {color::FromRGBA(0xB0C4DEFF)};
inline constexpr color LightYellow {color::FromRGBA(0xFFFFE0FF)};
inline constexpr color Lime {color::FromRGBA(0x00FF00FF)};
inline constexpr color LimeGreen {color::FromRGBA(0x32CD32FF)};
inline constexpr color Linen {color::FromRGBA(0xFAF0E6FF)};
inline constexpr color Magenta {color::FromRGBA(0xFF00FFFF)};
inline constexpr color Maroon {color::FromRGBA(0x800000FF)};
inline constexpr color MediumAquaMarine {color::FromRGBA(0x66CDAAFF)};
inline constexpr color MediumBlue {color::FromRGBA(0x0000CDFF)};
inline constexpr color MediumOrchid {color::FromRGBA(0xBA55D3FF)};
inline constexpr color MediumPurple {color::FromRGBA(0x9370DBFF)};
inline constexpr color MediumSeaGreen {color::FromRGBA(0x3CB371FF)};
inline constexpr color MediumSlateBlue {color::FromRGBA(0x7B68EEFF)};
inline constexpr color MediumSpringGreen {color::FromRGBA(0x00FA9AFF)};
inline constexpr color MediumTurquoise {color::FromRGBA(0x48D1CCFF)};
inline constexpr color MediumVioletRed {color::FromRGBA(0xC71585FF)};
inline constexpr color MidnightBlue {color::FromRGBA(0x191970FF)};
inline constexpr color MintCream {color::FromRGBA(0xF5FFFAFF)};
inline constexpr color MistyRose {color::FromRGBA(0xFFE4E1FF)};
inline constexpr color Moccasin {color::FromRGBA(0xFFE4B5FF)};
inline constexpr color NavajoWhite {color::FromRGBA(0xFFDEADFF)};
inline constexpr color Navy {color::FromRGBA(0x000080FF)};
inline constexpr color OldLace {color::FromRGBA(0xFDF5E6FF)};
inline constexpr color Olive {color::FromRGBA(0x808000FF)};
inline constexpr color OliveDrab {color::FromRGBA(0x6B8E23FF)};
inline constexpr color Orange {color::FromRGBA(0xFFA500FF)};
inline constexpr color OrangeRed {color::FromRGBA(0xFF4500FF)};
inline constexpr color Orchid {color::FromRGBA(0xDA70D6FF)};
inline constexpr color PaleGoldenRod {color::FromRGBA(0xEEE8AAFF)};
inline constexpr color PaleGreen {color::FromRGBA(0x98FB98FF)};
inline constexpr color PaleTurquoise {color::FromRGBA(0xAFEEEEFF)};
inline constexpr color PaleVioletRed {color::FromRGBA(0xDB7093FF)};
inline constexpr color PapayaWhip {color::FromRGBA(0xFFEFD5FF)};
inline constexpr color PeachPuff {color::FromRGBA(0xFFDAB9FF)};
inline constexpr color Peru {color::FromRGBA(0xCD853FFF)};
inline constexpr color Pink {color::FromRGBA(0xFFC0CBFF)};
inline constexpr color Plum {color::FromRGBA(0xDDA0DDFF)};
inline constexpr color PowderBlue {color::FromRGBA(0xB0E0E6FF)};
inline constexpr color Purple {color::FromRGBA(0x800080FF)};
inline constexpr color RebeccaPurple {color::FromRGBA(0x663399FF)};
inline constexpr color Red {color::FromRGBA(0xFF0000FF)};
inline constexpr color RosyBrown {color::FromRGBA(0xBC8F8FFF)};
inline constexpr color RoyalBlue {color::FromRGBA(0x4169E1FF)};
inline constexpr color SaddleBrown {color::FromRGBA(0x8B4513FF)};
inline constexpr color Salmon {color::FromRGBA(0xFA8072FF)};
inline constexpr color SandyBrown {color::FromRGBA(0xF4A460FF)};
inline constexpr color SeaGreen {color::FromRGBA(0x2E8B57FF)};
inline constexpr color SeaShell {color::FromRGBA(0xFFF5EEFF)};
inline constexpr color Sienna {color::FromRGBA(0xA0522DFF)};
inline constexpr color Silver {color::FromRGBA(0xC0C0C0FF)};
inline constexpr color SkyBlue {color::FromRGBA(0x87CEEBFF)};
inline constexpr color SlateBlue {color::FromRGBA(0x6A5ACDFF)};
inline constexpr color SlateGray {color::FromRGBA(0x708090FF)};
inline constexpr color Snow {color::FromRGBA(0xFFFAFAFF)};
inline constexpr color SpringGreen {color::FromRGBA(0x00FF7FFF)};
inline constexpr color SteelBlue {color::FromRGBA(0x4682B4FF)};
inline constexpr color Tan {color::FromRGBA(0xD2B48CFF)};
inline constexpr color Teal {color::FromRGBA(0x008080FF)};
inline constexpr color Thistle {color::FromRGBA(0xD8BFD8FF)};
inline constexpr color Tomato {color::FromRGBA(0xFF6347FF)};
inline constexpr color Turquoise {color::FromRGBA(0x40E0D0FF)};
inline constexpr color Violet {color::FromRGBA(0xEE82EEFF)};
inline constexpr color Wheat {color::FromRGBA(0xF5DEB3FF)};
inline constexpr color White {color::FromRGBA(0xFFFFFFFF)};
inline constexpr color WhiteSmoke {color::FromRGBA(0xF5F5F5FF)};
inline constexpr color Yellow {color::FromRGBA(0xFFFF00FF)};
inline constexpr color YellowGreen {color::FromRGBA(0x9ACD32FF)};
// additions:
inline constexpr color Transparent {color::FromRGBA(0x00000000)};
}
