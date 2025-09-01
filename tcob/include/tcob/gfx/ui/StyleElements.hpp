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
#include "tcob/gfx/ui/Paint.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

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

enum class icon_text_order : u8 {
    IconBeforeText,
    TextBeforeIcon,
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

    void lerp(caret_element const& from, caret_element const& to, f64 step);

    auto operator==(caret_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API shadow_element {
public:
    color  Color {colors::Transparent};
    length OffsetX {};
    length OffsetY {};

    void lerp(shadow_element const& from, shadow_element const& to, f64 step);

    auto operator==(shadow_element const& other) const -> bool = default;
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

    void lerp(deco_element const& from, deco_element const& to, f64 step);

    auto operator==(deco_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API text_element {
public:
    asset_ptr<gfx::font_family> Font;
    color                       Color {colors::White};
    color                       SelectColor {colors::Blue};
    shadow_element              Shadow {.Color = colors::Transparent, .OffsetX = length {1, length::type::Absolute}, .OffsetY = length {1, length::type::Absolute}};
    deco_element                Decoration {};
    gfx::font::style            Style {};
    length                      Size {16, length::type::Absolute};
    gfx::alignments             Alignment {.Horizontal = gfx::horizontal_alignment::Centered, .Vertical = gfx::vertical_alignment::Middle};
    text_transform              Transform {text_transform::None};
    auto_size_mode              AutoSize {auto_size_mode::Never};

    auto calc_font_size(f32 height) const -> u32;

    void lerp(text_element const& from, text_element const& to, f64 step);

    auto operator==(text_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API border_element {
public:
    border_type         Type {border_type::Solid};
    paint               Background {colors::Transparent};
    length              Radius {};
    length              Size {};
    std::vector<length> Dash {};
    f32                 DashOffset {0};

    auto thickness() const -> thickness;

    void lerp(border_element const& from, border_element const& to, f64 step);

    auto operator==(border_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API thumb_element {
public:
    struct context {
        orientation Orientation {orientation::Horizontal};
        f32         RelativePosition {0.0f};
    };

    thumb_type     Type {thumb_type::Rect};
    paint          Background {colors::White};
    length         LongSide {};
    length         ShortSide {};
    border_element Border {};

    auto calc(rect_f const& rect, context const& ctx) const -> rect_f;

    void lerp(thumb_element const& from, thumb_element const& to, f64 step);

    auto operator==(thumb_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API tick_element {
public:
    tick_type Type {tick_type::Checkmark};
    paint     Foreground {colors::White};
    length    Size {};

    void lerp(tick_element const& from, tick_element const& to, f64 step);

    auto operator==(tick_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API bar_element {
public:
    enum class position : u8 {
        LeftOrTop,
        RightOrBottom,
        CenterOrMiddle
    };

    enum class type : u8 {
        Low,
        High,
        Empty
    };

    struct context {
        orientation Orientation {orientation::Horizontal};
        position    Position {position::CenterOrMiddle};

        std::vector<f32>  Stops;
        std::vector<type> StopPattern;
    };

    paint          LowerBackground {colors::White};
    paint          HigherBackground {colors::White};
    length         Size {1, length::type::Relative};
    border_element Border {};
    milliseconds   Delay {0};

    auto calc(rect_f const& rect, orientation orien, position align) const -> rect_f;

    void lerp(bar_element const& from, bar_element const& to, f64 step);

    auto operator==(bar_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API scrollbar_element {
public:
    bar_element Bar;
    utf8_string ThumbClass {"scrollbar_thumb"};

    void lerp(scrollbar_element const& from, scrollbar_element const& to, f64 step);

    auto operator==(scrollbar_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API nav_arrow_element {
public:
    nav_arrow_type Type {nav_arrow_type::Triangle};
    paint          UpBackground {colors::Transparent};
    paint          DownBackground {colors::Transparent};
    paint          Foreground {colors::Transparent};
    dimensions     Size {};
    border_element Border {};
    thickness      Padding {};

    auto calc(rect_f const& rect) const -> rect_f;

    void lerp(nav_arrow_element const& from, nav_arrow_element const& to, f64 step);

    auto operator==(nav_arrow_element const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API item_element {
public:
    text_element    Text;
    paint           Background {colors::White};
    border_element  Border {};
    thickness       Padding {};
    icon_text_order IconTextOrder {icon_text_order::IconBeforeText};

    void lerp(item_element const& from, item_element const& to, f64 step);

    auto operator==(item_element const& other) const -> bool = default;
};

}
