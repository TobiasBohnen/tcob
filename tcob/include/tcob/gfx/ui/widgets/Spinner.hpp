// Copyright (c) 2024 Tobias Bohnen
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
    class TCOB_API style : public background_style {
    public:
        element::text Text;
        string        NavArrowClass {"nav_arrows"};
    };

    explicit spinner(init const& wi);

    prop_val<i32> Min;
    prop_val<i32> Max;
    prop<i32>     Step;
    prop_val<i32> Value;

protected:
    void on_paint(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event& ev) override;
    void on_mouse_down(input::mouse::button_event& ev) override;
    void on_mouse_up(input::mouse::button_event& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event& ev) override;

    void on_update(milliseconds deltaTime) override;

    auto get_attributes() const -> widget_attributes override;

private:
    enum class arrow {
        None,
        Increase,
        Decrease
    };

    arrow _hoverArrow {arrow::None};

    bool      _mouseDown {false};
    stopwatch _holdTime;
    f32       _holdCount {1};
};
}
