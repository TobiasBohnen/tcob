// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/DropDownList.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Transform.hpp"
#include "tcob/gfx/ui/Form.hpp"
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
    _vScrollbar.ValueChanged.connect([this]() {
        form().rehover_widget(this);
        queue_redraw();
    });

    SelectedItemIndex.Changed.connect([this](auto const&) { queue_redraw(); });
    SelectedItemIndex(INVALID_INDEX);
    HoveredItemIndex.Changed.connect([this](auto const&) { queue_redraw(); });
    HoveredItemIndex(INVALID_INDEX);
    MaxVisibleItems.Changed.connect([this](auto const&) { queue_redraw(); });
    MaxVisibleItems(5);

    Items.Changed.connect([this](auto const& val) {
        if ((std::ssize(val) <= SelectedItemIndex && SelectedItemIndex != INVALID_INDEX)
            || (std::ssize(val) <= HoveredItemIndex && HoveredItemIndex != INVALID_INDEX)) {
            SelectedItemIndex = INVALID_INDEX;
            HoveredItemIndex  = INVALID_INDEX;
            _itemRectCache.clear();
            clear_sub_styles();
            set_extended(false);
        }

        queue_redraw();
    });

    Class("drop_down_list");
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

auto drop_down_list::get_item_at(isize index) const -> item const&
{
    return get_items().at(static_cast<usize>(index));
}

auto drop_down_list::selected_item() const -> item const&
{
    return get_items().at(SelectedItemIndex);
}

auto drop_down_list::item_count() const -> isize
{
    return std::ssize(get_items());
}

void drop_down_list::on_styles_changed()
{
    widget::on_styles_changed();

    _vScrollbar.reset(0);
}

void drop_down_list::on_draw(widget_painter& painter)
{
    _itemRectCache.clear();

    rect_f rect {draw_background(_style, painter)};

    // arrow
    auto const& fls {flags()};

    nav_arrows_style arrowStyle {};
    prepare_sub_style(arrowStyle, -1, _style.NavArrowClass, {.Active = fls.Active && _mouseOverChevron, .Hover = !fls.Active && _mouseOverChevron});
    _chevronRectCache = painter.draw_nav_arrow(arrowStyle.NavArrow, rect, _isExtended ? direction::Up : direction::Down);

    // text
    if (_style.Text.Font && SelectedItemIndex >= 0) {
        f32 const arrowWidth {arrowStyle.NavArrow.Size.Width.calc(rect.width())};
        painter.draw_text(_style.Text, {rect.left(), rect.top(), rect.width() - arrowWidth, rect.height()}, selected_item().Text);
    }

    if (_isExtended) {
        f32 const itemHeight {_style.ItemHeight.calc(rect.height())};

        point_f const globalOffset {global_position() - Bounds->Position - form().Bounds->Position};

        painter.add_overlay([this, globalOffset, itemHeight, isActive = fls.Active](widget_painter& painter) {
            gfx::transform xform;
            xform.translate(globalOffset);
            painter.begin(Alpha, xform);

            rect_f listRect {Bounds};
            listRect.Position.Y += listRect.height();
            f32 const listHeight {itemHeight * MaxVisibleItems};
            listRect.Size.Height = listHeight;
            listRect.Size.Height += _style.Margin.Top.calc(listHeight) + _style.Margin.Bottom.calc(listHeight);
            listRect.Size.Height += _style.Padding.Top.calc(listHeight) + _style.Padding.Bottom.calc(listHeight);
            listRect.Size.Height += _style.Border.Size.calc(listHeight);

            _vScrollbar.Visible = std::ssize(get_items()) > MaxVisibleItems;

            // list background
            painter.draw_background_and_border(_style, listRect, false);
            _visibleItems = static_cast<isize>(listRect.height() / itemHeight);

            // scrollbar
            auto const scrollOffset {_vScrollbar.current_value() * get_scroll_max()};

            auto const  thumbFlags {!_vScrollbar.is_mouse_over_thumb() ? widget_flags {}
                                        : isActive                     ? widget_flags {.Active = true}
                                                                       : widget_flags {.Hover = true}};
            thumb_style thumbStyle;
            prepare_sub_style(thumbStyle, -2, _style.VScrollBar.ThumbClass, thumbFlags);
            _vScrollbar.draw(painter, _style.VScrollBar, thumbStyle.Thumb, listRect);

            // items
            scissor_guard const guard {painter, this};
            auto const&         items {get_items()};
            auto const          paintItem {[&](isize i) {
                rect_f itemRect {listRect};
                itemRect.Size.Height = itemHeight;
                itemRect.Position.Y  = listRect.top() + (itemRect.height() * static_cast<f32>(i)) - scrollOffset;

                if (itemRect.bottom() > listRect.top() && itemRect.top() < listRect.bottom()) {
                    item_style itemStyle {};
                    prepare_sub_style(itemStyle, i, _style.ItemClass, {.Active = i == SelectedItemIndex, .Hover = i == HoveredItemIndex});

                    painter.draw_item(itemStyle.Item, itemRect, items[i]);
                    _itemRectCache[i] = itemRect;
                } else {
                    reset_sub_style(i, _style.ItemClass, {.Active = i == SelectedItemIndex, .Hover = i == HoveredItemIndex});
                }
            }};

            // content
            for (isize i {0}; i < std::ssize(items); ++i) {
                if (i == HoveredItemIndex || i == SelectedItemIndex) { continue; }
                paintItem(i);
            }

            if (SelectedItemIndex >= 0) {
                paintItem(SelectedItemIndex);
            }
            if (HoveredItemIndex >= 0 && SelectedItemIndex != HoveredItemIndex) {
                paintItem(HoveredItemIndex);
            }

            painter.end();
        });
    } else {
        _visibleItems = 0;
    }
}

void drop_down_list::on_mouse_leave()
{
    HoveredItemIndex = INVALID_INDEX;

    if (_mouseOverBox) {
        _mouseOverBox     = false;
        _mouseOverChevron = false;
        queue_redraw();
    }
    _vScrollbar.mouse_leave();
}

void drop_down_list::on_mouse_hover(input::mouse::motion_event const& ev)
{
    // over scrollbar
    if (_vScrollbar.mouse_hover(*this, ev.Position)) {
        _mouseOverBox     = false;
        _mouseOverChevron = false;
        HoveredItemIndex  = INVALID_INDEX;
        ev.Handled        = true;
        return;
    }

    // over box
    auto const mp {global_to_parent(*this, ev.Position)};
    if (Bounds->contains(mp)) {
        bool const overChevron {_chevronRectCache.contains(mp)};
        if (_mouseOverChevron != overChevron) {
            _mouseOverChevron = overChevron;
            queue_redraw();
        }

        if (!_mouseOverBox) {
            _mouseOverBox    = true;
            HoveredItemIndex = INVALID_INDEX;
            ev.Handled       = true;
        }
        return;
    }

    _mouseOverBox     = false;
    _mouseOverChevron = false;
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

void drop_down_list::on_mouse_drag(input::mouse::motion_event const& ev)
{
    if (_vScrollbar.mouse_drag(*this, ev.Position)) {
        ev.Handled = true;
    }
}

void drop_down_list::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button == controls().PrimaryMouseButton) {
        _vScrollbar.mouse_down(*this, ev.Position);
        if (_mouseOverBox) {
            set_extended(!_isExtended);
        } else if (HoveredItemIndex != INVALID_INDEX) {
            if (SelectedItemIndex != HoveredItemIndex) {
                SelectedItemIndex = *HoveredItemIndex;
            }
        }

        ev.Handled = true;
    }
}

void drop_down_list::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (ev.Button == controls().PrimaryMouseButton) {
        _vScrollbar.mouse_up(*this, ev.Position);
        ev.Handled = true;
    }
}

void drop_down_list::on_double_click()
{
    if (!_mouseOverBox) {
        set_extended(false);
    }
}

void drop_down_list::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (!_vScrollbar.Visible) { return; }
    if (ev.Scroll.Y == 0) { return; }

    HoveredItemIndex = INVALID_INDEX;

    f32 const scrollOffset {(ev.Scroll.Y > 0) ? -get_scroll_distance() : get_scroll_distance()};
    _vScrollbar.start(_vScrollbar.target_value() + scrollOffset);

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
        Items.mutate([&](auto& items) {
            auto& item {items[SelectedItemIndex]};
            item.Icon.TextureRegion = val;
            if (item.Icon.Texture) {
                queue_redraw();
            }
            return false;
        });
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

    retValue["selected_index"] = *SelectedItemIndex;
    if (SelectedItemIndex >= 0 && SelectedItemIndex < size) {
        retValue["selected"] = items.at(SelectedItemIndex).Text;
    }
    retValue["hover_index"] = *HoveredItemIndex;
    if (HoveredItemIndex >= 0 && HoveredItemIndex < size) {
        retValue["hover"] = items.at(HoveredItemIndex).Text;
    }

    return retValue;
}

auto drop_down_list::get_items() const -> std::vector<item> const&
{
    return *Items;
}

auto drop_down_list::get_item_height() const -> f32
{
    rect_f bounds {*Bounds};
    widget::offset_content(bounds, false);
    return _style.ItemHeight.calc(bounds.height());
}

void drop_down_list::set_extended(bool v)
{
    if (_isExtended != v) {
        _isExtended = v;
        queue_redraw();
    }
}

auto drop_down_list::get_scroll_distance() const -> f32
{
    return get_item_height() * static_cast<f32>(_visibleItems) / get_scroll_max();
}

auto drop_down_list::get_scroll_max() const -> f32
{
    return std::max(1.0f, (get_item_height() * static_cast<f32>(std::max(std::ssize(get_items()), *MaxVisibleItems))) - content_bounds().height());
}

} // namespace ui
