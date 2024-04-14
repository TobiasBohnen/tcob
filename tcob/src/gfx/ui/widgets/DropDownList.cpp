// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/DropDownList.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

drop_down_list::drop_down_list(init const& wi)
    : widget {wi}
    , SelectedItemIndex {{[&](isize val) -> isize { return std::clamp<isize>(val, -1, std::ssize(_items) - 1); }}}
    , HoveredItemIndex {{[&](isize val) -> isize { return std::clamp<isize>(val, -1, std::ssize(_items) - 1); }}}
    , _vScrollbar {*this, orientation::Vertical}
{
    SelectedItemIndex.Changed.connect([&](auto const&) { force_redraw(get_name() + ": SelectedItem changed"); });
    SelectedItemIndex(-1);
    HoveredItemIndex.Changed.connect([&](auto const&) { force_redraw(get_name() + ": HoveredItem changed"); });
    HoveredItemIndex(-1);
    VisibleItems.Changed.connect([&](auto const&) { force_redraw(get_name() + ": VisibleItems changed"); });
    VisibleItems(5);

    Class("drop_down_list");
}

void drop_down_list::add_item(utf8_string const& item)
{
    _items.push_back(item);
    force_redraw(get_name() + ": item added");
}

void drop_down_list::clear_items()
{
    _items.clear();
    force_redraw(get_name() + ": items cleared");
}

auto drop_down_list::select_item(utf8_string const& item) -> bool
{
    for (isize i {0}; i < std::ssize(_items); ++i) {
        if (_items[i] == item) {
            SelectedItemIndex = i;
            return true;
        }
    }

    return false;
}

auto drop_down_list::get_item_at(isize index) const -> utf8_string const&
{
    return _items.at(static_cast<usize>(index));
}

auto drop_down_list::get_selected_item() const -> utf8_string const&
{
    return _items.at(SelectedItemIndex);
}

auto drop_down_list::get_item_count() const -> isize
{
    return std::ssize(_items);
}

void drop_down_list::on_styles_changed()
{
    widget::on_styles_changed();
    _vScrollbar.set_target_value(0, milliseconds {0});
}

void drop_down_list::paint_item(widget_painter& painter, rect_f& listRect, f32 itemHeight, isize i)
{
    auto const&  itemStyle {get_item_style(i)};
    rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
    if (itemRect.bottom() > 0 && itemRect.top() < listRect.bottom()) {
        painter.draw_item(itemStyle->Item, itemRect, _items[i]);
    }
}

void drop_down_list::on_paint(widget_painter& painter)
{
    if (auto const* style {get_style<drop_down_list::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        // arrow
        auto const& flags {get_flags()};
        auto*       normalArrow {get_sub_style<nav_arrows_style>(style->ArrowClass, {})};
        auto*       hoverArrow {get_sub_style<nav_arrows_style>(style->ArrowClass, {.Hover = true})};
        auto*       activeArrow {get_sub_style<nav_arrows_style>(style->ArrowClass, {.Active = true})};
        assert(normalArrow && hoverArrow && activeArrow);

        element::nav_arrow arrowStyle;
        if (_mouseOverBox) {
            arrowStyle = flags.Active ? activeArrow->NavArrow : hoverArrow->NavArrow;
        } else {
            arrowStyle = normalArrow->NavArrow;
        }
        painter.draw_nav_arrow(arrowStyle, rect);

        // text
        if (style->Text.Font && SelectedItemIndex >= 0) {
            f32 const arrowWidth {arrowStyle.Width.calc(rect.Width)};
            painter.draw_text(style->Text, {rect.X, rect.Y, rect.Width - arrowWidth, rect.Height}, get_selected_item());
        }

        if (_isExtended) {
            f32 const itemHeight {style->ItemHeight.calc(rect.Height)};

            // list background
            rect_f listRect {Bounds()};
            listRect.Y += listRect.Height;
            f32 const height {itemHeight * VisibleItems};
            listRect.Height = height;
            listRect.Height += style->Margin.Top.calc(height) + style->Margin.Bottom.calc(height);
            listRect.Height += style->Padding.Top.calc(height) + style->Padding.Bottom.calc(height);
            listRect.Height += style->Border.Size.calc(height);

            painter.draw_background_and_border(*style, listRect, false);

            // scrollbar
            _vScrollbar.paint(painter, style->VScrollBar, listRect, get_flags().Active);

            scissor_guard const guard {painter, this};

            // content
            for (i32 i {0}; i < std::ssize(_items); ++i) {
                if (i == HoveredItemIndex || i == SelectedItemIndex) { continue; }

                paint_item(painter, listRect, itemHeight, i);
            }

            if (SelectedItemIndex >= 0) {
                paint_item(painter, listRect, itemHeight, SelectedItemIndex);
            }
            if (HoveredItemIndex >= 0 && SelectedItemIndex != HoveredItemIndex) {
                paint_item(painter, listRect, itemHeight, HoveredItemIndex);
            }
        }
    }
}

void drop_down_list::on_mouse_leave()
{
    widget::on_mouse_leave();

    HoveredItemIndex = -1;
    if (_mouseOverBox) {
        _mouseOverBox = false;
        force_redraw(get_name() + ": mouse left");
    }
}

void drop_down_list::on_mouse_hover(input::mouse::motion_event& ev)
{
    HoveredItemIndex = -1;

    widget::on_mouse_hover(ev);

    if (auto const* style {get_style<drop_down_list::style>()}) {
        rect_f listRect {get_global_content_bounds()};
        if (_vScrollbar.inject_mouse_hover(ev.Position)) {
            _mouseOverBox = false;
            force_redraw(get_name() + ": scrollbar mouse hover");
            ev.Handled = true;
        } else {
            if (_isExtended) {
                // over list
                f32 const itemHeight {get_item_height()};
                if (_vScrollbar.Visible) {
                    listRect.Width -= style->VScrollBar.Bar.Size.calc(listRect.Width);
                }
                for (i32 i {0}; i < std::ssize(_items); ++i) {
                    rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
                    if (itemRect.contains(ev.Position)) {
                        HoveredItemIndex = i;
                        _mouseOverBox    = false;
                        ev.Handled       = true;
                        return;
                    }
                }
            }

            if (!_mouseOverBox) {
                _mouseOverBox = true;
                ev.Handled    = true;
                force_redraw(get_name() + ": mouse enter");
            }
        }
    }
}

void drop_down_list::on_mouse_down(input::mouse::button_event& ev)
{
    widget::on_mouse_down(ev);

    if (ev.Button == get_form()->Controls->PrimaryMouseButton) {
        _vScrollbar.inject_mouse_down(ev.Position);
        if (_mouseOverBox) {
            set_extended(!_isExtended);
        } else if (HoveredItemIndex != -1) {
            if (SelectedItemIndex != HoveredItemIndex()) {
                SelectedItemIndex = HoveredItemIndex();
            } // else SelectedItemClicked event?
        }

        ev.Handled = true;
        force_redraw(get_name() + ": mouse down");
    }
}

void drop_down_list::on_mouse_drag(input::mouse::motion_event& ev)
{
    widget::on_mouse_drag(ev);

    if (_vScrollbar.inject_mouse_drag(ev.Position)) {
        force_redraw(get_name() + ": vertical scrollbar dragged");
        ev.Handled = true;
    }
}

void drop_down_list::on_mouse_up(input::mouse::button_event& ev)
{
    widget::on_mouse_up(ev);

    if (ev.Button == get_form()->Controls->PrimaryMouseButton) {
        _vScrollbar.inject_mouse_up(ev.Position);
        force_redraw(get_name() + ": mouse up");
        ev.Handled = true;
    }
}

void drop_down_list::on_mouse_wheel(input::mouse::wheel_event& ev)
{
    if (!_vScrollbar.Visible) { return; }

    if (auto const* style {get_style<drop_down_list::style>()}) {
        orientation  orien {};
        bool         invert {false};
        milliseconds delay {};

        if (ev.Scroll.Y != 0) {
            orien  = orientation::Vertical;
            invert = ev.Scroll.Y > 0;
            delay  = style->VScrollBar.Bar.Delay;
        }

        f32 const diff {get_item_height() / std::ssize(_items) * (invert ? -5 : 5)};
        if (orien == orientation::Vertical) {
            _vScrollbar.set_target_value(_vScrollbar.get_target_value() + diff, delay);
        }
        ev.Handled = true;
    }
}

void drop_down_list::on_double_click()
{
    widget::on_double_click();

    if (SelectedItemIndex == HoveredItemIndex) {
        get_form()->focus_widget(nullptr);
    }
}

void drop_down_list::on_focus_lost()
{
    widget::on_focus_lost();

    set_extended(false);
}

void drop_down_list::on_update(milliseconds deltaTime)
{
    _vScrollbar.update(deltaTime);
}

void drop_down_list::offset_content(rect_f& bounds, bool isHitTest) const
{
    f32 const height {bounds.Height};

    widget::offset_content(bounds, isHitTest);

    if (auto const* style {get_style<drop_down_list::style>()}) {
        if (_isExtended) {
            if (!isHitTest) {
                bounds.Y += height;
            }

            f32 const itemHeight {style->ItemHeight.calc(bounds.Height)};
            bounds.Height = itemHeight * VisibleItems;

            if (isHitTest) {
                bounds.Height += height;
            }
        }
    }
}

auto drop_down_list::get_item_rect(isize index, f32 itemHeight, rect_f const& listRect) const -> rect_f
{
    rect_f retValue {listRect};
    retValue.Height = itemHeight;
    retValue.Y      = listRect.Y + (retValue.Height * index) - _vScrollbar.get_current_value();
    return retValue;
}

auto drop_down_list::get_item_style(isize index) const -> item_style*
{
    auto const* style {get_style<drop_down_list::style>()};
    return index == SelectedItemIndex ? get_sub_style<item_style>(style->ItemClass, {.Active = true})
        : index == HoveredItemIndex   ? get_sub_style<item_style>(style->ItemClass, {.Hover = true})
                                      : get_sub_style<item_style>(style->ItemClass, {});
}

auto drop_down_list::get_attributes() const -> widget_attributes
{
    auto retValue {widget::get_attributes()};
    if (SelectedItemIndex >= 0 && SelectedItemIndex < std::ssize(_items)) {
        retValue["selected"] = get_selected_item();
    }
    return retValue;
}

auto drop_down_list::get_item_height() const -> f32
{
    if (auto const* style {get_style<drop_down_list::style>()}) {
        rect_f bounds {Bounds()};
        widget::offset_content(bounds, false);
        return style->ItemHeight.calc(bounds.Height);
    }

    return 0;
}

auto drop_down_list::requires_scroll(orientation orien, rect_f const& /* rect */) const -> bool
{
    if (orien == orientation::Horizontal) {
        return false;
    }

    return std::ssize(_items) > VisibleItems();
}

auto drop_down_list::get_scroll_min_value(orientation /* orien */) const -> f32
{
    return 0;
}

auto drop_down_list::get_scroll_max_value(orientation orien) const -> f32
{
    if (orien == orientation::Horizontal) {
        return 0;
    }

    if (auto const* style {get_style<drop_down_list::style>()}) {
        f32 const itemHeight {get_item_height()};
        return std::max(0.0f, (itemHeight * std::ssize(_items)) - get_content_bounds().Height);
    }

    return 0;
}

auto drop_down_list::get_scroll_style(orientation orien) const -> element::scrollbar*
{
    if (auto* style {get_style<drop_down_list::style>()}) {
        if (orien == orientation::Vertical) {
            return &style->VScrollBar;
        }
    }

    return nullptr;
}

void drop_down_list::set_extended(bool v)
{
    if (_isExtended != v) {
        _isExtended = v;
        force_redraw(get_name() + ": extended change");
    }
}

} // namespace ui
