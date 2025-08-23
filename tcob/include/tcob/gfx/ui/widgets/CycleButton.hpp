// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Property.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/Item.hpp"
#include "tcob/gfx/ui/component/WidgetTweener.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API cycle_button : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        utf8_string ItemClass {"items"};
        length      ItemHeight {0.8f, length::type::Relative};
        bar_element Bar;
        f32         GapRatio {1.0f};

        void static Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit cycle_button(init const& wi);

    prop_val<isize> SelectedItemIndex;

    prop<std::vector<item>> Items;

    auto select_item(utf8_string const& item) -> bool;

    auto selected_item() const -> item const&;

protected:
    void on_draw(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;
    void on_click() override;

    auto attributes() const -> widget_attributes override;

private:
    auto is_select_valid() const -> bool;

    void select_next();

    widget_tweener _tween;

    cycle_button::style _style;
};
}
