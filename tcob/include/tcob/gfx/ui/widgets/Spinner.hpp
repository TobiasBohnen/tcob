// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <utility>

#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Stopwatch.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/TextEdit.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API spinner : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        utf8_string NavArrowClass {"nav_arrows"};

        text_element  Text;
        caret_element Caret;

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit spinner(init const& wi);

    prop_chk<f32> Min;
    prop_chk<f32> Max;
    prop<f32>     Step;
    prop_chk<f32> Value;

    prop<i32> Precision;

protected:
    void on_draw(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_key_up(input::keyboard::event const& ev) override;
    void on_controller_button_down(input::controller::button_event const& ev) override;
    void on_text_input(input::keyboard::text_input_event const& ev) override;

    void on_focus_lost() override;

    void on_update(milliseconds deltaTime) override;

    auto attributes() const -> widget_attributes override;

private:
    void commit_edit();
    void cancel_edit();
    void enter_edit_mode();

    enum class arrow : u8 {
        None,
        Increase,
        Decrease
    };

    text_edit                   _edit;
    gfx::text_formatter::result _formatResult;
    bool                        _needsFormat {false};
    bool                        _editing {false};

    arrow                     _hoverArrow {arrow::None};
    bool                      _mouseDown {false};
    float                     _holdCount {1};
    stopwatch                 _holdTime;
    std::pair<rect_f, rect_f> _rectCache;

    spinner::style _style;
};

}
