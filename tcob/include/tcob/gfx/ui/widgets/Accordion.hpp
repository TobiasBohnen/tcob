// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API accordion : public widget_container {
public:
    class TCOB_API style : public background_style {
    public:
        length SectionBarHeight;
        string SectionItemClass {"section_items"};
    };

    explicit accordion(init const& wi);

    prop_val<isize> ActiveSectionIndex;
    prop_val<isize> HoveredSectionIndex;

    template <std::derived_from<widget_container> T>
    auto create_section(utf8_string const& name) -> std::shared_ptr<T>;
    template <std::derived_from<widget_container> T>
    auto create_section(utf8_string const& name, utf8_string const& label) -> std::shared_ptr<T>;

    auto find_child_at(point_f pos) -> std::shared_ptr<widget> override;

    auto get_widgets() const -> std::vector<std::shared_ptr<widget>> const& override;

protected:
    void on_paint(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event& ev) override;
    void on_mouse_down(input::mouse::button_event& ev) override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

private:
    auto get_section_rect(isize index, f32 sectionHeight, rect_f const& rect) const -> rect_f;
    auto get_section_style(isize index) const -> std::shared_ptr<item_style>;

    void offset_section_content(rect_f& bounds, style const& style) const;

    void update_section_bounds(rect_f const& bounds);

    std::vector<std::shared_ptr<widget>> _sections;
    std::vector<utf8_string>             _sectionLabels;
    std::vector<rect_f>                  _sectionRects;
};

}

#include "Accordion.inl"
