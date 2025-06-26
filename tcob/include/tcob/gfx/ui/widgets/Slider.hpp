// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/core/Rect.hpp"
#include "tcob/tcob_config.hpp"

#include <utility>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Scrollbar.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/WidgetTweener.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API slider : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        bar_element Bar;
        utf8_string ThumbClass {"slider_thumb"};

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit slider(init const& wi);

    prop_val<i32> Min;
    prop_val<i32> Max;
    prop<i32>     Step;
    prop_val<i32> Value;

    bool IncrementalChange {false};

protected:
    void on_draw(widget_painter& painter) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;
    void on_controller_button_down(input::controller::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

    void on_value_changed(i32 newVal);

    auto attributes() const -> widget_attributes override;

private:
    void handle_dir_input(direction dir);

    void calculate_value(point_f mp);

    bool             _overThumb {false};
    bool             _isDragging {false};
    point_i          _dragOffset {point_i::Zero};
    scrollbar::rects _barRectCache {};

    widget_tweener _tween;

    slider::style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API range_slider : public widget {
public:
    class TCOB_API style : public slider::style {
    };

    explicit range_slider(init const& wi);

    prop_val<i32>                 Min;
    prop_val<i32>                 Max;
    prop<i32>                     Step;
    prop_val<std::pair<i32, i32>> Values;

    bool IncrementalChange {false};

protected:
    void on_draw(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

    void on_value_changed(std::pair<i32, i32> newVal);

    auto attributes() const -> widget_attributes override;

private:
    struct thumb {
        bool           Over {false};
        bool           IsDragging {false};
        rect_f         Rect;
        widget_tweener Tween;
    };

    void calculate_value(thumb& thumb, point_f mp);

    thumb   _min;
    thumb   _max;
    point_i _dragOffset {point_i::Zero};
    rect_f  _barRectCache {};

    range_slider::style _style;
};

}
