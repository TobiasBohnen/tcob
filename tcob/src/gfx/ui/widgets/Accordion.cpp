// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Accordion.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>
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
    , ActiveSectionIndex {{[this](isize val) -> isize {
        return std::clamp<isize>(val, INVALID_INDEX, std::ssize(_sections) - 1);
    }}}
    , HoveredSectionIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(_sections) - 1); }}}
{
    _expandTween.Changed.connect([&]() {
        queue_redraw();
    });

    ActiveSectionIndex.Changed.connect([this](auto const& val) {
        if (MaximizeActiveSection) {
            _expandTween.reset(1);
        } else {
            _expandTween.reset(0);
            auto const duration {val != INVALID_INDEX && _oldActiveSectionIndex != INVALID_INDEX
                                     ? _style.ExpandDuration * 2
                                     : _style.ExpandDuration};
            _expandTween.start(1, duration);
        }
        queue_redraw();
    });
    ActiveSectionIndex(INVALID_INDEX);

    HoveredSectionIndex.Changed.connect([this](auto const&) { queue_redraw(); });
    HoveredSectionIndex(INVALID_INDEX);

    MaximizeActiveSection.Changed.connect([this](auto const&) { queue_redraw(); });
    MaximizeActiveSection(false);

    Class("accordion");
}

void accordion::on_prepare_redraw()
{
    prepare_style(_style);

    auto const rect {content_bounds()};
    for (auto& t : _sections) {
        t->Bounds = {point_f::Zero, rect.Size};
    }

    widget_container::on_prepare_redraw();
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

    queue_redraw();
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
    queue_redraw();
}

void accordion::change_section_label(widget* sec, item const& label)
{
    for (usize i {0}; i < _sections.size(); ++i) {
        if (_sections[i].get() == sec) {
            _sectionLabels[i] = label;
            break;
        }
    }
    queue_redraw();
}

auto accordion::find_child_at(point_i pos) -> std::shared_ptr<widget>
{
    if (ActiveSectionIndex < 0 || ActiveSectionIndex >= std::ssize(_sections)) {
        return nullptr;
    }

    auto& activeSection {_sections[ActiveSectionIndex]};
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

    auto const [secIdx, val] {section_expand()};

    // sections
    f32 const  sectionHeight {_style.SectionBarHeight.calc(rect.height())};
    auto const getSectionRect {[&](item_style const& itemStyle, isize index) {
        rect_f retValue {rect};
        retValue.Position.Y += sectionHeight * static_cast<f32>(index);
        retValue.Size.Height = sectionHeight;
        retValue -= itemStyle.Item.Border.thickness();
        if (secIdx > INVALID_INDEX && index > secIdx) {
            retValue.Position.Y += content_bounds().height() * val;
        }
        return retValue;
    }};

    _sectionRectCache.clear();
    auto const paintSection {[&](isize i, isize rectIndex) {
        item_style sectionStyle {};
        prepare_sub_style(sectionStyle, i, _style.SectionItemClass, {.Active = i == ActiveSectionIndex, .Hover = i == HoveredSectionIndex});

        rect_f const sectionRect {getSectionRect(sectionStyle, rectIndex)};
        painter.draw_item(sectionStyle.Item, sectionRect, _sectionLabels[i]);
        _sectionRectCache.push_back(sectionRect);
    }};

    if (MaximizeActiveSection && ActiveSectionIndex > INVALID_INDEX) {
        paintSection(ActiveSectionIndex, 0);
    } else {
        for (isize i {0}; i < std::ssize(_sections); ++i) {
            paintSection(i, i);
        }
    }
}

void accordion::on_draw_children(widget_painter& painter)
{
    auto const [secIdx, val] {section_expand()};
    if (secIdx == INVALID_INDEX) { return; }

    // prepare_style(_style);

    // scissor
    rect_f bounds {global_content_bounds()};
    bounds.Position -= form().Bounds->Position;
    bounds.Size.Height *= val;
    painter.push_scissor(bounds);

    // content
    auto xform {gfx::transform::Identity};
    xform.translate(bounds.Position);

    auto& sec {_sections[secIdx]};
    painter.begin(Alpha, xform);
    sec->draw(painter);
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

        if (MaximizeActiveSection && ActiveSectionIndex >= 0) {
            HoveredSectionIndex = *ActiveSectionIndex;
        } else {
            HoveredSectionIndex = i;
        }

        ev.Handled = true;
        break;
    }
}

void accordion::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button == controls().PrimaryMouseButton) {
        if (HoveredSectionIndex >= 0) {
            _oldActiveSectionIndex = ActiveSectionIndex;
            if (ActiveSectionIndex == HoveredSectionIndex) {
                ActiveSectionIndex = INVALID_INDEX;
                if (MaximizeActiveSection) { HoveredSectionIndex = 0; }
            } else {
                ActiveSectionIndex = *HoveredSectionIndex;
            }
        }

        ev.Handled = true;
    }
}

void accordion::on_update(milliseconds deltaTime)
{
    _expandTween.update(deltaTime);
}

void accordion::on_animation_step(string const& val)
{
    if (ActiveSectionIndex >= 0) {
        auto& sec {_sectionLabels[ActiveSectionIndex]};
        sec.Icon.TextureRegion = val;
        if (sec.Icon.Texture) {
            queue_redraw();
        }
    }
}

void accordion::offset_section_content(rect_f& bounds, style const& style) const
{
    auto const [secIdx, _] {section_expand()};
    f32 const barHeight {style.SectionBarHeight.calc(bounds.height())};
    bounds.Size.Height -= barHeight * (MaximizeActiveSection ? 1 : static_cast<f32>(_sections.size()));
    bounds.Position.Y += barHeight * (MaximizeActiveSection ? 1 : static_cast<f32>(secIdx + 1));
}

auto accordion::section_expand() const -> std::pair<isize, f32>
{
    std::pair<isize, f32> retValue;
    f32 const             val {_expandTween.current_value()};
    if (_oldActiveSectionIndex == INVALID_INDEX) {
        retValue.first  = ActiveSectionIndex;
        retValue.second = val;
    } else if (ActiveSectionIndex == INVALID_INDEX) {
        retValue.first  = _oldActiveSectionIndex;
        retValue.second = 1 - val;
    } else {
        if (val >= 0.5f) {
            retValue.first  = ActiveSectionIndex;
            retValue.second = (val - 0.5f) * 2;
        } else {
            retValue.first  = _oldActiveSectionIndex;
            retValue.second = 1 - (val * 2.f);
        }
    }
    if (retValue.first < 0 || retValue.first >= std::ssize(_sections)) { return {INVALID_INDEX, 0}; }
    return retValue;
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

    retValue["active_index"] = *ActiveSectionIndex;
    if (ActiveSectionIndex >= 0 && ActiveSectionIndex < std::ssize(_sections)) {
        retValue["active"] = _sectionLabels[ActiveSectionIndex].Text;
    }
    retValue["hover_index"] = *HoveredSectionIndex;
    if (HoveredSectionIndex >= 0 && HoveredSectionIndex < std::ssize(_sections)) {
        retValue["hover"] = _sectionLabels[HoveredSectionIndex].Text;
    }

    return retValue;
}

} // namespace ui
