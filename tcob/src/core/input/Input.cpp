// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/input/Input.hpp"

#include <cassert>

#include <SDL.h>

namespace tcob::input {

auto static convert_enum(controller::button button) -> SDL_GameControllerButton
{
    switch (button) {
    case controller::button::Invalid:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_INVALID;
    case controller::button::A:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A;
    case controller::button::B:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B;
    case controller::button::X:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X;
    case controller::button::Y:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y;
    case controller::button::Back:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK;
    case controller::button::Guide:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_GUIDE;
    case controller::button::Start:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START;
    case controller::button::LeftStick:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK;
    case controller::button::RightStick:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK;
    case controller::button::LeftShoulder:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    case controller::button::RightShoulder:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    case controller::button::DPadUp:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP;
    case controller::button::DPadDown:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN;
    case controller::button::DPadLeft:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT;
    case controller::button::DPadRight:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
    case controller::button::Misc1:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MISC1;
    case controller::button::Paddle1:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE1;
    case controller::button::Paddle2:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE2;
    case controller::button::Paddle3:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE3;
    case controller::button::Paddle4:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE4;
    case controller::button::Touchpad:
        return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_TOUCHPAD;
    }

    return SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_INVALID;
}

auto static convert_enum(SDL_GameControllerButton button) -> controller::button
{
    switch (button) {
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_INVALID:
        return controller::button::Invalid;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A:
        return controller::button::A;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B:
        return controller::button::B;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X:
        return controller::button::X;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y:
        return controller::button::Y;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK:
        return controller::button::Back;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_GUIDE:
        return controller::button::Guide;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START:
        return controller::button::Start;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK:
        return controller::button::LeftStick;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK:
        return controller::button::RightStick;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        return controller::button::LeftShoulder;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        return controller::button::RightShoulder;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP:
        return controller::button::DPadUp;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        return controller::button::DPadDown;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        return controller::button::DPadLeft;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        return controller::button::DPadRight;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MISC1:
        return controller::button::Misc1;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE1:
        return controller::button::Paddle1;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE2:
        return controller::button::Paddle2;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE3:
        return controller::button::Paddle3;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE4:
        return controller::button::Paddle4;
    case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_TOUCHPAD:
        return controller::button::Touchpad;
    case SDL_CONTROLLER_BUTTON_MAX: break;
    }

    return controller::button::Invalid;
}

auto static convert_enum(controller::axis axis) -> SDL_GameControllerAxis
{
    switch (axis) {
    case controller::axis::Invalid:
        return SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_INVALID;
    case controller::axis::LeftX:
        return SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX;
    case controller::axis::LeftY:
        return SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY;
    case controller::axis::RightX:
        return SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX;
    case controller::axis::RightY:
        return SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY;
    case controller::axis::TriggerLeft:
        return SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT;
    case controller::axis::TriggerRight:
        return SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
    }

    return SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_INVALID;
}

auto static convert_enum(SDL_GameControllerAxis axis) -> controller::axis
{
    switch (axis) {
    case SDL_CONTROLLER_AXIS_INVALID:
        return controller::axis::Invalid;
    case SDL_CONTROLLER_AXIS_LEFTX:
        return controller::axis::LeftX;
    case SDL_CONTROLLER_AXIS_LEFTY:
        return controller::axis::LeftY;
    case SDL_CONTROLLER_AXIS_RIGHTX:
        return controller::axis::RightX;
    case SDL_CONTROLLER_AXIS_RIGHTY:
        return controller::axis::RightY;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        return controller::axis::TriggerLeft;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        return controller::axis::TriggerRight;
    case SDL_CONTROLLER_AXIS_MAX:
        return controller::axis::Invalid;
    }

    return controller::axis::Invalid;
}

controller::controller(SDL_GameController* controller, i32 id)
    : _controller {controller}
    , _id {id}
{
    assert(controller);
}

auto controller::has_rumble() const -> bool
{
    assert(_controller);
    return SDL_GameControllerHasRumble(_controller);
}

auto controller::rumble(u16 lowFrequencyRumble, u16 highFrequencyRumble, milliseconds duration) const -> bool
{
    assert(_controller);
    return SDL_GameControllerRumble(_controller, lowFrequencyRumble, highFrequencyRumble, static_cast<u32>(duration.count())) == 0;
}

auto controller::has_rumble_triggers() const -> bool
{
    assert(_controller);
    return SDL_GameControllerHasRumbleTriggers(_controller);
}

auto controller::rumble_triggers(u16 leftRumble, u16 rightRumble, milliseconds duration) const -> bool
{
    assert(_controller);
    return SDL_GameControllerRumbleTriggers(_controller, leftRumble, rightRumble, static_cast<u32>(duration.count())) == 0;
}

auto controller::get_ID() const -> i32
{
    assert(_controller);
    return _id;
}

auto controller::get_name() const -> string
{
    assert(_controller);
    return SDL_GameControllerName(_controller);
}

auto controller::is_button_pressed(controller::button button) const -> bool
{
    assert(_controller);
    return SDL_GameControllerGetButton(_controller, convert_enum(button)) == 1;
}

auto controller::has_button(controller::button button) const -> bool
{
    assert(_controller);
    return SDL_GameControllerHasButton(_controller, convert_enum(button));
}

auto controller::get_button_name(controller::button button) const -> string
{
    return SDL_GameControllerGetStringForButton(convert_enum(button));
}

auto controller::get_axis_value(controller::axis axis) const -> i16
{
    assert(_controller);
    return SDL_GameControllerGetAxis(_controller, convert_enum(axis));
}

auto controller::has_axis(controller::axis axis) const -> bool
{
    assert(_controller);
    return SDL_GameControllerHasAxis(_controller, convert_enum(axis));
}

auto controller::get_axis_name(controller::axis axis) const -> string
{
    return SDL_GameControllerGetStringForAxis(convert_enum(axis));
}

////////////////////////////////////////////////////////////

auto keyboard::get_scancode(key_code key) const -> scan_code
{
    return static_cast<scan_code>(SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key))); // TODO: add switch here
}

auto keyboard::get_keycode(scan_code key) const -> key_code
{
    return static_cast<key_code>(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(key))); // TODO: add switch here
}

auto keyboard::is_key_down(scan_code key) const -> bool
{
    return system::IsKeyDown(key);
}

auto keyboard::is_mod_down(key_mod mod) const -> bool
{
    return system::IsKeyModDown(mod);
}

auto keyboard::get_mod_state() const -> std::unordered_map<key_mod, bool>
{
    auto const state {SDL_GetModState()};
    return {
        {key_mod::LeftShift, state & KMOD_LSHIFT},
        {key_mod::RightShift, state & KMOD_RSHIFT},
        {key_mod::Shift, state & KMOD_SHIFT},
        {key_mod::LeftControl, state & KMOD_LCTRL},
        {key_mod::RightControl, state & KMOD_RCTRL},
        {key_mod::Control, state & KMOD_CTRL},
        {key_mod::LeftAlt, state & KMOD_LALT},
        {key_mod::RightAlt, state & KMOD_RALT},
        {key_mod::Alt, state & KMOD_ALT},
        {key_mod::LeftGui, state & KMOD_LGUI},
        {key_mod::RightGui, state & KMOD_RGUI},
        {key_mod::Gui, state & KMOD_GUI},
        {key_mod::NumLock, state & KMOD_NUM},
        {key_mod::CapsLock, state & KMOD_CAPS},
        {key_mod::Mode, state & KMOD_MODE},
    };
}

////////////////////////////////////////////////////////////

auto mouse::get_position() const -> point_i
{
    return system::GetMousePosition();
}

void mouse::set_position(point_i pos) const
{
    system::SetMousePosition(pos);
}

auto mouse::is_button_down(button button) const -> bool
{
    return system::IsMouseButtonDown(button);
}

////////////////////////////////////////////////////////////

system::system()
{
    CurrentInputMode = mode::KeyboardMouse;
}

system::~system()
{
    for (auto const& [_, gc] : _controllers) {
        SDL_GameControllerClose(gc->_controller);
    }

    _controllers.clear();
}

auto system::get_controller_count() const -> isize
{
    return std::ssize(_controllers);
}

auto system::get_controller(i32 index) const -> std::shared_ptr<controller>
{
    auto it {_controllers.find(index)};
    if (it == _controllers.end()) {
        return nullptr;
    }

    return it->second;
}

auto system::get_mouse() const -> mouse
{
    return {};
}

auto system::get_keyboard() const -> keyboard
{
    return {};
}

void system::process_events(SDL_Event* ev)
{
    switch (ev->type) {
    case SDL_KEYDOWN: {
        keyboard::event event {
            .Pressed  = ev->key.state == SDL_PRESSED,
            .Repeat   = ev->key.repeat != 0,
            .ScanCode = static_cast<scan_code>(ev->key.keysym.scancode), // TODO: add switch here
            .KeyCode  = static_cast<key_code>(ev->key.keysym.sym),       // TODO: add switch here
            .KeyMods  = static_cast<key_mod>(ev->key.keysym.mod)         // TODO: add switch here
        };
        KeyDown(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_KEYUP: {
        keyboard::event event {
            .Pressed  = ev->key.state == SDL_PRESSED,
            .Repeat   = ev->key.repeat != 0,
            .ScanCode = static_cast<scan_code>(ev->key.keysym.scancode), // TODO: add switch here
            .KeyCode  = static_cast<key_code>(ev->key.keysym.sym),       // TODO: add switch here
            .KeyMods  = static_cast<key_mod>(ev->key.keysym.mod)         // TODO: add switch here
        };
        KeyUp(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_TEXTINPUT: {
        keyboard::text_input_event event {
            .Text = ev->text.text};
        TextInput(event);
    } break;
    case SDL_TEXTEDITING: {
        keyboard::text_editing_event event {
            .Text   = ev->edit.text,
            .Start  = ev->edit.start,
            .Length = ev->edit.length};
        TextEditing(event);
    } break;
    case SDL_MOUSEMOTION: {
        mouse::motion_event event {
            .Position       = {ev->motion.x, ev->motion.y},
            .RelativeMotion = {ev->motion.xrel, ev->motion.yrel}};
        MouseMotion(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_MOUSEBUTTONDOWN: {
        mouse::button_event event {
            .Button   = static_cast<mouse::button>(ev->button.button), // TODO: add switch here
            .Pressed  = ev->button.state == SDL_PRESSED,
            .Clicks   = ev->button.clicks,
            .Position = {ev->button.x, ev->button.y}};
        MouseButtonDown(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_MOUSEBUTTONUP: {
        mouse::button_event event {
            .Button   = static_cast<mouse::button>(ev->button.button), // TODO: add switch here
            .Pressed  = ev->button.state == SDL_PRESSED,
            .Clicks   = ev->button.clicks,
            .Position = {ev->button.x, ev->button.y}};
        MouseButtonUp(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_MOUSEWHEEL: {
        mouse::wheel_event event {
            .Scroll   = ev->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? point_i {-ev->wheel.x, -ev->wheel.y} : point_i {ev->wheel.x, ev->wheel.y},
            .Precise  = ev->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? point_f {-ev->wheel.preciseX, -ev->wheel.preciseY} : point_f {ev->wheel.preciseX, ev->wheel.preciseY},
            .Position = {ev->wheel.mouseX, ev->wheel.mouseY}};
        MouseWheel(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_JOYAXISMOTION: {
        joystick::axis_event event {
            .ID    = ev->jaxis.which,
            .Axis  = ev->jaxis.axis,
            .Value = ev->jaxis.value};
        JoystickAxisMotion(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_JOYHATMOTION: {
        joystick::hat_event event {
            .ID    = ev->jhat.which,
            .Hat   = static_cast<joystick::hat>(ev->jhat.hat), // TODO: add switch here
            .Value = ev->jhat.value};
        JoystickHatMotion(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_JOYBUTTONDOWN: {
        joystick::button_event event {
            .ID      = ev->jbutton.which,
            .Button  = ev->jbutton.button,
            .Pressed = ev->jbutton.state == SDL_PRESSED};
        JoystickButtonDown(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_JOYBUTTONUP: {
        joystick::button_event event {
            .ID      = ev->jbutton.which,
            .Button  = ev->jbutton.button,
            .Pressed = ev->jbutton.state == SDL_PRESSED};
        JoystickButtonUp(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_CONTROLLERAXISMOTION: {
        controller::axis_event event {
            .ID            = ev->caxis.which,
            .Controller    = _controllers[ev->cbutton.which],
            .Axis          = convert_enum(static_cast<SDL_GameControllerAxis>(ev->caxis.axis)),
            .Value         = ev->caxis.value,
            .RelativeValue = static_cast<f32>(ev->caxis.value) / std::numeric_limits<i16>::max()};
        ControllerAxisMotion(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_CONTROLLERBUTTONDOWN: {
        controller::button_event event {
            .ID         = ev->cbutton.which,
            .Controller = _controllers[ev->cbutton.which],
            .Button     = convert_enum(static_cast<SDL_GameControllerButton>(ev->cbutton.button)),
            .Pressed    = ev->cbutton.state == SDL_PRESSED};
        ControllerButtonDown(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_CONTROLLERBUTTONUP: {
        controller::button_event event {
            .ID         = ev->cbutton.which,
            .Controller = _controllers[ev->cbutton.which],
            .Button     = convert_enum(static_cast<SDL_GameControllerButton>(ev->cbutton.button)),
            .Pressed    = ev->cbutton.state == SDL_PRESSED};
        ControllerButtonUp(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_JOYDEVICEADDED: {
        i32 id {ev->jdevice.which};
        JoystickAdded(id);
        if (SDL_IsGameController(id)) {
            _controllers[id] = std::shared_ptr<controller>(new controller {SDL_GameControllerOpen(id), id});
            ControllerAdded(id);
        }
    } break;
    case SDL_JOYDEVICEREMOVED: {
        i32 id {ev->jdevice.which};
        JoystickRemoved(id);
        if (_controllers.contains(id)) {
            SDL_GameControllerClose(_controllers[id]->_controller);
            _controllers.erase(id);
            ControllerRemoved(id);
        }
    } break;
    }
}

auto system::IsKeyDown(scan_code key) -> bool
{
    auto const* state {SDL_GetKeyboardState(nullptr)};
    return state[static_cast<SDL_Scancode>(key)] != 0; // TODO: add switch here
}

auto system::IsKeyDown(key_code key) -> bool
{
    return IsKeyDown(static_cast<scan_code>(SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key))));
}

auto system::IsKeyModDown(key_mod mod) -> bool
{
    auto state {SDL_GetModState()};
    return state & static_cast<SDL_Keymod>(mod); // TODO: add switch here
}

auto system::IsMouseButtonDown(mouse::button button) -> bool
{
    return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(static_cast<i32>(button)); // TODO: add switch here
}

void system::SetMousePosition(point_i pos)
{
    SDL_WarpMouseInWindow(nullptr, pos.X, pos.Y);
}

auto system::GetClipboardText() -> utf8_string
{
    auto*       c {SDL_GetClipboardText()};
    utf8_string retValue {c};
    SDL_free(c);
    return retValue;
}

void system::SetClipboardText(utf8_string const& text)
{
    SDL_SetClipboardText(text.c_str());
}

auto system::GetMousePosition() -> point_i
{
    i32 x {0}, y {0};
    SDL_GetMouseState(&x, &y);
    return {x, y};
}

}
