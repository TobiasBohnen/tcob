// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Style.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <limits>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

void widget_style::Transition(widget_style& target, widget_style const& left, widget_style const& right, f64 step)
{
    target.Padding = thickness::Lerp(left.Padding, right.Padding, step);
    target.Margin  = thickness::Lerp(left.Margin, right.Margin, step);
    if (auto const* lc {std::get_if<color>(&left.Background)}) {
        if (auto const* rc {std::get_if<color>(&right.Background)}) {
            target.Background = color::Lerp(*lc, *rc, step);
        }
    }

    shadow_element::Transition(target.DropShadow, left.DropShadow, right.DropShadow, step);
    border_element::Transition(target.Border, left.Border, right.Border, step);
}

////////////////////////////////////////////////////////////

style_attributes::style_attributes(std::initializer_list<std::pair<string, widget_attribute_types> const> values)
{
    for (auto const& val : values) {
        _values[val.first].insert(val.second);
    }
}

auto style_attributes::check(widget_attributes const& widgetAttribs) const -> bool
{
    if (_values.empty()) { return true; }

    return std::all_of(_values.begin(), _values.end(), [&](auto const& kv) {
        return widgetAttribs.contains(kv.first) && kv.second.contains(widgetAttribs.at(kv.first));
    });
}

////////////////////////////////////////////////////////////

auto style_flags::score(widget_flags other) const -> i32
{
    i32                                      retValue {0};
    std::array<std::optional<bool>, 5> const flagSet {Focus, Active, Hover, Checked, Disabled};
    std::array<bool, 5> const                otherFlagSet {other.Focus, other.Active, other.Hover, other.Checked, other.Disabled};

    for (usize i {0}; i < flagSet.size(); ++i) {
        if (flagSet[i]) {
            if (*flagSet[i] != otherFlagSet[i]) { return std::numeric_limits<i32>::min(); }
            ++retValue;
        }
    }
    return retValue;
}

////////////////////////////////////////////////////////////

auto style_collection::get(widget_style_selectors const& select) const -> style*
{
    auto const range {_styles.equal_range(select.Class)};
    if (range.first == range.second) { return nullptr; } // No match found

    style* bestCandidate {nullptr};
    i32    bestScore {std::numeric_limits<i32>::min()};

    for (auto it {range.first}; it != range.second; ++it) {
        auto const& [styleFlags, styleAttribs, stylePtr] {it->second};

        // check attributes
        if (!styleAttribs.check(select.Attributes)) { continue; }

        // check flags
        i32 const score {styleFlags.score(select.Flags)};
        if (score >= bestScore) {
            bestScore     = score;
            bestCandidate = stylePtr.get();
        }
    }

    return bestCandidate;
}

void style_collection::clear()
{
    _styles.clear();
}

////////////////////////////////////////////////////////////

auto thumb_element::calc(rect_f const& rect, context const& ctx) const -> rect_f
{
    rect_f retValue {rect};

    switch (ctx.Orientation) {
    case orientation::Horizontal:
        retValue.Size.Width  = LongSide.calc(rect.width());
        retValue.Size.Height = ShortSide.calc(rect.height());
        retValue.Position.X += (rect.width() - retValue.width()) * (ctx.Inverted ? (1.0f - ctx.Fraction) : ctx.Fraction);
        retValue.Position.Y += (rect.height() - retValue.height()) / 2.0f;
        break;
    case orientation::Vertical:
        retValue.Size.Height = LongSide.calc(rect.height());
        retValue.Size.Width  = ShortSide.calc(rect.width());
        retValue.Position.Y += (rect.height() - retValue.height()) * (!ctx.Inverted ? (1.0f - ctx.Fraction) : ctx.Fraction);
        retValue.Position.X += (rect.width() - retValue.width()) / 2.0f;
    }

    return retValue - Border.thickness();
}

auto nav_arrow_element::calc(rect_f const& rect) const -> rect_f
{
    rect_f retValue {rect};

    retValue.Size.Width  = Size.Width.calc(rect.width());
    retValue.Size.Height = Size.Height.calc(rect.height());

    retValue.Position.X += rect.width() - retValue.width();
    retValue.Position.Y += (rect.height() - retValue.height()) / 2;

    return retValue - Border.thickness() - Padding;
}

auto bar_element::calc(rect_f const& rect, orientation orien, position align) const -> rect_f
{
    rect_f retValue {rect};

    switch (orien) {
    case orientation::Horizontal:
        retValue.Size.Height = Size.calc(rect.height());

        switch (align) {
        case position::RightOrBottom:
            retValue.Position.Y += rect.height() - retValue.height();
            break;
        case position::CenterOrMiddle:
            retValue.Position.Y += (rect.height() - retValue.height()) / 2.0f;
            break;
        default: break;
        }
        break;
    case orientation::Vertical:
        retValue.Size.Width = Size.calc(rect.width());

        switch (align) {
        case position::RightOrBottom:
            retValue.Position.X += rect.width() - retValue.width();
            break;
        case position::CenterOrMiddle:
            retValue.Position.X += (rect.width() - retValue.width()) / 2.0f;
            break;
        default: break;
        }
        break;
    }

    return retValue - Border.thickness();
}

auto border_element::thickness() const -> ui::thickness
{
    return {Size / 2};
}

auto text_element::calc_font_size(rect_f const& rect) const -> u32
{
    return static_cast<u32>(std::ceil(Size.calc(1.0f, rect.height())));
}

////////////////////////////////////////////////////////////

void thumb_style::Transition(thumb_style& target, thumb_style const& left, thumb_style const& right, f64 step)
{
    thumb_element::Transition(target.Thumb, left.Thumb, right.Thumb, step);
}

void nav_arrows_style::Transition(nav_arrows_style& target, nav_arrows_style const& left, nav_arrows_style const& right, f64 step)
{
    nav_arrow_element::Transition(target.NavArrow, left.NavArrow, right.NavArrow, step);
}

void item_style::Transition(item_style& target, item_style const& left, item_style const& right, f64 step)
{
    item_element::Transition(target.Item, left.Item, right.Item, step);
}

void thumb_element::Transition(thumb_element& target, thumb_element const& left, thumb_element const& right, f64 step)
{
    if (auto const* lc {std::get_if<color>(&left.Background)}) {
        if (auto const* rc {std::get_if<color>(&right.Background)}) {
            target.Background = color::Lerp(*lc, *rc, step);
        }
    }

    target.LongSide  = length::Lerp(left.LongSide, right.LongSide, step);
    target.ShortSide = length::Lerp(left.ShortSide, right.ShortSide, step);
    border_element::Transition(target.Border, left.Border, right.Border, step);
}

void nav_arrow_element::Transition(nav_arrow_element& target, nav_arrow_element const& left, nav_arrow_element const& right, f64 step)
{
    if (auto const* lc {std::get_if<color>(&left.UpBackground)}) {
        if (auto const* rc {std::get_if<color>(&right.UpBackground)}) {
            target.UpBackground = color::Lerp(*lc, *rc, step);
        }
    }
    if (auto const* lc {std::get_if<color>(&left.DownBackground)}) {
        if (auto const* rc {std::get_if<color>(&right.DownBackground)}) {
            target.DownBackground = color::Lerp(*lc, *rc, step);
        }
    }
    if (auto const* lc {std::get_if<color>(&left.Foreground)}) {
        if (auto const* rc {std::get_if<color>(&right.Foreground)}) {
            target.Foreground = color::Lerp(*lc, *rc, step);
        }
    }

    target.Size = dimensions::Lerp(left.Size, right.Size, step);
    border_element::Transition(target.Border, left.Border, right.Border, step);
    target.Padding = thickness::Lerp(left.Padding, right.Padding, step);
}

void bar_element::Transition(bar_element& target, bar_element const& left, bar_element const& right, f64 step)
{
    if (auto const* lc {std::get_if<color>(&left.LowerBackground)}) {
        if (auto const* rc {std::get_if<color>(&right.LowerBackground)}) {
            target.LowerBackground = color::Lerp(*lc, *rc, step);
        }
    }
    if (auto const* lc {std::get_if<color>(&left.HigherBackground)}) {
        if (auto const* rc {std::get_if<color>(&right.HigherBackground)}) {
            target.HigherBackground = color::Lerp(*lc, *rc, step);
        }
    }

    target.Size = length::Lerp(left.Size, right.Size, step);
    border_element::Transition(target.Border, left.Border, right.Border, step);
}

void border_element::Transition(border_element& target, border_element const& left, border_element const& right, f64 step)
{
    if (auto const* lc {std::get_if<color>(&left.Background)}) {
        if (auto const* rc {std::get_if<color>(&right.Background)}) {
            target.Background = color::Lerp(*lc, *rc, step);
        }
    }

    target.Radius = length::Lerp(left.Radius, right.Radius, step);
    target.Size   = length::Lerp(left.Size, right.Size, step);

    usize const         ldashSize {left.Dash.size()};
    usize const         rdashSize {right.Dash.size()};
    std::vector<length> targetDash;
    targetDash.resize(std::max(ldashSize, rdashSize));
    for (usize i {0}; i < targetDash.size(); ++i) {
        auto const ldash {i < ldashSize ? left.Dash[i] : length {0, i < rdashSize ? right.Dash[i].Type : length::type::Absolute}};
        auto const rdash {i < rdashSize ? right.Dash[i] : length {0, i < ldashSize ? left.Dash[i].Type : length::type::Absolute}};
        targetDash[i] = length::Lerp(ldash, rdash, step);
    }
    target.Dash = targetDash;

    target.DashOffset = static_cast<f32>(left.DashOffset + (right.DashOffset - left.DashOffset) * step);
}

void text_element::Transition(text_element& target, text_element const& left, text_element const& right, f64 step)
{
    target.Color = color::Lerp(left.Color, right.Color, step);
    target.Size  = length::Lerp(left.Size, right.Size, step);
    deco_element::Transition(target.Decoration, left.Decoration, right.Decoration, step);
    shadow_element::Transition(target.Shadow, left.Shadow, right.Shadow, step);
}

void caret_element::Transition(caret_element& target, caret_element const& left, caret_element const& right, f64 step)
{
    target.Color = color::Lerp(left.Color, right.Color, step);
    target.Width = length::Lerp(left.Width, right.Width, step);
}

void shadow_element::Transition(shadow_element& target, shadow_element const& left, shadow_element const& right, f64 step)
{
    target.Color   = color::Lerp(left.Color, right.Color, step);
    target.OffsetX = length::Lerp(left.OffsetX, right.OffsetX, step);
    target.OffsetY = length::Lerp(left.OffsetY, right.OffsetY, step);
}

void deco_element::Transition(deco_element& target, deco_element const& left, deco_element const& right, f64 step)
{
    target.Color = color::Lerp(left.Color, right.Color, step);
    target.Size  = length::Lerp(left.Size, right.Size, step);
}

void tick_element::Transition(tick_element& target, tick_element const& left, tick_element const& right, f64 step)
{
    if (auto const* lc {std::get_if<color>(&left.Foreground)}) {
        if (auto const* rc {std::get_if<color>(&right.Foreground)}) {
            target.Foreground = color::Lerp(*lc, *rc, step);
        }
    }

    target.Size = length::Lerp(left.Size, right.Size, step);
}

void scrollbar_element::Transition(scrollbar_element& target, scrollbar_element const& left, scrollbar_element const& right, f64 step)
{
    bar_element::Transition(target.Bar, left.Bar, right.Bar, step);
}

void item_element::Transition(item_element& target, item_element const& left, item_element const& right, f64 step)
{
    text_element::Transition(target.Text, left.Text, right.Text, step);

    if (auto const* lc {std::get_if<color>(&left.Background)}) {
        if (auto const* rc {std::get_if<color>(&right.Background)}) {
            target.Background = color::Lerp(*lc, *rc, step);
        }
    }

    border_element::Transition(target.Border, left.Border, right.Border, step);
    target.Padding = thickness::Lerp(left.Padding, right.Padding, step);
}

////////////////////////////////////////////////////////////

} // namespace ui
