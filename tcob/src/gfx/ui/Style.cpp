// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Style.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

style_attributes::style_attributes(std::initializer_list<std::pair<string, attributes> const> values)
{
    for (auto const& val : values) {
        _values[val.first].insert(val.second);
    }
}

auto style_attributes::check(widget_attributes const& widgetAttribs) const -> bool
{
    if (_values.empty()) {
        return true;
    }

    return std::all_of(_values.begin(), _values.end(), [&](auto const& kv) {
        return widgetAttribs.contains(kv.first) && kv.second.contains(widgetAttribs.at(kv.first));
    });
}

////////////////////////////////////////////////////////////

auto style_flags::check(widget_flags other) const -> i32
{
    i32                                      retValue {0};
    std::array<std::optional<bool>, 5> const flagSet {Focus, Active, Hover, Checked, Disabled};
    std::array<bool, 5> const                otherFlagSet {other.Focus, other.Active, other.Hover, other.Checked, other.Disabled};

    for (usize i {0}; i < flagSet.size(); ++i) {
        if (flagSet[i]) {
            if (*flagSet[i] != otherFlagSet[i]) {
                return std::numeric_limits<i32>::min();
            }
            ++retValue;
        }
    }
    return retValue;
}

////////////////////////////////////////////////////////////

auto style_collection::get(string const& name, widget_flags flags, widget_attributes const& attribs) const -> style_base*
{
    style_base* bestCandidate {nullptr};
    i32         bestScore {std::numeric_limits<i32>::min()};

    for (auto const& [styleName, styleFlags, styleAttribs, style] : _styles) {
        if (styleName != name) { continue; }

        // check attributes
        if (!styleAttribs.check(attribs)) { continue; }

        // check flags
        i32 const score {styleFlags.check(flags)};
        if (score == bestScore) {
            bestCandidate = style.get();
        } else if (score > bestScore) {
            bestScore     = score;
            bestCandidate = style.get();
        }
    }

    return bestCandidate;
}

void style_collection::clear()
{
    _styles.clear();
}

namespace element {
    auto thumb::calc(rect_f const& rect, context const& ctx) const -> rect_f
    {
        rect_f retValue {rect};

        switch (ctx.Orientation) {
        case orientation::Horizontal:
            retValue.Width  = LongSide.calc(rect.Width);
            retValue.Height = ShortSide.calc(rect.Height);
            retValue.X += (rect.Width - retValue.Width) * (ctx.Inverted ? (1.0f - ctx.Fraction) : ctx.Fraction);
            retValue.Y += (rect.Height - retValue.Height) / 2.0f;
            break;
        case orientation::Vertical:
            retValue.Height = LongSide.calc(rect.Height);
            retValue.Width  = ShortSide.calc(rect.Width);
            retValue.Y += (rect.Height - retValue.Height) * (!ctx.Inverted ? (1.0f - ctx.Fraction) : ctx.Fraction);
            retValue.X += (rect.Width - retValue.Width) / 2.0f;
        }

        return retValue - Border.get_thickness();
    }

    auto nav_arrow::calc(rect_f const& rect) const -> rect_f
    {
        rect_f retValue {rect};

        retValue.Width  = Size.Width.calc(rect.Width);
        retValue.Height = Size.Height.calc(rect.Height);

        retValue.X += rect.Width - retValue.Width;
        retValue.Y += (rect.Height - retValue.Height) / 2;

        return retValue - Border.get_thickness() - Padding;
    }

    auto bar::calc(rect_f const& rect, orientation orien, position align) const -> rect_f
    {
        rect_f retValue {rect};

        switch (orien) {
        case orientation::Horizontal:
            retValue.Height = Size.calc(rect.Height);

            switch (align) {
            case position::RightOrBottom:
                retValue.Y += rect.Height - retValue.Height;
                break;
            case position::CenterOrMiddle:
                retValue.Y += (rect.Height - retValue.Height) / 2.0f;
                break;
            default: break;
            }
            break;
        case orientation::Vertical:
            retValue.Width = Size.calc(rect.Width);

            switch (align) {
            case position::RightOrBottom:
                retValue.X += rect.Width - retValue.Width;
                break;
            case position::CenterOrMiddle:
                retValue.X += (rect.Width - retValue.Width) / 2.0f;
                break;
            default: break;
            }
            break;
        }

        return retValue - Border.get_thickness();
    }

    auto border::get_thickness() const -> thickness
    {
        return {Size / 2};
    }

    auto text::calc_font_size(rect_f const& rect) const -> u32
    {
        return static_cast<u32>(std::ceil(Size.calc(1.0f, rect.Height)));
    }
}

void style::offset_content(rect_f& bounds, bool isHitTest) const
{
    bounds -= Margin;
    if (!isHitTest) {
        bounds -= Padding;
    }
}

void background_style::offset_content(rect_f& bounds, bool isHitTest) const
{
    style::offset_content(bounds, isHitTest);
    if (!isHitTest) {
        bounds -= Border.get_thickness();
    }
}

} // namespace ui
