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
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

void tab_container::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    target.TabBarHeight = length::Lerp(left.TabBarHeight, right.TabBarHeight, step);
}

tab_container::tab_container(init const& wi)
    : widget_container {wi}
    , ActiveTabIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(_tabs) - 1); }}}
    , HoveredTabIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(_tabs) - 1); }}}
{
    ActiveTabIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": ActiveTab changed"); });
    ActiveTabIndex(INVALID_INDEX);
    HoveredTabIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": HoveredTab changed"); });
    HoveredTabIndex(INVALID_INDEX);

    MaxTabsPerRow.Changed.connect([this](auto const&) { force_redraw(this->name() + ": MaxTabsPerRow changed"); });
    MaxTabsPerRow(std::numeric_limits<isize>::max());

    Class("tab_container");
}

void tab_container::prepare_redraw()
{
    widget_container::prepare_redraw();
    _updateTabs = true;
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
    ActiveTabIndex = 0;
    force_redraw(this->name() + ": tab removed");
}

void tab_container::clear_tabs()
{
    _tabs.clear();
    _tabLabels.clear();
    clear_sub_styles();

    ActiveTabIndex = 0;
    force_redraw(this->name() + ": tabs cleared");
}

void tab_container::change_tab_label(widget* tab, utf8_string const& label)
{
    for (isize i {0}; i < std::ssize(_tabs); ++i) {
        if (_tabs[i].get() == tab) {
            _tabLabels[i].Text = label;
            break;
        }
    }
    force_redraw(this->name() + ": tab renamed");
}

auto tab_container::find_child_at(point_f pos) -> std::shared_ptr<widget>
{
    if (ActiveTabIndex < 0 || ActiveTabIndex >= std::ssize(_tabs)) {
        return nullptr;
    }

    auto& activeTab {_tabs[ActiveTabIndex()]};
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

void tab_container::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    // tabs
    _tabRectCache.clear();
    if (_style.TabBarPosition != position::Hidden) {

        rect_f tabBarRowRect {rect};
        tabBarRowRect.Size.Height = _style.TabBarHeight.calc(tabBarRowRect.height());

        if (_style.TabBarPosition == position::Bottom) {
            tabBarRowRect.Position.Y = rect.bottom() - (tabBarRowRect.height() * get_tab_row_count());
        }

        auto const get_tab_rect {[&tabBarRowRect, this](item_style const& itemStyle, isize index) {
            isize const maxItems {std::min(*MaxTabsPerRow, std::ssize(_tabs))};
            rect_f      retValue {tabBarRowRect};
            retValue.Position.X  = tabBarRowRect.left() + (tabBarRowRect.width() / maxItems) * (index % maxItems);
            retValue.Size.Width  = tabBarRowRect.width() / maxItems;
            retValue.Position.Y  = retValue.top() + (tabBarRowRect.height() * (index / maxItems));
            retValue.Size.Height = tabBarRowRect.height();

            retValue -= itemStyle.Item.Border.thickness();
            return retValue;
        }};

        for (i32 i {0}; i < std::ssize(_tabs); ++i) {
            item_style tabStyle {};
            update_sub_style(tabStyle, i, _style.TabItemClass, {.Active = i == ActiveTabIndex, .Hover = i == HoveredTabIndex});

            rect_f const tabRect {get_tab_rect(tabStyle, i)};
            painter.draw_item(tabStyle.Item, tabRect, _tabLabels[i]);
            _tabRectCache.push_back(tabRect);
        }
    }

    // content
    scissor_guard const guard {painter, this};

    // active tab
    if (ActiveTabIndex >= 0 && ActiveTabIndex < std::ssize(_tabs)) {
        offset_tab_content(rect, _style);

        auto          xform {gfx::transform::Identity};
        point_f const translate {rect.Position + paint_offset()};
        xform.translate(translate);

        auto& tab {_tabs[ActiveTabIndex()]};
        painter.begin(Alpha(), xform);
        tab->paint(painter);
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

    auto const mp {global_to_local(ev.Position)};
    for (i32 i {0}; i < std::ssize(_tabRectCache); ++i) {
        if (!_tabRectCache[i].contains(mp)) { continue; }

        HoveredTabIndex = i;
        ev.Handled      = true;
        break;
    }
}

void tab_container::on_mouse_down(input::mouse::button_event const& ev)
{
    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        force_redraw(this->name() + ": mouse down");

        if (HoveredTabIndex != INVALID_INDEX) {
            ActiveTabIndex = HoveredTabIndex();
        }

        ev.Handled = true;
    }
}

void tab_container::on_update(milliseconds /* deltaTime */)
{
    if (_updateTabs) {
        update_style(_style);

        auto const rect {content_bounds()};
        for (auto& t : _tabs) {
            t->Bounds = {point_f::Zero, rect.Size};
        }
        _updateTabs = false;
    }
}

void tab_container::offset_tab_content(rect_f& bounds, style const& style) const
{
    f32 const barHeight {style.TabBarHeight.calc(bounds.height()) * get_tab_row_count()};

    bounds.Size.Height -= barHeight;
    if (style.TabBarPosition == position::Top) { bounds.Position.Y += barHeight; }
}

auto tab_container::get_tab_row_count() const -> isize
{
    if (*MaxTabsPerRow > 0) {
        return static_cast<isize>(std::ceil(std::ssize(_tabs) / static_cast<f32>(*MaxTabsPerRow)));
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
    retValue["active_index"] = ActiveTabIndex();
    if (ActiveTabIndex >= 0 && ActiveTabIndex < std::ssize(_tabs)) {
        retValue["active"] = _tabLabels[ActiveTabIndex].Text;
    }
    retValue["hover_index"] = HoveredTabIndex();
    if (HoveredTabIndex >= 0 && HoveredTabIndex < std::ssize(_tabs)) {
        retValue["hover"] = _tabLabels[HoveredTabIndex].Text;
    }
    return retValue;
}

} // namespace ui
