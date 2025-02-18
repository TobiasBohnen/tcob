// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Stopwatch.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API spinner : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        element::text Text;
        utf8_string   NavArrowClass {"nav_arrows"};

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit spinner(init const& wi);

    prop_val<i32> Min;
    prop_val<i32> Max;
    prop<i32>     Step;
    prop_val<i32> Value;

protected:
    void on_paint(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;
    void on_mouse_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

    auto attributes() const -> widget_attributes override;

private:
    enum class arrow : u8 {
        None,
        Increase,
        Decrease
    };

    arrow _hoverArrow {arrow::None};

    bool      _mouseDown {false};
    stopwatch _holdTime;
    f32       _holdCount {1};

    std::pair<rect_f, rect_f> _rectCache;

    spinner::style _style;
};
}
