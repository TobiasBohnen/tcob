// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <unordered_map>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input_Codes.hpp"

////////////////////////////////////////////////////////////

extern "C" {
struct SDL_Gamepad;
using SDL_Event = union SDL_Event;
}

////////////////////////////////////////////////////////////

namespace tcob::input {
////////////////////////////////////////////////////////////

enum class mode : u8 {
    KeyboardMouse,
    Controller
};

////////////////////////////////////////////////////////////

enum class key_mod : u16 {
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
    Scroll       = 0x8000,
    Control      = LeftControl | RightControl,
    Shift        = LeftShift | RightShift,
    Alt          = LeftAlt | RightAlt,
    Gui          = LeftGui | RightGui
};

////////////////////////////////////////////////////////////

class TCOB_API keyboard {
public:
    struct event : event_base {
        keyboard* Keyboard {nullptr};

        bool      Pressed {false};
        bool      Repeat {false};
        scan_code ScanCode {scan_code::UNKNOWN};
        key_mod   KeyMods {key_mod::None};
        key_code  KeyCode {key_code::UNKNOWN};
    };

    struct text_input_event : event_base {
        utf8_string Text {};
    };

    auto get_scancode(key_code key) const -> scan_code;
    auto get_keycode(scan_code key) const -> key_code;

    auto is_key_down(scan_code key) const -> bool;
    auto is_key_down(key_code key) const -> bool;
    auto is_mod_down(key_mod mod) const -> bool;

    auto mod_state() const -> std::unordered_map<key_mod, bool>;
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

    struct motion_event : event_base {
        mouse* Mouse {nullptr};

        point_i Position {point_i::Zero};
        point_i RelativeMotion {point_i::Zero};
    };

    struct button_event : event_base {
        mouse* Mouse {nullptr};

        button  Button {button::None};
        bool    Pressed {false};
        u8      Clicks {0};
        point_i Position {point_i::Zero};
    };

    struct wheel_event : event_base {
        mouse* Mouse {nullptr};

        point_f Scroll {point_f::Zero};
        point_i Position {point_i::Zero};
    };

    auto get_position() const -> point_i; // TODO: set_get_
    void set_position(point_i pos) const;
    auto is_button_down(button button) const -> bool;
};

////////////////////////////////////////////////////////////

class TCOB_API controller final {
    friend class system;

public:
    enum class button : i8 {
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

    enum class axis : i8 {
        Invalid = -1,
        LeftX,
        LeftY,
        RightX,
        RightY,
        TriggerLeft,
        TriggerRight,
    };

    struct button_event : event_base {
        u32         ID {0};
        controller* Controller {nullptr};
        button      Button {button::Invalid};
        bool        Pressed {false};
    };

    struct axis_event : event_base {
        u32         ID {0};
        controller* Controller {nullptr};
        axis        Axis {axis::Invalid};
        i16         Value {0};
        f32         RelativeValue {0};
    };

    auto id() const -> u32;
    auto name() const -> string;

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
    controller(SDL_Gamepad* controller, u32 id);

    SDL_Gamepad* _controller;
    u32          _id;
};

////////////////////////////////////////////////////////////

class TCOB_API clipboard {
public:
    auto has_text() const -> bool;
    auto get_text() const -> utf8_string; // TODO: set_get_
    void set_text(utf8_string const& text);
};

////////////////////////////////////////////////////////////

class TCOB_API system final {
public:
    system();
    ~system();

    signal<input::keyboard::event const>            KeyDown;
    signal<input::keyboard::event const>            KeyUp;
    signal<input::keyboard::text_input_event const> TextInput;

    signal<input::mouse::motion_event const> MouseMotion;
    signal<input::mouse::button_event const> MouseButtonDown;
    signal<input::mouse::button_event const> MouseButtonUp;
    signal<input::mouse::wheel_event const>  MouseWheel;

    signal<controller::axis_event const>   ControllerAxisMotion;
    signal<controller::button_event const> ControllerButtonDown;
    signal<controller::button_event const> ControllerButtonUp;
    signal<i32 const>                      ControllerAdded;
    signal<i32 const>                      ControllerRemoved;

    prop<mode> InputMode;

    auto controllers() const -> std::unordered_map<i32, std::shared_ptr<controller>> const&;
    auto first_controller() const -> controller&;
    auto has_controller() const -> bool;

    auto mouse() const -> input::mouse;
    auto keyboard() const -> input::keyboard;
    auto clipboard() const -> input::clipboard;

    void process_events(SDL_Event* ev);

    static inline char const* service_name {"input_system"};

private:
    std::unordered_map<i32, std::shared_ptr<controller>> _controllers;
};

////////////////////////////////////////////////////////////

class TCOB_API receiver : public non_copyable {
public:
    receiver()          = default;
    virtual ~receiver() = default;

    void virtual on_key_down(keyboard::event const& ev)                        = 0;
    void virtual on_key_up(keyboard::event const& ev)                          = 0;
    void virtual on_text_input(keyboard::text_input_event const& ev)           = 0;
    void virtual on_mouse_motion(mouse::motion_event const& ev)                = 0;
    void virtual on_mouse_button_down(mouse::button_event const& ev)           = 0;
    void virtual on_mouse_button_up(mouse::button_event const& ev)             = 0;
    void virtual on_mouse_wheel(mouse::wheel_event const& ev)                  = 0;
    void virtual on_controller_axis_motion(controller::axis_event const& ev)   = 0;
    void virtual on_controller_button_down(controller::button_event const& ev) = 0;
    void virtual on_controller_button_up(controller::button_event const& ev)   = 0;
};

////////////////////////////////////////////////////////////

}
