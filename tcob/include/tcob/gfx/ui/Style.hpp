// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/FontFamily.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

enum class border_type : u8 {
    Solid,
    Double,
    Dotted,
    Dashed,
    Cornered,
    Centered,
    Inset,
    Outset,
    // Groove,
    // Ridge,
    Wavy,
    Hidden
};

////////////////////////////////////////////////////////////

enum class line_type : u8 {
    Solid,
    Double,
    Dotted,
    Dashed,
    Wavy,
    Hidden
};

////////////////////////////////////////////////////////////

enum class text_transform : u8 {
    None,
    Capitalize,
    Uppercase,
    Lowercase
};

////////////////////////////////////////////////////////////

enum class auto_size_mode : u8 {
    Never,
    Always,
    OnlyGrow,
    OnlyShrink
};

////////////////////////////////////////////////////////////

enum class thumb_type : u8 {
    Rect,
    Disc
};

////////////////////////////////////////////////////////////

enum class tick_type : u8 {
    Checkmark,
    Cross,
    Disc,
    Circle,
    Rect,
    Square,
    Triangle
};

////////////////////////////////////////////////////////////

enum class nav_arrow_type : u8 {
    Triangle,
    Chevron,
    Arrow
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API caret_element {
public:
    color        Color {colors::Transparent};
    length       Width {3, length::type::Absolute};
    milliseconds BlinkRate {500};

    auto operator==(caret_element const& other) const -> bool = default;

    void static Transition(caret_element& target, caret_element const& left, caret_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API shadow_element {
public:
    color  Color {colors::Transparent};
    length OffsetX {};
    length OffsetY {};

    auto operator==(shadow_element const& other) const -> bool = default;

    void static Transition(shadow_element& target, shadow_element const& left, shadow_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API deco_element {
public:
    class TCOB_API line {
    public:
        bool Underline {false};
        bool Overline {false};
        bool LineThrough {false};

        auto operator==(line const& other) const -> bool = default;
    };

    line      Line {};
    line_type Style {line_type::Solid};
    color     Color {colors::Transparent};
    length    Size {};

    auto operator==(deco_element const& other) const -> bool = default;

    void static Transition(deco_element& target, deco_element const& left, deco_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API text_element {
public:
    assets::asset_ptr<gfx::font_family> Font;
    color                               Color {colors::White};
    color                               SelectColor {colors::Blue};
    shadow_element                      Shadow {.Color = colors::Transparent, .OffsetX = length {1, length::type::Absolute}, .OffsetY = length {1, length::type::Absolute}};
    deco_element                        Decoration {};
    gfx::font::style                    Style {};
    length                              Size {16, length::type::Absolute};
    gfx::alignments                     Alignment {.Horizontal = gfx::horizontal_alignment::Centered, .Vertical = gfx::vertical_alignment::Middle};
    text_transform                      Transform {text_transform::None};
    auto_size_mode                      AutoSize {auto_size_mode::Never};

    auto calc_font_size(rect_f const& rect) const -> u32;

    auto operator==(text_element const& other) const -> bool = default;

    void static Transition(text_element& target, text_element const& left, text_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API border_element {
public:
    border_type         Type {border_type::Solid};
    ui_paint            Background {colors::Transparent};
    length              Radius {};
    length              Size {};
    std::vector<length> Dash {};
    f32                 DashOffset {0};

    auto thickness() const -> thickness;

    auto operator==(border_element const& other) const -> bool = default;

    void static Transition(border_element& target, border_element const& left, border_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API thumb_element {
public:
    struct context {
        orientation Orientation {orientation::Horizontal};
        f32         RelativePosition {0.0f};
    };

    thumb_type     Type {thumb_type::Rect};
    ui_paint       Background {colors::White};
    length         LongSide {};
    length         ShortSide {};
    border_element Border {};

    auto calc(rect_f const& rect, context const& ctx) const -> rect_f;

    auto operator==(thumb_element const& other) const -> bool = default;

    void static Transition(thumb_element& target, thumb_element const& left, thumb_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API tick_element {
public:
    tick_type Type {tick_type::Checkmark};
    ui_paint  Foreground {colors::White};
    length    Size {};

    auto operator==(tick_element const& other) const -> bool = default;

    void static Transition(tick_element& target, tick_element const& left, tick_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API bar_element {
public:
    enum class position : u8 {
        LeftOrTop,
        RightOrBottom,
        CenterOrMiddle
    };

    struct context {
        orientation Orientation {orientation::Horizontal};
        position    Position {position::CenterOrMiddle};

        std::vector<f32> Stops;
    };

    ui_paint       LowerBackground {colors::White};
    ui_paint       HigherBackground {colors::White};
    length         Size {1, length::type::Relative};
    border_element Border {};
    milliseconds   MotionDuration {0};

    auto calc(rect_f const& rect, orientation orien, position align) const -> rect_f;

    auto operator==(bar_element const& other) const -> bool = default;

    void static Transition(bar_element& target, bar_element const& left, bar_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API scrollbar_element {
public:
    bar_element Bar;
    utf8_string ThumbClass {"scrollbar_thumb"};

    auto operator==(scrollbar_element const& other) const -> bool = default;

    void static Transition(scrollbar_element& target, scrollbar_element const& left, scrollbar_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API nav_arrow_element {
public:
    nav_arrow_type Type {nav_arrow_type::Triangle};
    ui_paint       UpBackground {colors::Transparent};
    ui_paint       DownBackground {colors::Transparent};
    ui_paint       Foreground {colors::Transparent};
    dimensions     Size {};
    border_element Border {};
    thickness      Padding {};

    auto calc(rect_f const& rect) const -> rect_f;

    auto operator==(nav_arrow_element const& other) const -> bool = default;

    void static Transition(nav_arrow_element& target, nav_arrow_element const& left, nav_arrow_element const& right, f64 step);
};

////////////////////////////////////////////////////////////

class TCOB_API item_element {
public:
    text_element   Text;
    ui_paint       Background {colors::White};
    border_element Border {};
    thickness      Padding {};

    auto operator==(item_element const& other) const -> bool = default;

    void static Transition(item_element& target, item_element const& left, item_element const& right, f64 step);
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API style {
public:
    style()                                               = default;
    style(style const& other) noexcept                    = default;
    auto operator=(style const& other) noexcept -> style& = default;
    style(style&& other) noexcept                         = default;
    auto operator=(style&& other) noexcept -> style&      = default;
    virtual ~style()                                      = default;
};

class TCOB_API widget_style : public style {
public:
    thickness Padding {};
    thickness Margin {};

    ui_paint       Background {colors::Transparent};
    shadow_element DropShadow {colors::Transparent, length {5, length::type::Absolute}, length {5, length::type::Absolute}};
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
