// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/DropDownList.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void drop_down_list::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    text_element::Transition(target.Text, left.Text, right.Text, step);
    target.ItemHeight = length::Lerp(left.ItemHeight, right.ItemHeight, step);
    scrollbar_element::Transition(target.VScrollBar, left.VScrollBar, right.VScrollBar, step);
}

drop_down_list::drop_down_list(init const& wi)
    : widget {wi}
    , SelectedItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(get_items()) - 1); }}}
    , HoveredItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(get_items()) - 1); }}}
    , _vScrollbar {orientation::Vertical}
{
    _vScrollbar.Changed.connect([this]() { request_redraw(this->name() + ": Scrollbar changed"); });
    SelectedItemIndex.Changed.connect([this](auto const&) { request_redraw(this->name() + ": SelectedItem changed"); });
    SelectedItemIndex(INVALID_INDEX);
    HoveredItemIndex.Changed.connect([this](auto const&) { request_redraw(this->name() + ": HoveredItem changed"); });
    HoveredItemIndex(INVALID_INDEX);
    MaxVisibleItems.Changed.connect([this](auto const&) { request_redraw(this->name() + ": MaxVisibleItems changed"); });
    MaxVisibleItems(5);
    Class("drop_down_list");
}

void drop_down_list::add_item(utf8_string const& item)
{
    add_item({.Text = item, .Icon = {}, .UserData = {}});
}

void drop_down_list::add_item(list_item const& item)
{
    _items.push_back(item);
    request_redraw(this->name() + ": item added");
}

void drop_down_list::clear_items()
{
    _items.clear();
    clear_sub_styles();
    request_redraw(this->name() + ": items cleared");
}

auto drop_down_list::select_item(utf8_string const& item) -> bool
{
    auto const& items {get_items()};
    for (isize i {0}; i < std::ssize(items); ++i) {
        if (items[i].Text == item) {
            SelectedItemIndex = i;
            return true;
        }
    }

    return false;
}

auto drop_down_list::get_item_at(isize index) const -> list_item const&
{
    return get_items().at(static_cast<usize>(index));
}

auto drop_down_list::selected_item() const -> list_item const&
{
    return get_items().at(SelectedItemIndex());
}

auto drop_down_list::item_count() const -> isize
{
    return std::ssize(get_items());
}

void drop_down_list::on_styles_changed()
{
    widget::on_styles_changed();

    _vScrollbar.reset();
}

void drop_down_list::on_draw(widget_painter& painter)
{
    _itemRectCache.clear();

    rect_f rect {draw_background(_style, painter)};

    // arrow
    auto const& fls {flags()};

    nav_arrows_style arrowStyle {};
    apply_sub_style(arrowStyle, -1, _style.NavArrowClass, {.Active = fls.Active && _mouseOverBox, .Hover = !fls.Active && _mouseOverBox});
    painter.draw_chevron(arrowStyle.NavArrow, rect);

    // text
    if (_style.Text.Font && SelectedItemIndex >= 0) {
        f32 const arrowWidth {arrowStyle.NavArrow.Size.Width.calc(rect.width())};
        painter.draw_text(_style.Text, {rect.left(), rect.top(), rect.width() - arrowWidth, rect.height()}, selected_item().Text);
    }

    if (_isExtended) {
        f32 const itemHeight {_style.ItemHeight.calc(rect.height())};

        auto const& items {get_items()};

        // list background
        rect_f listRect {Bounds()};
        listRect.Position.Y += listRect.height();
        f32 const listHeight {itemHeight * MaxVisibleItems};
        listRect.Size.Height = listHeight;
        listRect.Size.Height += _style.Margin.Top.calc(listHeight) + _style.Margin.Bottom.calc(listHeight);
        listRect.Size.Height += _style.Padding.Top.calc(listHeight) + _style.Padding.Bottom.calc(listHeight);
        listRect.Size.Height += _style.Border.Size.calc(listHeight);

        _visibleItems = static_cast<isize>(listRect.height() / itemHeight);

        painter.draw_background_and_border(_style, listRect, false);

        // scrollbar
        auto const scrollOffset {_vScrollbar.current_value() * get_scroll_max()};
        _vScrollbar.Visible = std::ssize(items) > MaxVisibleItems;

        auto const  thumbFlags {!_vScrollbar.is_mouse_over_thumb() ? widget_flags {}
                                    : fls.Active                   ? widget_flags {.Active = true}
                                                                   : widget_flags {.Hover = true}};
        thumb_style thumbStyle;
        apply_sub_style(thumbStyle, -2, _style.VScrollBar.ThumbClass, thumbFlags);
        _vScrollbar.draw(painter, _style.VScrollBar, thumbStyle.Thumb, listRect);

        scissor_guard const guard {painter, this};

        auto const paintItem {[&](isize i) {
            rect_f itemRect {listRect};
            itemRect.Size.Height = itemHeight;
            itemRect.Position.Y  = listRect.top() + (itemRect.height() * static_cast<f32>(i)) - scrollOffset;

            if (itemRect.bottom() > listRect.top() && itemRect.top() < listRect.bottom()) {
                item_style itemStyle {};
                apply_sub_style(itemStyle, i, _style.ItemClass, {.Active = i == SelectedItemIndex, .Hover = i == HoveredItemIndex});

                painter.draw_item(itemStyle.Item, itemRect, items[i]);
                _itemRectCache[i] = itemRect;
            }
        }};

        // content
        for (i32 i {0}; i < std::ssize(items); ++i) {
            if (i == HoveredItemIndex || i == SelectedItemIndex) { continue; }
            paintItem(i);
        }

        if (SelectedItemIndex >= 0) {
            paintItem(SelectedItemIndex());
        }
        if (HoveredItemIndex >= 0 && SelectedItemIndex != HoveredItemIndex) {
            paintItem(HoveredItemIndex());
        }
    } else {
        _visibleItems = 0;
    }
}

void drop_down_list::on_mouse_leave()
{
    HoveredItemIndex = INVALID_INDEX;

    if (_mouseOverBox) {
        _mouseOverBox = false;
        request_redraw(this->name() + ": mouse left");
    }
    _vScrollbar.mouse_leave();
}

void drop_down_list::on_mouse_hover(input::mouse::motion_event const& ev)
{
    // over scrollbar
    if (_vScrollbar.mouse_hover(*this, ev.Position)) {
        _mouseOverBox    = false;
        HoveredItemIndex = INVALID_INDEX;
        ev.Handled       = true;
        return;
    }

    // over box
    auto const mp {global_to_parent(*this, ev.Position)};
    if (Bounds->contains(mp)) {
        if (!_mouseOverBox) {
            _mouseOverBox    = true;
            HoveredItemIndex = INVALID_INDEX;
            ev.Handled       = true;
        }
        return;
    }

    _mouseOverBox = false;
    if (!_isExtended) { return; }

    // over list
    for (auto const& kvp : _itemRectCache) {
        if (!kvp.second.contains(mp)) { continue; }
        HoveredItemIndex = kvp.first;
        ev.Handled       = true;
        return;
    }

    HoveredItemIndex = INVALID_INDEX;
}

void drop_down_list::on_mouse_down(input::mouse::button_event const& ev)
{
    if (ev.Button == controls().PrimaryMouseButton) {
        _vScrollbar.mouse_down(*this, ev.Position);
        if (_mouseOverBox) {
            set_extended(!_isExtended);
        } else if (HoveredItemIndex != INVALID_INDEX) {
            if (SelectedItemIndex != HoveredItemIndex()) {
                SelectedItemIndex = HoveredItemIndex();
            }
        }

        ev.Handled = true;
    }
}

void drop_down_list::on_mouse_drag(input::mouse::motion_event const& ev)
{
    if (_vScrollbar.mouse_drag(*this, ev.Position)) {
        ev.Handled = true;
    }
}

void drop_down_list::on_mouse_up(input::mouse::button_event const& ev)
{
    if (ev.Button == controls().PrimaryMouseButton) {
        _vScrollbar.mouse_up(*this, ev.Position);
        ev.Handled = true;
    }
}

void drop_down_list::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (!_vScrollbar.Visible) { return; }
    if (ev.Scroll.Y == 0) { return; }

    HoveredItemIndex = INVALID_INDEX;

    f32 const scrollOffset {(ev.Scroll.Y > 0) ? -get_scroll_distance() : get_scroll_distance()};
    _vScrollbar.start_scroll(_vScrollbar.target_value() + scrollOffset, _style.VScrollBar.Bar.Delay);

    ev.Handled = true;
}

void drop_down_list::on_focus_lost()
{
    set_extended(false);
}

void drop_down_list::on_update(milliseconds deltaTime)
{
    _vScrollbar.update(deltaTime);
}

void drop_down_list::on_animation_step(string const& val)
{
    if (_isExtended && SelectedItemIndex >= 0) {
        _items[SelectedItemIndex].Icon.Region = val;
        request_redraw(this->name() + ": Animation Frame changed ");
    }
}

void drop_down_list::offset_content(rect_f& bounds, bool isHitTest) const
{
    rect_f    listRect {bounds};
    f32 const boxHeight {bounds.height()};

    widget::offset_content(bounds, isHitTest);

    if (!_isExtended) { return; }
    if (!isHitTest) { bounds.Position.Y += listRect.Size.Height; }
    f32 refListHeight {listRect.Size.Height};
    refListHeight -= _style.Margin.Top.calc(listRect.Size.Height) + _style.Margin.Bottom.calc(listRect.Size.Height);
    refListHeight -= _style.Padding.Top.calc(listRect.Size.Height) + _style.Padding.Bottom.calc(listRect.Size.Height);
    refListHeight -= _style.Border.Size.calc(listRect.Size.Height);
    f32 const itemHeight {_style.ItemHeight.calc(refListHeight)};
    f32 const listHeight {itemHeight * MaxVisibleItems};
    bounds.Size.Height = listHeight;

    if (isHitTest) {
        bounds.Size.Height += _style.Margin.Top.calc(listHeight) + _style.Margin.Bottom.calc(listHeight);
        bounds.Size.Height += _style.Border.Size.calc(listHeight);
        bounds.Size.Height += boxHeight;
    }
}

auto drop_down_list::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    auto const& items {get_items()};
    auto const  size {std::ssize(items)};

    retValue["selected_index"] = SelectedItemIndex();
    if (SelectedItemIndex >= 0 && SelectedItemIndex < size) {
        retValue["selected"] = items.at(SelectedItemIndex).Text;
    }
    retValue["hover_index"] = HoveredItemIndex();
    if (HoveredItemIndex >= 0 && HoveredItemIndex < size) {
        retValue["hover"] = items.at(HoveredItemIndex()).Text;
    }

    return retValue;
}

auto drop_down_list::get_items() const -> std::vector<list_item> const&
{
    return _items;
}

auto drop_down_list::get_item_height() const -> f32
{
    rect_f bounds {Bounds()};
    widget::offset_content(bounds, false);
    return _style.ItemHeight.calc(bounds.height());
}

void drop_down_list::set_extended(bool v)
{
    if (_isExtended != v) {
        _isExtended = v;
        request_redraw(this->name() + ": extended change");
    }
}

auto drop_down_list::get_scroll_distance() const -> f32
{
    return get_item_height() * static_cast<f32>(_visibleItems) / get_scroll_max();
}

auto drop_down_list::get_scroll_max() const -> f32
{
    return std::max(1.0f, (get_item_height() * static_cast<f32>(std::max(std::ssize(get_items()), MaxVisibleItems()))) - content_bounds().height());
}

} // namespace ui
