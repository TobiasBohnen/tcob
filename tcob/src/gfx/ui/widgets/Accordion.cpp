// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Accordion.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

accordion::accordion(init const& wi)
    : widget_container {wi}
    , ActiveSectionIndex {{[&](isize val) -> isize { return std::clamp<isize>(val, -1, std::ssize(_sections) - 1); }}}
    , HoveredSectionIndex {{[&](isize val) -> isize { return std::clamp<isize>(val, -1, std::ssize(_sections) - 1); }}}
{
    ActiveSectionIndex.Changed.connect([&](auto const&) { force_redraw(this->name() + ": ActiveSection changed"); });
    ActiveSectionIndex(-1);
    HoveredSectionIndex.Changed.connect([&](auto const&) { force_redraw(this->name() + ": HoveredSection changed"); });
    HoveredSectionIndex(-1);
    MaximizeActiveSection.Changed.connect([&](auto const&) { force_redraw(this->name() + ": MaximizeActiveSection changed"); });
    MaximizeActiveSection(false);

    Class("accordion");
}

void accordion::remove_section(widget* sec)
{
    for (isize i {0}; i < std::ssize(_sections); ++i) {
        if (_sections[i].get() == sec) {
            _sections.erase(_sections.begin() + i);
            _sectionLabels.erase(_sectionLabels.begin() + i);
            break;
        }
    }
    ActiveSectionIndex = 0;
    force_redraw(this->name() + ": section removed");
}

void accordion::clear_sections()
{
    _sections.clear();
    _sectionLabels.clear();
    ActiveSectionIndex = 0;
    force_redraw(this->name() + ": sections cleared");
}

void accordion::change_section_label(widget* tab, utf8_string const& label)
{
    for (isize i {0}; i < std::ssize(_sections); ++i) {
        if (_sections[i].get() == tab) {
            _sectionLabels[i] = label;
            break;
        }
    }
    force_redraw(this->name() + ": section renamed");
}

auto accordion::find_child_at(point_f pos) -> std::shared_ptr<widget>
{
    if (ActiveSectionIndex >= 0 && ActiveSectionIndex < std::ssize(_sections)) {
        auto& activeSection {_sections[ActiveSectionIndex()]};
        if (activeSection->hit_test(pos)) {
            if (auto retValue {activeSection->find_child_at(pos)}) {
                return retValue;
            }
            return activeSection;
        }
    }

    return nullptr;
}

auto accordion::widgets() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _sections;
}

void accordion::on_paint(widget_painter& painter)
{
    if (auto const* style {current_style<accordion::style>()}) {
        rect_f rect {Bounds()};
        // TODO: section chevron
        //  background
        painter.draw_background_and_border(*style, rect, false);

        // sections
        f32 const sectionHeight {style->SectionBarHeight.calc(rect.height())};
        _sectionRects.clear();
        if (MaximizeActiveSection() && ActiveSectionIndex >= 0) {
            isize const i {ActiveSectionIndex()};
            if (auto const* sectionStyle {get_section_style(i)}) {
                rect_f const sectionRect {get_section_rect(*sectionStyle, 0, sectionHeight, rect)};
                painter.draw_item(sectionStyle->Item, sectionRect, _sectionLabels[i]);
                _sectionRects.push_back(sectionRect);
            }
        } else {
            for (isize i {0}; i < std::ssize(_sections); ++i) {
                if (auto const* sectionStyle {get_section_style(i)}) {
                    rect_f const sectionRect {get_section_rect(*sectionStyle, i, sectionHeight, rect)};
                    painter.draw_item(sectionStyle->Item, sectionRect, _sectionLabels[i]);
                    _sectionRects.push_back(sectionRect);
                }
            }
        }

        // content
        scissor_guard const guard {painter, this};

        // active section
        if (ActiveSectionIndex >= 0 && ActiveSectionIndex < std::ssize(_sections)) {
            offset_section_content(rect, *style);
            update_section_bounds(rect);

            auto          xform {transform::Identity};
            point_f const translate {rect.Position + paint_offset()};
            xform.translate(translate);

            auto& tab {_sections[ActiveSectionIndex()]};
            painter.begin(Alpha(), xform);
            tab->paint(painter);
            painter.end();
        }
    }
}

void accordion::on_mouse_leave()
{
    widget::on_mouse_leave();

    HoveredSectionIndex = -1;
}

void accordion::on_mouse_hover(input::mouse::motion_event const& ev)
{
    HoveredSectionIndex = -1;

    widget_container::on_mouse_hover(ev);

    auto const mp {global_to_parent_local(ev.Position)};
    for (i32 i {0}; i < std::ssize(_sectionRects); ++i) {
        if (_sectionRects[i].contains(mp)) {
            if (MaximizeActiveSection() && ActiveSectionIndex >= 0) {
                HoveredSectionIndex = ActiveSectionIndex();
            } else {
                HoveredSectionIndex = i;
            }

            ev.Handled = true;
            break;
        }
    }
}

void accordion::on_mouse_down(input::mouse::button_event const& ev)
{
    widget::on_mouse_down(ev);

    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        force_redraw(this->name() + ": mouse down");

        if (HoveredSectionIndex >= 0) {
            if (ActiveSectionIndex == HoveredSectionIndex) {
                ActiveSectionIndex = -1;
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
}

auto accordion::get_section_rect(item_style const& itemStyle, isize index, f32 sectionHeight, rect_f const& rect) const -> rect_f
{
    rect_f retValue {rect};
    retValue.Position.Y += sectionHeight * index;
    retValue.Size.Height = sectionHeight;
    retValue -= itemStyle.Item.Border.thickness();
    if (ActiveSectionIndex >= 0 && index > ActiveSectionIndex) {
        retValue.Position.Y += content_bounds().height();
    }
    return retValue;
}

auto accordion::get_section_style(isize index) const -> item_style*
{
    auto const* style {current_style<accordion::style>()};
    return index == ActiveSectionIndex ? get_sub_style<item_style>(style->SectionItemClass, {.Active = true})
        : index == HoveredSectionIndex ? get_sub_style<item_style>(style->SectionItemClass, {.Hover = true})
                                       : get_sub_style<item_style>(style->SectionItemClass, {});
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

    if (!isHitTest) {
        if (auto const* style {current_style<accordion::style>()}) {
            offset_section_content(bounds, *style);
        }
    }
}

void accordion::update_section_bounds(rect_f const& bounds)
{
    for (auto& t : _sections) {
        t->Bounds = {point_f::Zero, bounds.Size};
    }
}

} // namespace ui
