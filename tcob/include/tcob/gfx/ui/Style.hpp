// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <initializer_list>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

struct text_decoration {
    struct line {
        bool Underline {false};
        bool Overline {false};
        bool LineThrough {false};
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
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

namespace element {
    ////////////////////////////////////////////////////////////

    class TCOB_API caret {
    public:
        color        Color {colors::Transparent};
        length       Width {3, length::type::Absolute};
        milliseconds BlinkRate {500};
    };

    class TCOB_API shadow {
    public:
        color  Color {colors::Transparent};
        length OffsetX {};
        length OffsetY {};
    };

    class TCOB_API text {
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

        assets::asset_ptr<font_family> Font;
        color                          Color {colors::White};
        shadow                         Shadow {colors::Transparent, length {1, length::type::Absolute}, length {1, length::type::Absolute}};
        text_decoration                Decoration {};
        font::style                    Style {};
        length                         Size {16, length::type::Absolute};
        alignments                     Alignment {horizontal_alignment::Centered, vertical_alignment::Middle};
        transform                      Transform {transform::None};
        auto_size_mode                 AutoSize {auto_size_mode::Never};

        auto calc_font_size(rect_f const& rect) const -> u32;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API border {
    public:
        enum class type : u8 {
            Solid,
            Double,
            //   Dotted,
            //   Dashed,
            Hidden
        };

        type     Type {type::Solid};
        ui_paint Background {colors::Transparent};
        length   Radius {};
        length   Size {};

        auto get_thickness() const -> thickness;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API thumb {
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

        type     Type {type::Rect};
        ui_paint Background {colors::White};
        length   LongSide {};
        length   ShortSide {};
        border   Border {};

        auto calc(rect_f const& rect, context const& ctx) const -> rect_f;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API tick {
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
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API bar {
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

        type         Type {type::Continuous};
        ui_paint     LowerBackground {colors::White};
        ui_paint     HigherBackground {colors::White};
        length       Size {1, length::type::Relative};
        border       Border {};
        milliseconds Delay {0};

        auto calc(rect_f const& rect, orientation orien, position align) const -> rect_f;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API scrollbar {
    public:
        bar    Bar;
        string ThumbClass {"scrollbar_thumb"};

        struct rects {
            rect_f Bar;
            rect_f Thumb;
        };
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API nav_arrow {
    public:
        enum class type : u8 {
            None,
            Triangle
        };

        type     Type {type::Triangle};
        ui_paint IncBackground {colors::Transparent};
        ui_paint DecBackground {colors::Transparent};
        color    Foreground {colors::Transparent};
        length   Width {};
        length   Height {};
        border   Border {};

        auto calc(rect_f const& rect) const -> rect_f;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API item {
    public:
        text      Text;
        ui_paint  Background {colors::White};
        border    Border {};
        thickness Padding {};
    };

}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API style_base {
public:
    style_base()                                                    = default;
    style_base(style_base const& other) noexcept                    = default;
    auto operator=(style_base const& other) noexcept -> style_base& = default;
    style_base(style_base&& other) noexcept                         = default;
    auto operator=(style_base&& other) noexcept -> style_base&      = default;
    virtual ~style_base()                                           = default;

    void virtual offset_content(rect_f& /* bounds */, bool /* isHitTest*/) const { }
};

class TCOB_API style : public style_base {
public:
    thickness Padding {};
    thickness Margin {};

    string Cursor {"default"};

    void offset_content(rect_f& bounds, bool isHitTest) const override;
};

class TCOB_API background_style : public style {
public:
    ui_paint        Background {colors::Transparent};
    element::shadow DropShadow {colors::Transparent, length {5, length::type::Absolute}, length {5, length::type::Absolute}};
    element::border Border;

    void offset_content(rect_f& bounds, bool isHitTest) const override;
};

class TCOB_API thumb_style : public style_base {
public:
    element::thumb Thumb;
};

class TCOB_API nav_arrows_style : public style_base {
public:
    element::nav_arrow NavArrow;
};

class TCOB_API item_style : public style_base {
public:
    element::item Item;
};

////////////////////////////////////////////////////////////

class TCOB_API style_attributes final {
public:
    style_attributes() = default;
    style_attributes(std::initializer_list<std::pair<string, attributes> const> values);

    auto check(widget_attributes const& widgetAttribs) const -> bool;

private:
    std::unordered_map<string, std::set<attributes>> _values;
};

////////////////////////////////////////////////////////////

class TCOB_API style_collection final {
public:
    template <std::derived_from<style_base> T>
    auto create(string const& name, flags flags, style_attributes const& attribs = {}) -> std::shared_ptr<T>;

    template <std::derived_from<widget> T>
    auto create(string const& name, flags flags, style_attributes const& attribs = {}) -> std::shared_ptr<typename T::style>;

    auto get(string const& name, flags flags, widget_attributes const& attribs) const -> style_base*;

    void clear();

private:
    std::vector<std::tuple<string, flags, style_attributes, std::shared_ptr<style_base>>> _styles;
};

}

#include "Style.inl"
