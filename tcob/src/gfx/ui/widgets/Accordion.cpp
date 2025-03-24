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
#include "tcob/gfx/ui/Form.hpp"
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
    _tween.Changed.connect([&]() {
        request_redraw(this->name() + ": Tween value changed");
    });

    ActiveSectionIndex.Changed.connect([this](auto const&) {
        if (MaximizeActiveSection) {
            _tween.reset(1);
        } else {
            _tween.reset(0);
            _tween.start(1, _style.Delay);
        }
        request_redraw(this->name() + ": ActiveSection changed");
    });
    ActiveSectionIndex(INVALID_INDEX);

    HoveredSectionIndex.Changed.connect([this](auto const&) { request_redraw(this->name() + ": HoveredSection changed"); });
    HoveredSectionIndex(INVALID_INDEX);

    MaximizeActiveSection.Changed.connect([this](auto const&) { request_redraw(this->name() + ": MaximizeActiveSection changed"); });
    MaximizeActiveSection(false);

    Class("accordion");
}

void accordion::on_prepare_redraw()
{
    apply_style(_style);

    auto const rect {content_bounds()};
    for (auto& t : _sections) {
        t->Bounds = {point_f::Zero, rect.Size};
    }
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

    if (_sections.empty()) {
        ActiveSectionIndex = INVALID_INDEX;
    } else {
        ActiveSectionIndex = 0;
    }

    request_redraw(this->name() + ": section removed");
}

void accordion::clear_sections()
{
    while (!_sections.empty()) {
        remove_section(_sections.front().get());
    }
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

void accordion::change_section_label(widget* sec, item const& label)
{
    for (usize i {0}; i < _sections.size(); ++i) {
        if (_sections[i].get() == sec) {
            _sectionLabels[i] = label;
            break;
        }
    }
    request_redraw(this->name() + ": section label changed");
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
    rect_f rect {draw_background(_style, painter)};

    // sections
    f32 const  sectionHeight {_style.SectionBarHeight.calc(rect.height())};
    auto const getSectionRect {[&](item_style const& itemStyle, isize index) {
        rect_f retValue {rect};
        retValue.Position.Y += sectionHeight * static_cast<f32>(index);
        retValue.Size.Height = sectionHeight;
        retValue -= itemStyle.Item.Border.thickness();
        if (ActiveSectionIndex >= 0 && index > ActiveSectionIndex) {
            retValue.Position.Y += content_bounds().height() * _tween.current_value();
        }
        return retValue;
    }};

    _sectionRectCache.clear();
    auto const paintSection {[&](isize i, isize rectIndex) {
        item_style sectionStyle {};
        apply_sub_style(sectionStyle, i, _style.SectionItemClass, {.Active = i == ActiveSectionIndex, .Hover = i == HoveredSectionIndex});

        rect_f const sectionRect {getSectionRect(sectionStyle, rectIndex)};
        painter.draw_item(sectionStyle.Item, sectionRect, _sectionLabels[i]);
        _sectionRectCache.push_back(sectionRect);
    }};

    if (MaximizeActiveSection() && ActiveSectionIndex >= 0) {
        paintSection(ActiveSectionIndex(), 0);
    } else {
        for (isize i {0}; i < std::ssize(_sections); ++i) {
            paintSection(i, i);
        }
    }
}

void accordion::on_draw_children(widget_painter& painter)
{
    if (ActiveSectionIndex < 0 || ActiveSectionIndex >= std::ssize(_sections)) { return; }

    // scissor
    rect_f bounds {global_content_bounds()};
    bounds.Position -= parent_form()->Bounds->Position;
    painter.push_scissor(bounds);

    // active section
    apply_style(_style);

    rect_f rect {content_bounds()};

    // content
    auto          xform {gfx::transform::Identity};
    point_f const translate {rect.Position + paint_offset()};
    xform.translate(translate);

    auto& tab {_sections[ActiveSectionIndex()]};
    painter.begin(Alpha(), xform);
    tab->draw(painter);
    painter.end();

    painter.pop_scissor();
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

void accordion::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
}

void accordion::on_animation_step(string const& val)
{
    if (ActiveSectionIndex >= 0) {
        auto& sec {_sectionLabels[ActiveSectionIndex]};
        sec.Icon.Region = val;
        if (sec.Icon.Texture) {
            request_redraw(this->name() + ": Animation Frame changed ");
        }
    }
}

void accordion::offset_section_content(rect_f& bounds, style const& style) const
{
    f32 const barHeight {style.SectionBarHeight.calc(bounds.height())};
    bounds.Size.Height -= barHeight * (MaximizeActiveSection() ? 1 : static_cast<f32>(_sections.size()));
    bounds.Position.Y += barHeight * (MaximizeActiveSection() ? 1 : static_cast<f32>(ActiveSectionIndex() + 1));
    // bounds.Size.Height *= _tween.current_value();
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
