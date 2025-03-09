// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <initializer_list>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/FontFamily.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API caret_element {
public:
    color        Color {colors::Transparent};
    length       Width {3, length::type::Absolute};
    milliseconds BlinkRate {500};

    auto operator==(caret_element const& other) const -> bool = default;

    auto static Lerp(caret_element const& left, caret_element const& right, f64 step) -> caret_element;
};

////////////////////////////////////////////////////////////

class TCOB_API shadow_element {
public:
    color  Color {colors::Transparent};
    length OffsetX {};
    length OffsetY {};

    auto operator==(shadow_element const& other) const -> bool = default;

    auto static Lerp(shadow_element const& left, shadow_element const& right, f64 step) -> shadow_element;
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

    enum class style : u8 {
        Solid,
        Double,
        Dotted,
        Dashed,
        Wavy
    };

    line   Line {};
    style  Style {style::Solid};
    color  Color {colors::Transparent};
    length Size {};

    auto operator==(deco_element const& other) const -> bool = default;

    auto static Lerp(deco_element const& left, deco_element const& right, f64 step) -> deco_element;
};

////////////////////////////////////////////////////////////

class TCOB_API text_element {
public:
    enum class transform : u8 {
        None,
        Capitalize,
        Uppercase,
        Lowercase
    };
    enum class auto_size_mode : u8 {
        Never,
        Always,
        OnlyGrow,
        OnlyShrink
    };

    assets::asset_ptr<gfx::font_family> Font;
    color                               Color {colors::White};
    color                               SelectColor {colors::Blue};
    shadow_element                      Shadow {colors::Transparent, length {1, length::type::Absolute}, length {1, length::type::Absolute}};
    deco_element                        Decoration {};
    gfx::font::style                    Style {};
    length                              Size {16, length::type::Absolute};
    gfx::alignments                     Alignment {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
    transform                           Transform {transform::None};
    auto_size_mode                      AutoSize {auto_size_mode::Never};

    auto calc_font_size(rect_f const& rect) const -> u32;

    auto operator==(text_element const& other) const -> bool = default;

    auto static Lerp(text_element const& left, text_element const& right, f64 step) -> text_element;
};

////////////////////////////////////////////////////////////

class TCOB_API border_element {
public:
    enum class type : u8 {
        Solid,
        Double,
        Dotted,
        Dashed,
        // Cornered,
        // Centered,
        // Inset,
        // Outset,
        // Groove,
        // Ridge
        Hidden
    };

    type                Type {type::Solid};
    ui_paint            Background {colors::Transparent};
    length              Radius {};
    length              Size {};
    std::vector<length> Dash {};
    f32                 DashOffset {0};

    auto thickness() const -> thickness;

    auto operator==(border_element const& other) const -> bool = default;

    auto static Lerp(border_element const& left, border_element const& right, f64 step) -> border_element;
};

////////////////////////////////////////////////////////////

class TCOB_API thumb_element {
public:
    enum class type : u8 {
        Rect,
        Disc
    };

    struct context {
        orientation Orientation {orientation::Horizontal};
        bool        Inverted {false};
        f32         Fraction {0.0f};
    };

    type           Type {type::Rect};
    ui_paint       Background {colors::White};
    length         LongSide {};
    length         ShortSide {};
    border_element Border {};

    auto calc(rect_f const& rect, context const& ctx) const -> rect_f;

    auto operator==(thumb_element const& other) const -> bool = default;

    auto static Lerp(thumb_element const& left, thumb_element const& right, f64 step) -> thumb_element;
};

////////////////////////////////////////////////////////////

class TCOB_API tick_element {
public:
    enum class type : u8 {
        None,
        Checkmark,
        Cross,
        Disc,
        Circle,
        Rect,
        Square
    };

    type     Type {type::Checkmark};
    ui_paint Foreground {colors::White};
    length   Size {};

    auto operator==(tick_element const& other) const -> bool = default;

    auto static Lerp(tick_element const& left, tick_element const& right, f64 step) -> tick_element;
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
        Continuous,
        Blocks
    };

    struct context {
        orientation Orientation {orientation::Horizontal};
        bool        Inverted {false};
        position    Position {position::CenterOrMiddle};
        i32         BlockCount {0};
        f32         Fraction {0};
    };

    type           Type {type::Continuous};
    ui_paint       LowerBackground {colors::White};
    ui_paint       HigherBackground {colors::White};
    length         Size {1, length::type::Relative};
    border_element Border {};
    milliseconds   Delay {0};

    auto calc(rect_f const& rect, orientation orien, position align) const -> rect_f;

    auto operator==(bar_element const& other) const -> bool = default;

    auto static Lerp(bar_element const& left, bar_element const& right, f64 step) -> bar_element;
};

////////////////////////////////////////////////////////////

class TCOB_API scrollbar_element {
public:
    bar_element Bar;
    utf8_string ThumbClass {"scrollbar_thumb"};

    auto operator==(scrollbar_element const& other) const -> bool = default;

    auto static Lerp(scrollbar_element const& left, scrollbar_element const& right, f64 step) -> scrollbar_element;
};

////////////////////////////////////////////////////////////

class TCOB_API nav_arrow_element {
public:
    enum class type : u8 {
        None,
        Triangle
    };

    type           Type {type::Triangle};
    ui_paint       UpBackground {colors::Transparent};
    ui_paint       DownBackground {colors::Transparent};
    ui_paint       Foreground {colors::Transparent};
    dimensions     Size {};
    border_element Border {};
    thickness      Padding {};

    auto calc(rect_f const& rect) const -> rect_f;

    auto operator==(nav_arrow_element const& other) const -> bool = default;

    auto static Lerp(nav_arrow_element const& left, nav_arrow_element const& right, f64 step) -> nav_arrow_element;
};

////////////////////////////////////////////////////////////

class TCOB_API item_element {
public:
    text_element   Text;
    ui_paint       Background {colors::White};
    border_element Border {};
    thickness      Padding {};

    auto operator==(item_element const& other) const -> bool = default;

    auto static Lerp(item_element const& left, item_element const& right, f64 step) -> item_element;
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

////////////////////////////////////////////////////////////

class TCOB_API style_attributes final {
public:
    style_attributes() = default;
    style_attributes(std::initializer_list<std::pair<string, widget_attribute_types> const> values);

    auto check(widget_attributes const& widgetAttribs) const -> bool;

    auto operator==(style_attributes const& other) const -> bool = default;

private:
    std::unordered_map<string, std::unordered_set<widget_attribute_types>> _values;
};

////////////////////////////////////////////////////////////

class TCOB_API style_flags final {
public:
    std::optional<bool> Focus {};
    std::optional<bool> Active {};
    std::optional<bool> Hover {};
    std::optional<bool> Checked {};
    std::optional<bool> Disabled {};

    auto score(widget_flags other) const -> i32;

    auto operator==(style_flags const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API style_collection final {
public:
    template <std::derived_from<style> T>
    auto create(string const& name, style_flags flags, style_attributes const& attribs = {}) -> std::shared_ptr<T>;

    template <std::derived_from<widget> T>
    auto create(string const& name, style_flags flags, style_attributes const& attribs = {}) -> std::shared_ptr<typename T::style>;

    auto get(widget_style_selectors const& select) const -> style*;

    void clear();

private:
    std::vector<std::tuple<string, style_flags, style_attributes, std::shared_ptr<style>>> _styles;
};

}

#include "Style.inl"
