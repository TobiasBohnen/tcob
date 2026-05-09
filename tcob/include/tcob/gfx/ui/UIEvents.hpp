// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

struct widget_event {
    widget* Sender {nullptr};
};

struct keyboard_event {
    widget*                       Sender {nullptr};
    input::keyboard::event const& Event;
};

struct mouse_button_event {
    widget*                           Sender {nullptr};
    point_i                           RelativePosition {point_i::Zero};
    input::mouse::button_event const& Event;
};

struct mouse_motion_event {
    widget*                           Sender {nullptr};
    point_i                           RelativePosition {point_i::Zero};
    input::mouse::motion_event const& Event;
};

struct mouse_wheel_event {
    widget*                          Sender {nullptr};
    input::mouse::wheel_event const& Event;
};

struct controller_button_event {
    widget*                                Sender {nullptr};
    input::controller::button_event const& Event;
};

struct text_event {
    widget*     Sender {nullptr};
    utf8_string Text;
};

struct drop_event {
    widget* Sender {nullptr};
    widget* Target {nullptr};
    point_i Position {point_i::Zero};
};

}
