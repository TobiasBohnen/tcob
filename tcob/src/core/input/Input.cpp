// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/input/Input.hpp"

#include "InputEnums.hpp"

#include <SDL3/SDL.h>

#include <cassert>
#include <iterator>
#include <limits>
#include <memory>
#include <unordered_map>

#include "tcob/core/Point.hpp"
#include "tcob/core/input/Input_Codes.hpp"

namespace tcob::input {

controller::controller(SDL_Gamepad* controller, u32 id)
    : _controller {controller}
    , _id {id}
{
    assert(controller);
}

auto controller::has_rumble() const -> bool
{
    assert(_controller);
    return SDL_GetBooleanProperty(SDL_GetGamepadProperties(_controller), SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false);
}

auto controller::rumble(u16 lowFrequencyRumble, u16 highFrequencyRumble, milliseconds duration) const -> bool
{
    assert(_controller);
    return SDL_RumbleGamepad(_controller, lowFrequencyRumble, highFrequencyRumble, static_cast<u32>(duration.count())) == 0;
}

auto controller::has_rumble_triggers() const -> bool
{
    assert(_controller);
    return SDL_GetBooleanProperty(SDL_GetGamepadProperties(_controller), SDL_PROP_GAMEPAD_CAP_TRIGGER_RUMBLE_BOOLEAN, false);
}

auto controller::rumble_triggers(u16 leftRumble, u16 rightRumble, milliseconds duration) const -> bool
{
    assert(_controller);
    return SDL_RumbleGamepadTriggers(_controller, leftRumble, rightRumble, static_cast<u32>(duration.count())) == 0;
}

auto controller::id() const -> u32
{
    assert(_controller);
    return _id;
}

auto controller::name() const -> string
{
    assert(_controller);
    return SDL_GetGamepadName(_controller);
}

auto controller::is_button_pressed(controller::button button) const -> bool
{
    assert(_controller);
    return SDL_GetGamepadButton(_controller, convert_enum(button)) == 1;
}

auto controller::has_button(controller::button button) const -> bool
{
    assert(_controller);
    return SDL_GamepadHasButton(_controller, convert_enum(button));
}

auto controller::get_button_name(controller::button button) const -> string
{
    return SDL_GetGamepadStringForButton(convert_enum(button));
}

auto controller::get_axis_value(controller::axis axis) const -> i16
{
    assert(_controller);
    return SDL_GetGamepadAxis(_controller, convert_enum(axis));
}

auto controller::has_axis(controller::axis axis) const -> bool
{
    assert(_controller);
    return SDL_GamepadHasAxis(_controller, convert_enum(axis));
}

auto controller::get_axis_name(controller::axis axis) const -> string
{
    return SDL_GetGamepadStringForAxis(convert_enum(axis));
}

////////////////////////////////////////////////////////////

auto keyboard::get_scancode(key_code key) const -> scan_code
{
    return convert_enum(SDL_GetScancodeFromKey(convert_enum(key), nullptr));
}

auto keyboard::get_keycode(scan_code key) const -> key_code
{
    return convert_enum(SDL_GetKeyFromScancode(convert_enum(key), 0, false));
}

auto keyboard::is_key_down(scan_code key) const -> bool
{
    auto const* state {SDL_GetKeyboardState(nullptr)};
    return state[convert_enum(key)] != 0;
}

auto keyboard::is_key_down(key_code key) const -> bool
{
    return is_key_down(get_scancode(key));
}

auto keyboard::is_mod_down(key_mod mod) const -> bool
{
    auto const state {SDL_GetModState()};
    return state & convert_enum(mod);
}

auto keyboard::mod_state() const -> std::unordered_map<key_mod, bool>
{
    auto const state {SDL_GetModState()};
    return {
        {key_mod::LeftShift, state & SDL_KMOD_LSHIFT},
        {key_mod::RightShift, state & SDL_KMOD_RSHIFT},
        {key_mod::Shift, state & SDL_KMOD_SHIFT},
        {key_mod::LeftControl, state & SDL_KMOD_LCTRL},
        {key_mod::RightControl, state & SDL_KMOD_RCTRL},
        {key_mod::Control, state & SDL_KMOD_CTRL},
        {key_mod::LeftAlt, state & SDL_KMOD_LALT},
        {key_mod::RightAlt, state & SDL_KMOD_RALT},
        {key_mod::Alt, state & SDL_KMOD_ALT},
        {key_mod::LeftGui, state & SDL_KMOD_LGUI},
        {key_mod::RightGui, state & SDL_KMOD_RGUI},
        {key_mod::Gui, state & SDL_KMOD_GUI},
        {key_mod::NumLock, state & SDL_KMOD_NUM},
        {key_mod::CapsLock, state & SDL_KMOD_CAPS},
        {key_mod::Mode, state & SDL_KMOD_MODE},
    };
}

////////////////////////////////////////////////////////////

auto mouse::get_position() const -> point_i
{
    f32 x {0}, y {0};
    SDL_GetMouseState(&x, &y);
    return {static_cast<i32>(x), static_cast<i32>(y)};
}

void mouse::set_position(point_i pos) const
{
    SDL_WarpMouseInWindow(nullptr, pos.X, pos.Y);
}

auto mouse::is_button_down(button button) const -> bool
{
    return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(convert_enum(button));
}

////////////////////////////////////////////////////////////

auto clipboard::has_text() const -> bool
{
    return SDL_HasClipboardText();
}

auto clipboard::get_text() const -> utf8_string
{
    auto*       c {SDL_GetClipboardText()};
    utf8_string retValue {c};
    SDL_free(c);
    return retValue;
}

void clipboard::set_text(utf8_string const& text)
{
    SDL_SetClipboardText(text.c_str());
}

////////////////////////////////////////////////////////////

system::system()
{
    InputMode = mode::KeyboardMouse;
}

system::~system()
{
    for (auto const& [_, gc] : _controllers) {
        SDL_CloseGamepad(gc->_controller);
    }

    _controllers.clear();
}

auto system::controller_count() const -> isize
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

auto system::mouse() const -> input::mouse
{
    return {};
}

auto system::keyboard() const -> input::keyboard
{
    return {};
}

auto system::clipboard() const -> input::clipboard
{
    return {};
}

void system::process_events(SDL_Event* ev)
{
    static class keyboard key;
    static class mouse    mouse;

    switch (ev->type) {
    case SDL_EVENT_KEY_DOWN: {
        keyboard::event const event {
            .Keyboard = &key,
            .Pressed  = !ev->key.down,
            .Repeat   = ev->key.repeat != 0,
            .ScanCode = convert_enum(ev->key.scancode),
            .KeyMods  = convert_enum(static_cast<SDL_Keymod>(ev->key.mod)),
            .KeyCode  = convert_enum(ev->key.key)};
        KeyDown(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_KEY_UP: {
        keyboard::event const event {
            .Keyboard = &key,
            .Pressed  = ev->key.down,
            .Repeat   = ev->key.repeat != 0,
            .ScanCode = convert_enum(ev->key.scancode),
            .KeyMods  = convert_enum(static_cast<SDL_Keymod>(ev->key.mod)),
            .KeyCode  = convert_enum(ev->key.key)};
        KeyUp(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_TEXT_INPUT: {
        keyboard::text_input_event const event {.Text = ev->text.text};
        TextInput(event);
    } break;
    case SDL_EVENT_MOUSE_MOTION: {
        mouse::motion_event const event {
            .Mouse          = &mouse,
            .Position       = {static_cast<i32>(ev->motion.x), static_cast<i32>(ev->motion.y)},
            .RelativeMotion = {static_cast<i32>(ev->motion.xrel), static_cast<i32>(ev->motion.yrel)}};
        MouseMotion(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        mouse::button_event const event {
            .Mouse    = &mouse,
            .Button   = convert_mouse_button(ev->button.button),
            .Pressed  = ev->button.down,
            .Clicks   = ev->button.clicks,
            .Position = {static_cast<i32>(ev->button.x), static_cast<i32>(ev->button.y)}};
        MouseButtonDown(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        mouse::button_event const event {
            .Mouse    = &mouse,
            .Button   = convert_mouse_button(ev->button.button),
            .Pressed  = !ev->button.down,
            .Clicks   = ev->button.clicks,
            .Position = {static_cast<i32>(ev->button.x), static_cast<i32>(ev->button.y)}};
        MouseButtonUp(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_MOUSE_WHEEL: {
        mouse::wheel_event const event {
            .Mouse    = &mouse,
            .Scroll   = ev->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? point_f {-ev->wheel.x, -ev->wheel.y} : point_f {ev->wheel.x, ev->wheel.y},
            .Position = {static_cast<i32>(ev->wheel.mouse_x), static_cast<i32>(ev->wheel.mouse_y)}};
        MouseWheel(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
        joystick::axis_event const event {
            .ID    = ev->jaxis.which,
            .Axis  = ev->jaxis.axis,
            .Value = ev->jaxis.value};
        JoystickAxisMotion(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_JOYSTICK_HAT_MOTION: {
        joystick::hat_event const event {
            .ID    = ev->jhat.which,
            .Hat   = convert_joystick_hat(ev->jhat.hat),
            .Value = ev->jhat.value};
        JoystickHatMotion(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN: {
        joystick::button_event const event {
            .ID      = ev->jbutton.which,
            .Button  = ev->jbutton.button,
            .Pressed = ev->jbutton.down};
        JoystickButtonDown(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_JOYSTICK_BUTTON_UP: {
        joystick::button_event const event {
            .ID      = ev->jbutton.which,
            .Button  = ev->jbutton.button,
            .Pressed = !ev->jbutton.down};
        JoystickButtonUp(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        controller::axis_event const event {
            .ID            = ev->gaxis.which,
            .Controller    = _controllers[ev->gaxis.which],
            .Axis          = convert_enum(static_cast<SDL_GamepadAxis>(ev->gaxis.axis)),
            .Value         = ev->gaxis.value,
            .RelativeValue = static_cast<f32>(ev->gaxis.value) / std::numeric_limits<i16>::max()};
        ControllerAxisMotion(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
        controller::button_event const event {
            .ID         = ev->gbutton.which,
            .Controller = _controllers[ev->gbutton.which],
            .Button     = convert_enum(static_cast<SDL_GamepadButton>(ev->gbutton.button)),
            .Pressed    = ev->gbutton.down};
        ControllerButtonDown(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        controller::button_event const event {
            .ID         = ev->gbutton.which,
            .Controller = _controllers[ev->gbutton.which],
            .Button     = convert_enum(static_cast<SDL_GamepadButton>(ev->gbutton.button)),
            .Pressed    = !ev->gbutton.down};
        ControllerButtonUp(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_JOYSTICK_ADDED: {
        u32 const id {ev->jdevice.which};
        JoystickAdded(id);
        if (SDL_IsGamepad(id)) {
            _controllers[id] = std::shared_ptr<controller>(new controller {SDL_OpenGamepad(id), id});
            ControllerAdded(id);
        }
    } break;
    case SDL_EVENT_JOYSTICK_REMOVED: {
        u32 const id {ev->jdevice.which};
        JoystickRemoved(id);
        if (_controllers.contains(id)) {
            SDL_CloseGamepad(_controllers[id]->_controller);
            _controllers.erase(id);
            ControllerRemoved(id);
        }
    } break;
    }
}
}
