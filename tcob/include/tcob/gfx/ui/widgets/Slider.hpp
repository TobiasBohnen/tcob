// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/WidgetTweener.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API slider : public widget {
public:
    class TCOB_API style : public ui::style {
    public:
        element::bar Bar;
        string       ThumbClass {"slider_thumb"};
    };

    explicit slider(init const& wi);

    prop_val<i32> Min;
    prop_val<i32> Max;
    prop<i32>     Step;
    prop_val<i32> Value;

    bool IncrementalChange {false};

protected:
    void on_paint(widget_painter& painter) override;

    void on_key_down(input::keyboard::event& ev) override;
    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event& ev) override;
    void on_mouse_drag(input::mouse::motion_event& ev) override;
    void on_mouse_up(input::mouse::button_event& ev) override;
    void on_mouse_down(input::mouse::button_event& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event& ev) override;
    void on_controller_button_down(input::controller::button_event& ev) override;

    void on_update(milliseconds deltaTime) override;

    void virtual on_value_changed(i32 newVal);

    auto get_attributes() const -> widget_attributes override;

private:
    void handle_dir_input(direction dir);

    void calculate_value(point_f mp);

    bool                      _overThumb {false};
    bool                      _isDragging {false};
    point_i                   _dragOffset {point_i::Zero};
    element::scrollbar::rects _paintResult {};

    widget_tweener _tween;
};
}
