// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/TabContainer.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <memory>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Transform.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

void tab_container::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    target.HeaderSize = length::Lerp(left.HeaderSize, right.HeaderSize, step);
}

tab_container::tab_container(init const& wi)
    : widget_container {wi}
    , ActiveTabIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(_tabs) - 1); }}}
    , HoveredTabIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(_tabs) - 1); }}}
{
    ActiveTabIndex.Changed.connect([this](auto const&) { queue_redraw(); });
    ActiveTabIndex(INVALID_INDEX);
    HoveredTabIndex.Changed.connect([this](auto const&) { queue_redraw(); });
    HoveredTabIndex(INVALID_INDEX);

    MaxTabsPerLine.Changed.connect([this](auto const&) { queue_redraw(); });
    MaxTabsPerLine(std::numeric_limits<isize>::max());

    Class("tab_container");
}

void tab_container::on_prepare_redraw()
{
    prepare_style(_style);

    auto const rect {content_bounds()};
    for (auto& t : _tabs) {
        t->Bounds = {point_f::Zero, rect.Size};
    }

    widget_container::on_prepare_redraw();
}

void tab_container::remove_tab(widget* tab)
{
    for (usize i {0}; i < _tabs.size(); ++i) {
        if (_tabs[i].get() == tab) {
            _tabs.erase(_tabs.begin() + i);
            _tabLabels.erase(_tabLabels.begin() + i);
            clear_sub_styles();
            break;
        }
    }

    if (_tabs.empty()) {
        ActiveTabIndex = INVALID_INDEX;
    } else {
        ActiveTabIndex = 0;
    }

    queue_redraw();
}

void tab_container::clear_tabs()
{
    while (!_tabs.empty()) {
        remove_tab(_tabs.front().get());
    }
}

void tab_container::change_tab_label(widget* tab, utf8_string const& label)
{
    for (isize i {0}; i < std::ssize(_tabs); ++i) {
        if (_tabs[i].get() == tab) {
            _tabLabels[i].Text = label;
            break;
        }
    }
    queue_redraw();
}

void tab_container::change_tab_label(widget* tab, item const& label)
{
    for (isize i {0}; i < std::ssize(_tabs); ++i) {
        if (_tabs[i].get() == tab) {
            _tabLabels[i] = label;
            break;
        }
    }
    queue_redraw();
}

auto tab_container::find_child_at(point_i pos) -> std::shared_ptr<widget>
{
    if (ActiveTabIndex < 0 || ActiveTabIndex >= std::ssize(_tabs)) {
        return nullptr;
    }

    auto& activeTab {_tabs[ActiveTabIndex]};
    if (!activeTab->hit_test(pos)) { return nullptr; }

    if (auto container {std::dynamic_pointer_cast<widget_container>(activeTab)}) {
        if (auto retValue {container->find_child_at(pos)}) {
            return retValue;
        }
    }
    return activeTab;
}

auto tab_container::widgets() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _tabs;
}

void tab_container::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

    // tabs
    _tabRectCache.clear();

    rect_f tabHeaderRect {rect};
    switch (_style.HeaderPosition) {
    case position::Top:
        tabHeaderRect.Size.Height = _style.HeaderSize.calc(tabHeaderRect.height());
        break;
    case position::Bottom:
        tabHeaderRect.Size.Height = _style.HeaderSize.calc(tabHeaderRect.height());
        tabHeaderRect.Position.Y  = rect.bottom() - (tabHeaderRect.height() * static_cast<f32>(get_tab_line_count()));
        break;
    case position::Left:
        tabHeaderRect.Size.Width = _style.HeaderSize.calc(tabHeaderRect.width());
        break;
    case position::Right:
        tabHeaderRect.Size.Width = _style.HeaderSize.calc(tabHeaderRect.width());
        tabHeaderRect.Position.X = rect.right() - (tabHeaderRect.width() * static_cast<f32>(get_tab_line_count()));
        break;

    case position::None: return;
    }

    isize const maxItems {std::min(*MaxTabsPerLine, std::ssize(_tabs))};
    f32         tabRectX {0};
    f32         tabRectY {0};

    auto const getNextTabRect {[&](item const& item, item_style const& itemStyle) {
        rect_f retValue {};

        rect_f itemTabHeaderRect {tabHeaderRect};
        itemTabHeaderRect -= itemStyle.Item.Padding;
        itemTabHeaderRect -= itemStyle.Item.Border.thickness();

        switch (_style.HeaderPosition) {
        case position::Top:
        case position::Bottom: {
            f32 const itemHeight {itemTabHeaderRect.height()};
            f32       itemWidth {0};

            switch (_style.HeaderMode) {
            case tab_container::header_mode::Fill: {
                itemWidth = itemTabHeaderRect.width() / static_cast<f32>(maxItems);
            } break;
            case tab_container::header_mode::Compact: {
                auto const textFormat {painter.format_text(itemStyle.Item.Text, {itemTabHeaderRect.width() / static_cast<f32>(maxItems), itemHeight}, item.Text)};
                itemWidth = textFormat.UsedSize.Width;
            } break;
            }

            retValue.Position.X  = itemTabHeaderRect.left() + tabRectX;
            retValue.Size.Width  = itemWidth;
            retValue.Position.Y  = itemTabHeaderRect.top();
            retValue.Size.Height = itemHeight;
            tabRectX += itemWidth;
        } break;
        case position::Left:
        case position::Right: {
            f32       itemHeight {0};
            f32 const itemWidth {itemTabHeaderRect.width()};

            switch (_style.HeaderMode) {
            case tab_container::header_mode::Fill: {
                itemHeight = itemTabHeaderRect.height() / static_cast<f32>(maxItems);
            } break;
            case tab_container::header_mode::Compact: {
                auto const textFormat {painter.format_text(itemStyle.Item.Text, {itemWidth, itemTabHeaderRect.height() / static_cast<f32>(maxItems)}, item.Text)};
                itemHeight = textFormat.UsedSize.Height;
            } break;
            }

            retValue.Position.X  = itemTabHeaderRect.left();
            retValue.Size.Width  = itemWidth;
            retValue.Position.Y  = itemTabHeaderRect.top() + tabRectY;
            retValue.Size.Height = itemHeight;
            tabRectY += itemHeight;
        } break;
        case position::None: break;
        }

        return retValue;
    }};

    for (i32 i {0}; i < std::ssize(_tabs); ++i) {
        item_style tabStyle {};
        prepare_sub_style(tabStyle, i, _style.TabItemClass, {.Active = i == ActiveTabIndex, .Hover = i == HoveredTabIndex});

        rect_f const tabRect {getNextTabRect(_tabLabels[i], tabStyle)};
        painter.draw_item(tabStyle.Item, tabRect, _tabLabels[i]);
        _tabRectCache.push_back(tabRect);
    }
}

void tab_container::on_draw_children(widget_painter& painter)
{
    // prepare_style(_style);

    rect_f rect {content_bounds()};

    // content
    scissor_guard const guard {painter, this};

    // active tab
    if (ActiveTabIndex >= 0 && ActiveTabIndex < std::ssize(_tabs)) {
        auto          xform {gfx::transform::Identity};
        point_f const translate {rect.Position + paint_offset()};
        xform.translate(translate);

        auto& tab {_tabs[ActiveTabIndex]};
        painter.begin(Alpha, xform);
        tab->draw(painter);
        painter.end();
    }
}

void tab_container::on_mouse_leave()
{
    HoveredTabIndex = INVALID_INDEX;
}

void tab_container::on_mouse_hover(input::mouse::motion_event const& ev)
{
    HoveredTabIndex = INVALID_INDEX;

    auto const mp {global_to_parent(*this, ev.Position)};
    for (i32 i {0}; i < std::ssize(_tabRectCache); ++i) {
        if (!_tabRectCache[i].contains(mp)) { continue; }

        HoveredTabIndex = i;
        ev.Handled      = true;
        break;
    }
}

void tab_container::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button == controls().PrimaryMouseButton) {
        if (HoveredTabIndex != INVALID_INDEX) {
            ActiveTabIndex = *HoveredTabIndex;
        }

        ev.Handled = true;
    }
}

void tab_container::on_update(milliseconds /* deltaTime */)
{
}

void tab_container::on_animation_step(string const& val)
{
    if (ActiveTabIndex >= 0) {
        auto& tab {_tabLabels[ActiveTabIndex]};
        tab.Icon.TextureRegion = val;
        if (tab.Icon.Texture) {
            queue_redraw();
        }
    }
}

void tab_container::offset_tab_content(rect_f& bounds, style const& style) const
{
    switch (style.HeaderPosition) {
    case position::Top:
    case position::Bottom: {
        f32 const size {style.HeaderSize.calc(bounds.height()) * get_tab_line_count()};
        bounds.Size.Height -= size;
        if (style.HeaderPosition == position::Top) { bounds.Position.Y += size; }
    } break;
    case position::Left:
    case position::Right: {
        f32 const size {style.HeaderSize.calc(bounds.width()) * get_tab_line_count()};
        bounds.Size.Width -= size;
        if (style.HeaderPosition == position::Left) { bounds.Position.X += size; }
    } break;
    case position::None: break;
    }
}

auto tab_container::get_tab_line_count() const -> isize
{
    if (MaxTabsPerLine > 0) {
        return static_cast<isize>(std::ceil(static_cast<f32>(std::ssize(_tabs)) / static_cast<f32>(*MaxTabsPerLine)));
    }

    return 1;
}

void tab_container::offset_content(rect_f& bounds, bool isHitTest) const
{
    widget::offset_content(bounds, isHitTest);

    // subtract tab bar from content
    if (isHitTest) { return; }
    offset_tab_content(bounds, _style);
}

auto tab_container::attributes() const -> widget_attributes
{
    auto retValue {widget_container::attributes()};
    retValue["active_index"] = *ActiveTabIndex;
    if (ActiveTabIndex >= 0 && ActiveTabIndex < std::ssize(_tabs)) {
        retValue["active"] = _tabLabels[ActiveTabIndex].Text;
    }
    retValue["hover_index"] = *HoveredTabIndex;
    if (HoveredTabIndex >= 0 && HoveredTabIndex < std::ssize(_tabs)) {
        retValue["hover"] = _tabLabels[HoveredTabIndex].Text;
    }
    return retValue;
}

} // namespace ui
