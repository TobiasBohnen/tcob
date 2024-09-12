// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input_Codes.hpp"

////////////////////////////////////////////////////////////

extern "C" {
using SDL_GameController = struct _SDL_GameController;
using SDL_Event          = union SDL_Event;
}

////////////////////////////////////////////////////////////

namespace tcob::input {
////////////////////////////////////////////////////////////

enum class mode : u8 {
    KeyboardMouse,
    Controller
};

////////////////////////////////////////////////////////////

enum class key_mod {
    None         = 0x0000,
    LeftShift    = 0x0001,
    RightShift   = 0x0002,
    LeftControl  = 0x0040,
    RightControl = 0x0080,
    LeftAlt      = 0x0100,
    RightAlt     = 0x0200,
    LeftGui      = 0x0400,
    RightGui     = 0x0800,
    NumLock      = 0x1000,
    CapsLock     = 0x2000,
    Mode         = 0x4000,
    Control      = LeftControl | RightControl,
    Shift        = LeftShift | RightShift,
    Alt          = LeftAlt | RightAlt,
    Gui          = LeftGui | RightGui
};

////////////////////////////////////////////////////////////

class TCOB_API keyboard {
public:
    struct event {
        bool      Handled {false};
        bool      Pressed {false};
        bool      Repeat {false};
        scan_code ScanCode {scan_code::UNKNOWN};
        key_code  KeyCode {key_code::UNKNOWN};
        key_mod   KeyMods {key_mod::None};
    };

    struct text_input_event {
        bool        Handled {false};
        utf8_string Text {};
    };

    struct text_editing_event {
        bool        Handled {false};
        utf8_string Text {};
        i32         Start {0};
        i32         Length {0};
    };

    auto get_scancode(key_code key) const -> scan_code;
    auto get_keycode(scan_code key) const -> key_code;

    auto is_key_down(scan_code key) const -> bool;
    auto is_mod_down(key_mod mod) const -> bool;

    auto get_mod_state() const -> std::unordered_map<key_mod, bool>;
};

////////////////////////////////////////////////////////////

class TCOB_API mouse {
public:
    enum class button : u8 {
        None   = 0,
        Left   = 1,
        Middle = 2,
        Right  = 3,
        X1     = 4,
        X2     = 5
    };

    struct motion_event {
        bool    Handled {false};
        point_i Position {point_i::Zero};
        point_i RelativeMotion {point_i::Zero};
    };

    struct button_event {
        bool    Handled {false};
        button  Button {button::None};
        bool    Pressed {false};
        u8      Clicks {0};
        point_i Position {point_i::Zero};
    };

    struct wheel_event {
        bool    Handled {false};
        point_i Scroll {point_i::Zero};
        point_f Precise {point_f::Zero};
        point_i Position {point_i::Zero};
    };

    auto get_position() const -> point_i;
    void set_position(point_i pos) const;
    auto is_button_down(button button) const -> bool;
};

////////////////////////////////////////////////////////////

class TCOB_API controller final {
    friend class system;

public:
    enum class button {
        Invalid = -1,
        A,
        B,
        X,
        Y,
        Back,
        Guide,
        Start,
        LeftStick,
        RightStick,
        LeftShoulder,
        RightShoulder,
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,
        Misc1,
        Paddle1,
        Paddle2,
        Paddle3,
        Paddle4,
        Touchpad
    };

    enum class axis {
        Invalid = -1,
        LeftX,
        LeftY,
        RightX,
        RightY,
        TriggerLeft,
        TriggerRight,
    };

    struct button_event {
        bool                        Handled {false};
        i32                         ID {0};
        std::shared_ptr<controller> Controller;
        button                      Button {button::Invalid};
        bool                        Pressed {false};
    };

    struct axis_event {
        bool                        Handled {false};
        i32                         ID {0};
        std::shared_ptr<controller> Controller;
        axis                        Axis {axis::Invalid};
        i16                         Value {0};
        f32                         RelativeValue {0};
    };

    auto get_ID() const -> i32;
    auto get_name() const -> string;
    auto has_rumble() const -> bool;
    auto has_rumble_triggers() const -> bool;

    auto rumble(u16 lowFrequencyRumble, u16 highFrequencyRumble, milliseconds duration) const -> bool;
    auto rumble_triggers(u16 leftRumble, u16 rightRumble, milliseconds duration) const -> bool;

    auto is_button_pressed(button b) const -> bool;
    auto has_button(button b) const -> bool;
    auto get_button_name(button b) const -> string;

    auto get_axis_value(axis a) const -> i16;
    auto has_axis(axis a) const -> bool;
    auto get_axis_name(axis a) const -> string;

private:
    controller(SDL_GameController* controller, i32 id);

    SDL_GameController* _controller;
    i32                 _id;
};

////////////////////////////////////////////////////////////

class TCOB_API joystick {
    friend class system;

public:
    enum class hat : u8 {
        Centered  = 0x00,
        Up        = 0x01,
        Right     = 0x02,
        Down      = 0x04,
        Left      = 0x08,
        RightUp   = Right | Up,
        RightDown = Right | Down,
        LeftUp    = Left | Up,
        LeftDown  = Left | Down,
    };

    struct hat_event {
        bool Handled {false};
        i32  ID {0};
        hat  Hat {hat::Centered};
        u8   Value {0};
    };

    struct button_event {
        bool Handled {false};
        i32  ID {0};
        u8   Button {0};
        bool Pressed {false};
    };

    struct axis_event {
        bool Handled {false};
        i32  ID {0};
        u8   Axis {0};
        i16  Value {0};
    };
};

////////////////////////////////////////////////////////////

class TCOB_API system final {
public:
    system();
    ~system();

    signal<keyboard::event>              KeyDown;
    signal<keyboard::event>              KeyUp;
    signal<keyboard::text_input_event>   TextInput;
    signal<keyboard::text_editing_event> TextEditing;

    signal<mouse::motion_event> MouseMotion;
    signal<mouse::button_event> MouseButtonDown;
    signal<mouse::button_event> MouseButtonUp;
    signal<mouse::wheel_event>  MouseWheel;

    signal<joystick::axis_event>   JoystickAxisMotion;
    signal<joystick::hat_event>    JoystickHatMotion;
    signal<joystick::button_event> JoystickButtonDown;
    signal<joystick::button_event> JoystickButtonUp;
    signal<i32>                    JoystickAdded;
    signal<i32>                    JoystickRemoved;

    signal<controller::axis_event>   ControllerAxisMotion;
    signal<controller::button_event> ControllerButtonDown;
    signal<controller::button_event> ControllerButtonUp;
    signal<i32>                      ControllerAdded;
    signal<i32>                      ControllerRemoved;

    prop<mode> CurrentInputMode;

    auto get_controller_count() const -> isize;
    auto get_controller(i32 index) const -> std::shared_ptr<controller>;

    auto get_mouse() const -> mouse;
    auto get_keyboard() const -> keyboard;

    void process_events(SDL_Event* ev);

    auto static IsKeyDown(scan_code key) -> bool;
    auto static IsKeyDown(key_code key) -> bool;
    auto static IsKeyModDown(key_mod mod) -> bool;
    auto static IsMouseButtonDown(mouse::button button) -> bool;
    auto static GetMousePosition() -> point_i;
    void static SetMousePosition(point_i pos);

    auto static GetClipboardText() -> utf8_string;
    void static SetClipboardText(utf8_string const& text);

    static inline char const* service_name {"input_system"};

private:
    std::unordered_map<i32, std::shared_ptr<controller>> _controllers;
};

////////////////////////////////////////////////////////////

class TCOB_API receiver : public non_copyable {
public:
    receiver()          = default;
    virtual ~receiver() = default;

    void virtual on_key_down(keyboard::event& ev)                        = 0;
    void virtual on_key_up(keyboard::event& ev)                          = 0;
    void virtual on_text_input(keyboard::text_input_event& ev)           = 0;
    void virtual on_text_editing(keyboard::text_editing_event& ev)       = 0;
    void virtual on_mouse_motion(mouse::motion_event& ev)                = 0;
    void virtual on_mouse_button_down(mouse::button_event& ev)           = 0;
    void virtual on_mouse_button_up(mouse::button_event& ev)             = 0;
    void virtual on_mouse_wheel(mouse::wheel_event& ev)                  = 0;
    void virtual on_controller_axis_motion(controller::axis_event& ev)   = 0;
    void virtual on_controller_button_down(controller::button_event& ev) = 0;
    void virtual on_controller_button_up(controller::button_event& ev)   = 0;
};

////////////////////////////////////////////////////////////

}
