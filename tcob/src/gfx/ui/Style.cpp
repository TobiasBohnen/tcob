// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Style.hpp"

namespace tcob::gfx::ui {
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
            if (*flagSet[i] != otherFlagSet[i]) {
                return std::numeric_limits<i32>::min();
            }
            ++retValue;
        }
    }
    return retValue;
}

////////////////////////////////////////////////////////////

auto style_collection::get(string const& name, widget_flags flags, widget_attributes const& attribs) const -> style*
{
    style* bestCandidate {nullptr};
    i32    bestScore {std::numeric_limits<i32>::min()};

    for (auto const& [styleName, styleFlags, styleAttribs, style] : _styles) {
        if (styleName != name) { continue; }

        // check attributes
        if (!styleAttribs.check(attribs)) { continue; }

        // check flags
        i32 const score {styleFlags.score(flags)};
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

    auto nav_arrow::calc(rect_f const& rect) const -> rect_f
    {
        rect_f retValue {rect};

        retValue.Size.Width  = Size.Width.calc(rect.width());
        retValue.Size.Height = Size.Height.calc(rect.height());

        retValue.Position.X += rect.width() - retValue.width();
        retValue.Position.Y += (rect.height() - retValue.height()) / 2;

        return retValue - Border.thickness() - Padding;
    }

    auto bar::calc(rect_f const& rect, orientation orien, position align) const -> rect_f
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

    auto border::thickness() const -> gfx::ui::thickness
    {
        return {Size / 2};
    }

    auto text::calc_font_size(rect_f const& rect) const -> u32
    {
        return static_cast<u32>(std::ceil(Size.calc(1.0f, rect.height())));
    }
}

void widget_style::offset_content(rect_f& bounds, bool isHitTest) const
{
    bounds -= Margin;
    if (!isHitTest) {
        bounds -= Padding;
    }
}

void background_style::offset_content(rect_f& bounds, bool isHitTest) const
{
    widget_style::offset_content(bounds, isHitTest);
    if (!isHitTest) {
        bounds -= Border.thickness();
    }
}

} // namespace ui
