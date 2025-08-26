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
#include "tcob/core/TypeFactory.hpp"
#include "tcob/core/input/Input_Codes.hpp"

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

class TCOB_API key_mods {
public:
    explicit key_mods(key_mod mod);

    auto num_lock() const -> bool;
    auto caps_lock() const -> bool;
    auto mode() const -> bool;
    auto scroll() const -> bool;

    auto control() const -> bool;
    auto left_control() const -> bool;
    auto right_control() const -> bool;

    auto shift() const -> bool;
    auto left_shift() const -> bool;
    auto right_shift() const -> bool;

    auto alt() const -> bool;
    auto left_alt() const -> bool;
    auto right_alt() const -> bool;

    auto gui() const -> bool;
    auto left_gui() const -> bool;
    auto right_gui() const -> bool;

    auto is_down(key_mod mod) const -> bool;

private:
    key_mod _mod;
};

////////////////////////////////////////////////////////////

class TCOB_API keyboard {
public:
    struct event : event_base {
        keyboard* Keyboard {nullptr};

        bool      Pressed {false};
        bool      Repeat {false};
        scan_code ScanCode {scan_code::UNKNOWN};
        key_mods  KeyMods {key_mod::None};
        key_code  KeyCode {key_code::UNKNOWN};
    };

    struct text_input_event : event_base {
        utf8_string Text {};
    };

    virtual ~keyboard() = default;

    auto virtual get_scancode(key_code key) const -> scan_code = 0;
    auto virtual get_keycode(scan_code key) const -> key_code  = 0;

    auto virtual is_key_down(scan_code key) const -> bool = 0;
    auto virtual is_key_down(key_code key) const -> bool  = 0;
    auto virtual is_mod_down(key_mod mod) const -> bool   = 0;

    auto virtual mods() const -> key_mods = 0;
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

    virtual ~mouse() = default;

    auto virtual get_position() const -> point_i             = 0; // TODO: set_get_
    void virtual set_position(point_i pos) const             = 0;
    auto virtual is_button_down(button button) const -> bool = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API controller {
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
        Misc2,
        Misc3,
        Misc4,
        Misc5,
        Misc6,
        RightPaddle1,
        LeftPaddle1,
        RightPaddle2,
        LeftPaddle2,
        Touchpad
    };

    enum class button_label : i8 {
        Invalid = -1,
        A,
        B,
        X,
        Y,
        Cross,
        Circle,
        Square,
        Triangle
    };

    enum class axis : i8 {
        Invalid = -1,
        LeftX,
        LeftY,
        RightX,
        RightY,
        LeftTrigger,
        RightTrigger,
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

    virtual ~controller() = default;

    auto virtual id() const -> u32      = 0;
    auto virtual name() const -> string = 0;

    auto virtual has_rumble() const -> bool                                                                   = 0;
    auto virtual rumble(u16 lowFrequencyRumble, u16 highFrequencyRumble, milliseconds duration) const -> bool = 0;

    auto virtual has_rumble_triggers() const -> bool                                                   = 0;
    auto virtual rumble_triggers(u16 leftRumble, u16 rightRumble, milliseconds duration) const -> bool = 0;

    auto virtual is_button_pressed(button b) const -> bool        = 0;
    auto virtual has_button(button b) const -> bool               = 0;
    auto virtual get_button_name(button b) const -> string        = 0;
    auto virtual get_button_label(button b) const -> button_label = 0;

    auto virtual get_axis_value(axis a) const -> i16   = 0;
    auto virtual has_axis(axis a) const -> bool        = 0;
    auto virtual get_axis_name(axis a) const -> string = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API clipboard {
public:
    virtual ~clipboard() = default;

    auto virtual has_text() const -> bool          = 0;
    auto virtual get_text() const -> utf8_string   = 0; // TODO: set_get_
    void virtual set_text(utf8_string const& text) = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API system {
public:
    struct factory : public type_factory<std::shared_ptr<system>> {
        static inline char const* ServiceName {"input::system::factory"};
    };

    virtual ~system() = default;

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

    signal<> ClipboardUpdated;

    prop<mode> InputMode;

    auto virtual controllers() const -> std::unordered_map<i32, std::shared_ptr<controller>> const& = 0;
    auto first_controller() const -> controller&;
    auto has_controller() const -> bool;

    auto virtual mouse() const -> std::shared_ptr<input::mouse> = 0;

    auto virtual keyboard() const -> std::shared_ptr<input::keyboard> = 0;

    auto virtual clipboard() const -> std::shared_ptr<input::clipboard> = 0;

    void virtual process_events(void* ev) = 0;

    static inline char const* ServiceName {"input::system"};
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
