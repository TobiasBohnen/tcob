// Copyright (c) 2023 Tobias Bohnen
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

auto style_collection::get(string const& name, flags flags, widget_attributes const& attribs) const -> std::shared_ptr<style_base>
{
    std::shared_ptr<style_base> bestCandidate;
    i32                         bestScore {std::numeric_limits<i32>::min()};

    for (auto const& [sname, sflags, sattribs, style] : _styles) {
        if (sname == name) {
            // check attributes
            if (sattribs.check(attribs)) {
                // check flags
                i32 const score {sflags.check(flags)};
                if (score == bestScore) {
                    bestCandidate = style;
                } else if (score > bestScore) {
                    bestScore     = score;
                    bestCandidate = style;
                }
            }
        }
    }

    return bestCandidate;
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

        retValue.Width  = Width.calc(rect.Width);
        retValue.Height = Height.calc(rect.Height);

        retValue.X += rect.Width - retValue.Width;
        retValue.Y += (rect.Height - retValue.Height) / 2;

        return retValue - Border.get_thickness();
    }

    auto bar::calc(rect_f const& rect, orientation orien, alignment align) const -> rect_f
    {
        rect_f retValue {rect};

        switch (orien) {
        case orientation::Horizontal:
            retValue.Height = Size.calc(rect.Height);

            switch (align) {
            case alignment::RightOrBottom:
                retValue.Y += rect.Height - retValue.Height;
                break;
            case alignment::CenterOrMiddle:
                retValue.Y += (rect.Height - retValue.Height) / 2.0f;
                break;
            }
            break;
        case orientation::Vertical:
            retValue.Width = Size.calc(rect.Width);

            switch (align) {
            case alignment::RightOrBottom:
                retValue.X += rect.Width - retValue.Width;
                break;
            case alignment::CenterOrMiddle:
                retValue.X += (rect.Width - retValue.Width) / 2.0f;
                break;
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
