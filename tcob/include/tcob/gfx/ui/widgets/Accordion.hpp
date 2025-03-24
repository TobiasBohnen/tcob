// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/gfx/ui/WidgetTweener.hpp"
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API accordion : public widget_container {
public:
    class TCOB_API style : public widget_style {
    public:
        length       SectionBarHeight;
        utf8_string  SectionItemClass {"section_items"};
        milliseconds Delay {0};

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit accordion(init const& wi);

    prop_val<isize> ActiveSectionIndex;
    prop_val<isize> HoveredSectionIndex;
    prop<bool>      MaximizeActiveSection;

    template <std::derived_from<widget_container> T>
    auto create_section(utf8_string const& name) -> std::shared_ptr<T>;
    template <std::derived_from<widget_container> T>
    auto create_section(utf8_string const& name, list_item const& label) -> std::shared_ptr<T>;

    void remove_section(widget* sec);
    void clear_sections();

    void change_section_label(widget* sec, utf8_string const& label);
    void change_section_label(widget* sec, list_item const& label);

    auto find_child_at(point_f pos) -> std::shared_ptr<widget> override;

    auto widgets() const -> std::vector<std::shared_ptr<widget>> const& override;

protected:
    void on_prepare_redraw() override;

    void on_draw(widget_painter& painter) override;
    void on_draw_children(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;
    void on_animation_step(string const& val) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto attributes() const -> widget_attributes override;

private:
    void offset_section_content(rect_f& bounds, style const& style) const;

    std::vector<std::shared_ptr<widget>> _sections;
    std::vector<list_item>               _sectionLabels;
    std::vector<rect_f>                  _sectionRectCache;

    accordion::style _style;
    widget_tweener   _tween;
};

}

#include "Accordion.inl"
