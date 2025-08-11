// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/Paint.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

enum class easing_func : u8 {
    Linear,
    SmoothStep,
    SmootherStep,
    QuadIn,
    QuadOut,
    QuadInOut,
    CubicIn,
    CubicOut,
    CubicInOut,
    QuartIn,
    QuartOut,
    QuartInOut,
    QuintIn,
    QuintOut,
    QuintInOut,
    ExpoIn,
    ExpoOut,
    ExpoInOut,
};

////////////////////////////////////////////////////////////

class TCOB_API style {
public:
    style()                                               = default;
    style(style const& other) noexcept                    = default;
    auto operator=(style const& other) noexcept -> style& = default;
    style(style&& other) noexcept                         = default;
    auto operator=(style&& other) noexcept -> style&      = default;
    virtual ~style()                                      = default;

    easing_func EasingFunc {easing_func::Linear};

    auto ease_value(f64 t) const -> f64;
};

class TCOB_API widget_style : public style {
public:
    thickness Padding {};
    thickness Margin {};

    paint          Background {colors::Transparent};
    shadow_element DropShadow {.Color = colors::Transparent, .OffsetX = length {5, length::type::Absolute}, .OffsetY = length {5, length::type::Absolute}};
    border_element Border;

    void static Transition(widget_style& target, widget_style const& left, widget_style const& right, f64 step);
};

class TCOB_API thumb_style : public style {
public:
    thumb_element Thumb;

    void static Transition(thumb_style& target, thumb_style const& left, thumb_style const& right, f64 step);
};

class TCOB_API nav_arrows_style : public style {
public:
    nav_arrow_element NavArrow;

    void static Transition(nav_arrows_style& target, nav_arrows_style const& left, nav_arrows_style const& right, f64 step);
};

class TCOB_API item_style : public style {
public:
    item_element Item;

    void static Transition(item_style& target, item_style const& left, item_style const& right, f64 step);
};

}
