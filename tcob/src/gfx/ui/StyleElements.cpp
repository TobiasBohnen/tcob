// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/StyleElements.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/ui/Paint.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

auto thumb_element::calc(rect_f const& rect, context const& ctx) const -> rect_f
{
    rect_f retValue {rect};

    switch (ctx.Orientation) {
    case orientation::Horizontal:
        retValue.Size.Width  = LongSide.calc(rect.width());
        retValue.Size.Height = ShortSide.calc(rect.height());
        retValue.Position.X += (rect.width() - retValue.width()) * ctx.RelativePosition;
        retValue.Position.Y += (rect.height() - retValue.height()) / 2.0f;
        break;
    case orientation::Vertical:
        retValue.Size.Height = LongSide.calc(rect.height());
        retValue.Size.Width  = ShortSide.calc(rect.width());
        retValue.Position.Y += (rect.height() - retValue.height()) * ctx.RelativePosition;
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
        case position::RightOrBottom:  retValue.Position.Y += rect.height() - retValue.height(); break;
        case position::CenterOrMiddle: retValue.Position.Y += (rect.height() - retValue.height()) / 2.0f; break;
        default:                       break;
        }
        break;
    case orientation::Vertical:
        retValue.Size.Width = Size.calc(rect.width());

        switch (align) {
        case position::RightOrBottom:  retValue.Position.X += rect.width() - retValue.width(); break;
        case position::CenterOrMiddle: retValue.Position.X += (rect.width() - retValue.width()) / 2.0f; break;
        default:                       break;
        }
        break;
    }

    return retValue - Border.thickness();
}

auto border_element::thickness() const -> ui::thickness
{
    return {Size / 2};
}

auto text_element::calc_font_size(f32 height) const -> u32
{
    return static_cast<u32>(std::floor(Size.calc(1.0f, height)));
}

////////////////////////////////////////////////////////////

void thumb_element::lerp(thumb_element const& from, thumb_element const& to, f64 step)
{
    paint_lerp(Background, from.Background, to.Background, step);

    LongSide  = length::Lerp(from.LongSide, to.LongSide, step);
    ShortSide = length::Lerp(from.ShortSide, to.ShortSide, step);
    Border.lerp(from.Border, to.Border, step);
}

void nav_arrow_element::lerp(nav_arrow_element const& from, nav_arrow_element const& to, f64 step)
{
    paint_lerp(UpBackground, from.UpBackground, to.UpBackground, step);
    paint_lerp(DownBackground, from.DownBackground, to.DownBackground, step);
    paint_lerp(Foreground, from.Foreground, to.Foreground, step);

    Size = dimensions::Lerp(from.Size, to.Size, step);
    Border.lerp(from.Border, to.Border, step);
    Padding = thickness::Lerp(from.Padding, to.Padding, step);
}

void bar_element::lerp(bar_element const& from, bar_element const& to, f64 step)
{
    paint_lerp(LowerBackground, from.LowerBackground, to.LowerBackground, step);
    paint_lerp(HigherBackground, from.HigherBackground, to.HigherBackground, step);

    Size = length::Lerp(from.Size, to.Size, step);
    Border.lerp(from.Border, to.Border, step);
}

void border_element::lerp(border_element const& from, border_element const& to, f64 step)
{
    paint_lerp(Background, from.Background, to.Background, step);

    Radius = length::Lerp(from.Radius, to.Radius, step);
    Size   = length::Lerp(from.Size, to.Size, step);

    usize const         ldashSize {from.Dash.size()};
    usize const         rdashSize {to.Dash.size()};
    std::vector<length> targetDash;
    targetDash.resize(std::max(ldashSize, rdashSize));
    for (usize i {0}; i < targetDash.size(); ++i) {
        auto const ldash {i < ldashSize ? from.Dash[i] : length {0, i < rdashSize ? to.Dash[i].Type : length::type::Absolute}};
        auto const rdash {i < rdashSize ? to.Dash[i] : length {0, i < ldashSize ? from.Dash[i].Type : length::type::Absolute}};
        targetDash[i] = length::Lerp(ldash, rdash, step);
    }
    Dash = targetDash;

    DashOffset = helper::lerp(from.DashOffset, to.DashOffset, step);
}

void text_element::lerp(text_element const& from, text_element const& to, f64 step)
{
    Color = color::Lerp(from.Color, to.Color, step);
    Size  = length::Lerp(from.Size, to.Size, step);
    Decoration.lerp(from.Decoration, to.Decoration, step);
    Shadow.lerp(from.Shadow, to.Shadow, step);
}

void caret_element::lerp(caret_element const& from, caret_element const& to, f64 step)
{
    Color = color::Lerp(from.Color, to.Color, step);
    Width = length::Lerp(from.Width, to.Width, step);
}

void shadow_element::lerp(shadow_element const& from, shadow_element const& to, f64 step)
{
    Color   = color::Lerp(from.Color, to.Color, step);
    OffsetX = length::Lerp(from.OffsetX, to.OffsetX, step);
    OffsetY = length::Lerp(from.OffsetY, to.OffsetY, step);
}

void deco_element::lerp(deco_element const& from, deco_element const& to, f64 step)
{
    Color = color::Lerp(from.Color, to.Color, step);
    Size  = length::Lerp(from.Size, to.Size, step);
}

void tick_element::lerp(tick_element const& from, tick_element const& to, f64 step)
{
    paint_lerp(Foreground, from.Foreground, to.Foreground, step);

    Size = length::Lerp(from.Size, to.Size, step);
}

void scrollbar_element::lerp(scrollbar_element const& from, scrollbar_element const& to, f64 step)
{
    Bar.lerp(from.Bar, to.Bar, step);
}

void item_element::lerp(item_element const& from, item_element const& to, f64 step)
{
    Text.lerp(from.Text, to.Text, step);

    paint_lerp(Background, from.Background, to.Background, step);

    Border.lerp(from.Border, to.Border, step);
    Padding = thickness::Lerp(from.Padding, to.Padding, step);
}

////////////////////////////////////////////////////////////

}
