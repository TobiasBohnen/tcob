// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/TabContainer.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

tab_container::tab_container(init const& wi)
    : widget_container {wi}
    , ActiveTabIndex {{[&](isize val) -> isize { return std::clamp<isize>(val, -1, std::ssize(_tabs) - 1); }}}
    , HoveredTabIndex {{[&](isize val) -> isize { return std::clamp<isize>(val, -1, std::ssize(_tabs) - 1); }}}
{
    ActiveTabIndex.Changed.connect([&](auto const&) { force_redraw(this->name() + ": ActiveTab changed"); });
    ActiveTabIndex(-1);
    HoveredTabIndex.Changed.connect([&](auto const&) { force_redraw(this->name() + ": HoveredTab changed"); });
    HoveredTabIndex(-1);

    Class("tab_container");
}

void tab_container::remove_tab(widget* tab)
{
    for (isize i {0}; i < std::ssize(_tabs); ++i) {
        if (_tabs[i].get() == tab) {
            _tabs.erase(_tabs.begin() + i);
            _tabLabels.erase(_tabLabels.begin() + i);
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
    ActiveTabIndex = 0;
    force_redraw(this->name() + ": tabs cleared");
}

void tab_container::change_tab_label(widget* tab, utf8_string const& label)
{
    for (isize i {0}; i < std::ssize(_tabs); ++i) {
        if (_tabs[i].get() == tab) {
            _tabLabels[i] = label;
            break;
        }
    }
    force_redraw(this->name() + ": tab renamed");
}

auto tab_container::find_child_at(point_f pos) -> std::shared_ptr<widget>
{
    if (ActiveTabIndex >= 0 && ActiveTabIndex < std::ssize(_tabs)) {
        auto& activeTab {_tabs[ActiveTabIndex()]};
        if (activeTab->hit_test(pos)) {
            if (auto retValue {activeTab->find_child_at(pos)}) {
                return retValue;
            }
            return activeTab;
        }
    }

    return nullptr;
}

auto tab_container::widgets() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _tabs;
}

void tab_container::force_redraw(string const& reason)
{
    widget_container::force_redraw(reason);
    _isDirty = true;
}

void tab_container::on_styles_changed()
{
    widget_container::on_styles_changed();
    _isDirty = true;
}

void tab_container::on_paint(widget_painter& painter)
{
    if (auto const* style {current_style<tab_container::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        // tabs
        _tabRects.clear();
        if (style->TabBarPosition != position::Hidden) {
            rect_f tabBarRowRect {rect};
            tabBarRowRect.Size.Height = style->TabBarHeight.calc(tabBarRowRect.height());
            if (style->TabBarPosition == position::Bottom) {
                f32 const tabRows {style->MaxTabsPerRow > 0 ? std ::ceil(std::ssize(_tabs) / static_cast<f32>(style->MaxTabsPerRow)) : 1};
                tabBarRowRect.Position.Y = rect.bottom() - (tabBarRowRect.height() * tabRows);
            }

            for (i32 i {0}; i < std::ssize(_tabs); ++i) {
                if (auto const* tabStyle {get_tab_style(i)}) {
                    rect_f const tabRect {get_tab_rect(*style, *tabStyle, i, tabBarRowRect)};
                    painter.draw_item(tabStyle->Item, tabRect, _tabLabels[i]);
                    _tabRects.push_back(tabRect);
                }
            }
        }

        // content
        scissor_guard const guard {painter, this};

        // active tab
        if (ActiveTabIndex >= 0 && ActiveTabIndex < std::ssize(_tabs)) {
            offset_tab_content(rect, *style);

            auto          xform {transform::Identity};
            point_f const translate {rect.Position + paint_offset()};
            xform.translate(translate);

            auto& tab {_tabs[ActiveTabIndex()]};
            painter.begin(Alpha(), xform);
            tab->paint(painter);
            painter.end();
        }
    }
}

void tab_container::on_mouse_leave()
{
    widget::on_mouse_leave();

    HoveredTabIndex = -1;
}

void tab_container::on_mouse_hover(input::mouse::motion_event const& ev)
{
    HoveredTabIndex = -1;

    widget_container::on_mouse_hover(ev);

    auto const mp {global_to_parent_local(ev.Position)};
    for (i32 i {0}; i < std::ssize(_tabRects); ++i) {
        if (_tabRects[i].contains(mp)) {
            HoveredTabIndex = i;
            break;
        }
    }

    ev.Handled = true;
}

void tab_container::on_mouse_down(input::mouse::button_event const& ev)
{
    widget::on_mouse_down(ev);

    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        force_redraw(this->name() + ": mouse down");

        if (HoveredTabIndex != -1) {
            ActiveTabIndex = HoveredTabIndex();
        }

        ev.Handled = true;
    }
}

void tab_container::on_update(milliseconds /* deltaTime */)
{
    if (_isDirty) {
        update_tab_bounds();
        _isDirty = false;
    }
}

auto tab_container::get_tab_rect(style const& style, item_style const& itemStyle, isize index, rect_f const& rect) const -> rect_f
{
    isize const maxItems {std::min(style.MaxTabsPerRow, std::ssize(_tabs))};
    rect_f      retValue {rect};
    retValue.Position.X  = rect.left() + (rect.width() / maxItems) * (index % maxItems);
    retValue.Size.Width  = rect.width() / maxItems;
    retValue.Position.Y  = retValue.top() + (rect.height() * (index / maxItems));
    retValue.Size.Height = rect.height();

    retValue -= itemStyle.Item.Border.thickness();
    return retValue;
}

auto tab_container::get_tab_style(isize index) const -> item_style*
{
    auto const* style {current_style<tab_container::style>()};
    return index == ActiveTabIndex ? get_sub_style<item_style>(style->TabItemClass, {.Active = true})
        : index == HoveredTabIndex ? get_sub_style<item_style>(style->TabItemClass, {.Hover = true})
                                   : get_sub_style<item_style>(style->TabItemClass, {});
}

void tab_container::offset_tab_content(rect_f& bounds, style const& style) const
{
    f32 barHeight {style.TabBarHeight.calc(bounds.height())};
    if (style.MaxTabsPerRow != -1) {
        barHeight *= std::ceil(std::ssize(_tabs) / static_cast<f32>(style.MaxTabsPerRow));
    }

    bounds.Size.Height -= barHeight;
    if (style.TabBarPosition == position::Top) {
        bounds.Position.Y += barHeight;
    }
}

void tab_container::offset_content(rect_f& bounds, bool isHitTest) const
{
    widget::offset_content(bounds, isHitTest);

    if (!isHitTest) {
        if (auto const* style {current_style<tab_container::style>()}) {
            offset_tab_content(bounds, *style);
        }
    }
}

void tab_container::update_tab_bounds()
{
    auto rect {content_bounds()};
    for (auto& t : _tabs) {
        t->Bounds = {point_f::Zero, rect.Size};
    }
}

} // namespace ui
