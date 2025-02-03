// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/DropDownList.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

static constexpr isize INVALID {-1};

drop_down_list::drop_down_list(init const& wi)
    : widget {wi}
    , SelectedItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID, std::ssize(_items) - 1); }}}
    , HoveredItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID, std::ssize(_items) - 1); }}}
    , _vScrollbar {*this, orientation::Vertical}
{
    SelectedItemIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": SelectedItem changed"); });
    SelectedItemIndex(INVALID);
    HoveredItemIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": HoveredItem changed"); });
    HoveredItemIndex(INVALID);

    Class("drop_down_list");
}

void drop_down_list::add_item(utf8_string const& item)
{
    add_item({.Text = item, .Icon = {}, .UserData = {}});
}

void drop_down_list::add_item(list_item const& item)
{
    _items.push_back(item);
    force_redraw(this->name() + ": item added");
}

void drop_down_list::clear_items()
{
    _items.clear();
    force_redraw(this->name() + ": items cleared");
}

auto drop_down_list::select_item(utf8_string const& item) -> bool
{
    for (isize i {0}; i < std::ssize(_items); ++i) {
        if (_items[i].Text == item) {
            SelectedItemIndex = i;
            return true;
        }
    }

    return false;
}

auto drop_down_list::get_item_at(isize index) const -> list_item const&
{
    return _items.at(static_cast<usize>(index));
}

auto drop_down_list::selected_item() const -> list_item const&
{
    return _items.at(SelectedItemIndex());
}

auto drop_down_list::item_count() const -> isize
{
    return std::ssize(_items);
}

void drop_down_list::on_styles_changed()
{
    widget::on_styles_changed();

    if (auto* style {current_style<drop_down_list::style>()}) {
        _vScrollbar.Style = &style->VScrollBar;
    } else {
        _vScrollbar.Style = nullptr;
    }

    _vScrollbar.start_scroll(0, milliseconds {0});
}

void drop_down_list::prepare_redraw()
{
    widget::prepare_redraw();

    _vScrollbar.Min = 0;
    _vScrollbar.Max = std::max(0.0f, (get_item_height() * std::ssize(_items)) - content_bounds().height());
}

void drop_down_list::on_paint(widget_painter& painter)
{
    if (auto const* style {current_style<drop_down_list::style>()}) {
        _itemRectCache.clear();

        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        // arrow
        auto const& fls {flags()};
        auto*       normalArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {})};
        auto*       hoverArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {.Hover = true})};
        auto*       activeArrow {get_sub_style<nav_arrows_style>(style->NavArrowClass, {.Active = true})};
        assert(normalArrow && hoverArrow && activeArrow);

        element::nav_arrow arrowStyle;
        if (_mouseOverBox) {
            arrowStyle = fls.Active ? activeArrow->NavArrow : hoverArrow->NavArrow;
        } else {
            arrowStyle = normalArrow->NavArrow;
        }
        painter.draw_nav_arrow(arrowStyle, rect);

        // text
        if (style->Text.Font && SelectedItemIndex >= 0) {
            f32 const arrowWidth {arrowStyle.Size.Width.calc(rect.width())};
            painter.draw_text(style->Text, {rect.left(), rect.top(), rect.width() - arrowWidth, rect.height()}, selected_item().Text);
        }

        if (_isExtended) {
            f32 const itemHeight {style->ItemHeight.calc(rect.height())};

            // list background
            rect_f listRect {Bounds()};
            listRect.Position.Y += listRect.height();
            f32 const listHeight {itemHeight * style->VisibleItemCount};
            listRect.Size.Height = listHeight;
            listRect.Size.Height += style->Margin.Top.calc(listHeight) + style->Margin.Bottom.calc(listHeight);
            listRect.Size.Height += style->Padding.Top.calc(listHeight) + style->Padding.Bottom.calc(listHeight);
            listRect.Size.Height += style->Border.Size.calc(listHeight);

            painter.draw_background_and_border(*style, listRect, false);

            // scrollbar
            _vScrollbar.Visible = std::ssize(_items) > style->VisibleItemCount;
            _vScrollbar.paint(painter, style->VScrollBar, listRect, fls.Active);

            scissor_guard const guard {painter, this};

            auto const paint_item {[&](isize i) {
                auto const&  itemStyle {get_item_style(i)};
                rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
                if (itemRect.bottom() > listRect.top() && itemRect.top() < listRect.bottom()) {
                    painter.draw_item(itemStyle->Item, itemRect, _items[i]);
                    _itemRectCache[i] = itemRect;
                }
            }};

            // content
            for (i32 i {0}; i < std::ssize(_items); ++i) {
                if (i == HoveredItemIndex || i == SelectedItemIndex) { continue; }

                paint_item(i);
            }

            if (SelectedItemIndex >= 0) {
                paint_item(SelectedItemIndex());
            }
            if (HoveredItemIndex >= 0 && SelectedItemIndex != HoveredItemIndex) {
                paint_item(HoveredItemIndex());
            }
        }
    }
}

void drop_down_list::on_mouse_leave()
{
    HoveredItemIndex = INVALID;

    if (_mouseOverBox) {
        _mouseOverBox = false;
        force_redraw(this->name() + ": mouse left");
    }
}

void drop_down_list::on_mouse_hover(input::mouse::motion_event const& ev)
{
    HoveredItemIndex = INVALID;

    bool const wasMouseOverBox {_mouseOverBox};
    _mouseOverBox = false;

    _vScrollbar.mouse_hover(ev.Position);
    if (_vScrollbar.is_mouse_over()) {
        force_redraw(this->name() + ": scrollbar mouse hover");
        ev.Handled = true;
    } else {
        rect_f const boxRect {global_position(), Bounds->Size};
        if (boxRect.contains(ev.Position)) {
            _mouseOverBox = true;
            ev.Handled    = true;
            if (!wasMouseOverBox) { force_redraw(this->name() + ": mouse enter"); }
        } else if (_isExtended) {
            auto const mp {global_to_local(ev.Position)};

            for (auto const& kvp : _itemRectCache) {
                if (!kvp.second.contains(mp)) { continue; }
                HoveredItemIndex = kvp.first;
                ev.Handled       = true;
                return;
            }
        }
    }
}

void drop_down_list::on_mouse_down(input::mouse::button_event const& ev)
{
    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        _vScrollbar.mouse_down(ev.Position);
        if (_mouseOverBox) {
            set_extended(!_isExtended);
        } else if (HoveredItemIndex != INVALID) {
            if (SelectedItemIndex != HoveredItemIndex()) {
                SelectedItemIndex = HoveredItemIndex();
                set_extended(false); // close on select
            }
        }

        ev.Handled = true;
        force_redraw(this->name() + ": mouse down");
    }
}

void drop_down_list::on_mouse_drag(input::mouse::motion_event const& ev)
{
    _vScrollbar.mouse_drag(ev.Position);
    if (_vScrollbar.is_dragging()) {
        force_redraw(this->name() + ": vertical scrollbar dragged");
        ev.Handled = true;
    }
}

void drop_down_list::on_mouse_up(input::mouse::button_event const& ev)
{
    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        _vScrollbar.mouse_up(ev.Position);
        force_redraw(this->name() + ": mouse up");
        ev.Handled = true;
    }
}

void drop_down_list::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (!_vScrollbar.Visible) { return; }

    HoveredItemIndex = INVALID;

    if (auto const* style {current_style<drop_down_list::style>()}) {
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
            _vScrollbar.start_scroll(_vScrollbar.target_value() + diff, delay);
        }
        ev.Handled = true;
    }
}

void drop_down_list::on_focus_lost()
{
    set_extended(false);
}

void drop_down_list::on_update(milliseconds deltaTime)
{
    _vScrollbar.update(deltaTime);
}

void drop_down_list::offset_content(rect_f& bounds, bool isHitTest) const
{
    rect_f    listRect {bounds};
    f32 const boxHeight {bounds.height()};

    widget::offset_content(bounds, isHitTest);

    if (!_isExtended) { return; }
    if (auto const* style {current_style<drop_down_list::style>()}) {
        if (!isHitTest) { bounds.Position.Y += listRect.Size.Height; }
        f32 refListHeight {listRect.Size.Height};
        refListHeight -= style->Margin.Top.calc(listRect.Size.Height) + style->Margin.Bottom.calc(listRect.Size.Height);
        refListHeight -= style->Padding.Top.calc(listRect.Size.Height) + style->Padding.Bottom.calc(listRect.Size.Height);
        refListHeight -= style->Border.Size.calc(listRect.Size.Height);
        f32 const itemHeight {style->ItemHeight.calc(refListHeight)};
        f32 const listHeight {itemHeight * style->VisibleItemCount};
        bounds.Size.Height = listHeight;

        if (isHitTest) {
            bounds.Size.Height += style->Margin.Top.calc(listHeight) + style->Margin.Bottom.calc(listHeight);
            bounds.Size.Height += style->Border.Size.calc(listHeight);
            bounds.Size.Height += boxHeight;
        }
    }
}

auto drop_down_list::get_item_rect(isize index, f32 itemHeight, rect_f const& listRect) const -> rect_f
{
    rect_f retValue {listRect};
    retValue.Size.Height = itemHeight;
    retValue.Position.Y  = listRect.top() + (retValue.height() * index) - _vScrollbar.current_value();
    return retValue;
}

auto drop_down_list::get_item_style(isize index) const -> item_style*
{
    auto const* style {current_style<drop_down_list::style>()};
    return index == SelectedItemIndex ? get_sub_style<item_style>(style->ItemClass, {.Active = true})
        : index == HoveredItemIndex   ? get_sub_style<item_style>(style->ItemClass, {.Hover = true})
                                      : get_sub_style<item_style>(style->ItemClass, {});
}

auto drop_down_list::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};
    if (SelectedItemIndex >= 0 && SelectedItemIndex < std::ssize(_items)) {
        retValue["selected"] = selected_item().Text;
    }
    return retValue;
}

auto drop_down_list::get_item_height() const -> f32
{
    if (auto const* style {current_style<drop_down_list::style>()}) {
        rect_f bounds {Bounds()};
        widget::offset_content(bounds, false);
        return style->ItemHeight.calc(bounds.height());
    }

    return 0;
}

void drop_down_list::set_extended(bool v)
{
    if (_isExtended != v) {
        _isExtended = v;
        force_redraw(this->name() + ": extended change");
    }
}

} // namespace ui
