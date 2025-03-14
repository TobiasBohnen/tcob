// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Accordion.hpp"

#include <algorithm>
#include <iterator>
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

void accordion::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    target.SectionBarHeight = length::Lerp(left.SectionBarHeight, right.SectionBarHeight, step);
}

accordion::accordion(init const& wi)
    : widget_container {wi}
    , ActiveSectionIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(_sections) - 1); }}}
    , HoveredSectionIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(_sections) - 1); }}}
{
    ActiveSectionIndex.Changed.connect([this](auto const&) { request_redraw(this->name() + ": ActiveSection changed"); });
    ActiveSectionIndex(INVALID_INDEX);
    HoveredSectionIndex.Changed.connect([this](auto const&) { request_redraw(this->name() + ": HoveredSection changed"); });
    HoveredSectionIndex(INVALID_INDEX);
    MaximizeActiveSection.Changed.connect([this](auto const&) { request_redraw(this->name() + ": MaximizeActiveSection changed"); });
    MaximizeActiveSection(false);

    Class("accordion");
}

void accordion::prepare_redraw()
{
    widget_container::prepare_redraw();
    _updateSections = true;
}

void accordion::remove_section(widget* sec)
{
    for (usize i {0}; i < _sections.size(); ++i) {
        if (_sections[i].get() == sec) {
            _sections.erase(_sections.begin() + i);
            _sectionLabels.erase(_sectionLabels.begin() + i);
            clear_sub_styles();
            break;
        }
    }
    ActiveSectionIndex = 0;
    request_redraw(this->name() + ": section removed");
}

void accordion::clear_sections()
{
    _sections.clear();
    _sectionLabels.clear();
    clear_sub_styles();

    ActiveSectionIndex = 0;
    request_redraw(this->name() + ": sections cleared");
}

void accordion::change_section_label(widget* sec, utf8_string const& label)
{
    for (usize i {0}; i < _sections.size(); ++i) {
        if (_sections[i].get() == sec) {
            _sectionLabels[i].Text = label;
            break;
        }
    }
    request_redraw(this->name() + ": section renamed");
}

auto accordion::find_child_at(point_f pos) -> std::shared_ptr<widget>
{
    if (ActiveSectionIndex < 0 || ActiveSectionIndex >= std::ssize(_sections)) {
        return nullptr;
    }

    auto& activeSection {_sections[ActiveSectionIndex()]};
    if (!activeSection->hit_test(pos)) { return nullptr; }

    if (auto container {std::dynamic_pointer_cast<widget_container>(activeSection)}) {
        if (auto retValue {container->find_child_at(pos)}) {
            return retValue;
        }
    }
    return activeSection;
}

auto accordion::widgets() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _sections;
}

void accordion::on_draw(widget_painter& painter)
{
    apply_style(_style);

    rect_f rect {Bounds()};
    // TODO: section chevron
    //  background
    painter.draw_background_and_border(_style, rect, false);

    // sections
    f32 const  sectionHeight {_style.SectionBarHeight.calc(rect.height())};
    auto const get_section_rect {[&](item_style const& itemStyle, isize index) {
        rect_f retValue {rect};
        retValue.Position.Y += sectionHeight * index;
        retValue.Size.Height = sectionHeight;
        retValue -= itemStyle.Item.Border.thickness();
        if (ActiveSectionIndex >= 0 && index > ActiveSectionIndex) {
            retValue.Position.Y += content_bounds().height();
        }
        return retValue;
    }};

    _sectionRectCache.clear();
    auto const paint_section {[&](isize i, isize rectIndex) {
        item_style sectionStyle {};
        apply_sub_style(sectionStyle, i, _style.SectionItemClass, {.Active = i == ActiveSectionIndex, .Hover = i == HoveredSectionIndex});

        rect_f const sectionRect {get_section_rect(sectionStyle, rectIndex)};
        painter.draw_item(sectionStyle.Item, sectionRect, _sectionLabels[i]);
        _sectionRectCache.push_back(sectionRect);
    }};

    if (MaximizeActiveSection() && ActiveSectionIndex >= 0) {
        paint_section(ActiveSectionIndex(), 0);
    } else {
        for (isize i {0}; i < std::ssize(_sections); ++i) {
            paint_section(i, i);
        }
    }
}

void accordion::on_draw_children(widget_painter& painter)
{
    apply_style(_style);

    rect_f rect {content_bounds()};

    // content
    scissor_guard const guard {painter, this};

    // active section
    if (ActiveSectionIndex >= 0 && ActiveSectionIndex < std::ssize(_sections)) {
        auto          xform {gfx::transform::Identity};
        point_f const translate {rect.Position + paint_offset()};
        xform.translate(translate);

        auto& tab {_sections[ActiveSectionIndex()]};
        painter.begin(Alpha(), xform);
        tab->draw(painter);
        painter.end();
    }
}

void accordion::on_mouse_leave()
{
    HoveredSectionIndex = INVALID_INDEX;
}

void accordion::on_mouse_hover(input::mouse::motion_event const& ev)
{
    HoveredSectionIndex = INVALID_INDEX;

    auto const mp {global_to_parent(*this, ev.Position)};
    for (i32 i {0}; i < std::ssize(_sectionRectCache); ++i) {
        if (!_sectionRectCache[i].contains(mp)) { continue; }

        if (MaximizeActiveSection() && ActiveSectionIndex >= 0) {
            HoveredSectionIndex = ActiveSectionIndex();
        } else {
            HoveredSectionIndex = i;
        }

        ev.Handled = true;
        break;
    }
}

void accordion::on_mouse_down(input::mouse::button_event const& ev)
{
    if (ev.Button == controls().PrimaryMouseButton) {
        if (HoveredSectionIndex >= 0) {
            if (ActiveSectionIndex == HoveredSectionIndex) {
                ActiveSectionIndex = INVALID_INDEX;
                if (MaximizeActiveSection()) { HoveredSectionIndex = 0; }
            } else {
                ActiveSectionIndex = HoveredSectionIndex();
            }
        }

        ev.Handled = true;
    }
}

void accordion::on_update(milliseconds /* deltaTime */)
{
    if (_updateSections) {
        apply_style(_style);

        auto const rect {content_bounds()};
        for (auto& t : _sections) {
            t->Bounds = {point_f::Zero, rect.Size};
        }
        _updateSections = false;
    }
}

void accordion::offset_section_content(rect_f& bounds, style const& style) const
{
    f32 const barHeight {style.SectionBarHeight.calc(bounds.height())};
    bounds.Size.Height -= barHeight * (MaximizeActiveSection() ? 1 : _sections.size());
    bounds.Position.Y += barHeight * (MaximizeActiveSection() ? 1 : ActiveSectionIndex() + 1);
}

void accordion::offset_content(rect_f& bounds, bool isHitTest) const
{
    widget::offset_content(bounds, isHitTest);

    if (isHitTest) { return; }
    offset_section_content(bounds, _style);
}

auto accordion::attributes() const -> widget_attributes
{
    auto retValue {widget_container::attributes()};

    retValue["active_index"] = ActiveSectionIndex();
    if (ActiveSectionIndex >= 0 && ActiveSectionIndex < std::ssize(_sections)) {
        retValue["active"] = _sectionLabels[ActiveSectionIndex].Text;
    }
    retValue["hover_index"] = HoveredSectionIndex();
    if (HoveredSectionIndex >= 0 && HoveredSectionIndex < std::ssize(_sections)) {
        retValue["hover"] = _sectionLabels[HoveredSectionIndex].Text;
    }

    return retValue;
}

} // namespace ui
