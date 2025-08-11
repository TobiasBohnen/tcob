// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/StyleElements.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
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

void thumb_element::lerp(thumb_element const& left, thumb_element const& right, f64 step)
{
    paint_lerp(Background, left.Background, right.Background, step);

    LongSide  = length::Lerp(left.LongSide, right.LongSide, step);
    ShortSide = length::Lerp(left.ShortSide, right.ShortSide, step);
    Border.lerp(left.Border, right.Border, step);
}

void nav_arrow_element::lerp(nav_arrow_element const& left, nav_arrow_element const& right, f64 step)
{
    paint_lerp(UpBackground, left.UpBackground, right.UpBackground, step);
    paint_lerp(DownBackground, left.DownBackground, right.DownBackground, step);
    paint_lerp(Foreground, left.Foreground, right.Foreground, step);

    Size = dimensions::Lerp(left.Size, right.Size, step);
    Border.lerp(left.Border, right.Border, step);
    Padding = thickness::Lerp(left.Padding, right.Padding, step);
}

void bar_element::lerp(bar_element const& left, bar_element const& right, f64 step)
{
    paint_lerp(LowerBackground, left.LowerBackground, right.LowerBackground, step);
    paint_lerp(HigherBackground, left.HigherBackground, right.HigherBackground, step);

    Size = length::Lerp(left.Size, right.Size, step);
    Border.lerp(left.Border, right.Border, step);
}

void border_element::lerp(border_element const& left, border_element const& right, f64 step)
{
    paint_lerp(Background, left.Background, right.Background, step);

    Radius = length::Lerp(left.Radius, right.Radius, step);
    Size   = length::Lerp(left.Size, right.Size, step);

    usize const         ldashSize {left.Dash.size()};
    usize const         rdashSize {right.Dash.size()};
    std::vector<length> targetDash;
    targetDash.resize(std::max(ldashSize, rdashSize));
    for (usize i {0}; i < targetDash.size(); ++i) {
        auto const ldash {i < ldashSize ? left.Dash[i] : length {0, i < rdashSize ? right.Dash[i].Type : length::type::Absolute}};
        auto const rdash {i < rdashSize ? right.Dash[i] : length {0, i < ldashSize ? left.Dash[i].Type : length::type::Absolute}};
        targetDash[i] = length::Lerp(ldash, rdash, step);
    }
    Dash = targetDash;

    DashOffset = static_cast<f32>(left.DashOffset + ((right.DashOffset - left.DashOffset) * step));
}

void text_element::lerp(text_element const& left, text_element const& right, f64 step)
{
    Color = color::Lerp(left.Color, right.Color, step);
    Size  = length::Lerp(left.Size, right.Size, step);
    Decoration.lerp(left.Decoration, right.Decoration, step);
    Shadow.lerp(left.Shadow, right.Shadow, step);
}

void caret_element::lerp(caret_element const& left, caret_element const& right, f64 step)
{
    Color = color::Lerp(left.Color, right.Color, step);
    Width = length::Lerp(left.Width, right.Width, step);
}

void shadow_element::lerp(shadow_element const& left, shadow_element const& right, f64 step)
{
    Color   = color::Lerp(left.Color, right.Color, step);
    OffsetX = length::Lerp(left.OffsetX, right.OffsetX, step);
    OffsetY = length::Lerp(left.OffsetY, right.OffsetY, step);
}

void deco_element::lerp(deco_element const& left, deco_element const& right, f64 step)
{
    Color = color::Lerp(left.Color, right.Color, step);
    Size  = length::Lerp(left.Size, right.Size, step);
}

void tick_element::lerp(tick_element const& left, tick_element const& right, f64 step)
{
    paint_lerp(Foreground, left.Foreground, right.Foreground, step);

    Size = length::Lerp(left.Size, right.Size, step);
}

void scrollbar_element::lerp(scrollbar_element const& left, scrollbar_element const& right, f64 step)
{
    Bar.lerp(left.Bar, right.Bar, step);
}

void item_element::lerp(item_element const& left, item_element const& right, f64 step)
{
    Text.lerp(left.Text, right.Text, step);

    paint_lerp(Background, left.Background, right.Background, step);

    Border.lerp(left.Border, right.Border, step);
    Padding = thickness::Lerp(left.Padding, right.Padding, step);
}

////////////////////////////////////////////////////////////

} // namespace ui
