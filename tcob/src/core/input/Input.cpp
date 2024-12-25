// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/input/Input.hpp"

#include <cassert>

#include "InputEnums.hpp"
#include <SDL.h>

namespace tcob::input {

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

auto controller::id() const -> i32
{
    assert(_controller);
    return _id;
}

auto controller::name() const -> string
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
    return convert_enum(SDL_GetScancodeFromKey(convert_enum(key)));
}

auto keyboard::get_keycode(scan_code key) const -> key_code
{
    return convert_enum(SDL_GetKeyFromScancode(convert_enum(key)));
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
    i32 x {0}, y {0};
    SDL_GetMouseState(&x, &y);
    return {x, y};
}

void mouse::set_position(point_i pos) const
{
    SDL_WarpMouseInWindow(nullptr, pos.X, pos.Y);
}

auto mouse::is_button_down(button button) const -> bool
{
    return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(convert_enum(button));
}

////////////////////////////////////////////////////////////

auto clipboard::has_text() const -> bool
{
    return SDL_HasClipboardText() == SDL_TRUE;
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
    CurrentInputMode = mode::KeyboardMouse;
}

system::~system()
{
    for (auto const& [_, gc] : _controllers) {
        SDL_GameControllerClose(gc->_controller);
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

auto system::get_mouse() const -> mouse
{
    return {};
}

auto system::get_keyboard() const -> keyboard
{
    return {};
}

auto system::get_clipboard() const -> clipboard
{
    return {};
}

void system::process_events(SDL_Event* ev)
{
    switch (ev->type) {
    case SDL_KEYDOWN: {
        keyboard::event const event {
            .Pressed  = ev->key.state == SDL_PRESSED,
            .Repeat   = ev->key.repeat != 0,
            .ScanCode = convert_enum(ev->key.keysym.scancode),
            .KeyMods  = convert_enum(static_cast<SDL_Keymod>(ev->key.keysym.mod)),
            .KeyCode  = convert_enum(ev->key.keysym.sym)};
        KeyDown(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_KEYUP: {
        keyboard::event const event {
            .Pressed  = ev->key.state == SDL_PRESSED,
            .Repeat   = ev->key.repeat != 0,
            .ScanCode = convert_enum(ev->key.keysym.scancode),
            .KeyMods  = convert_enum(static_cast<SDL_Keymod>(ev->key.keysym.mod)),
            .KeyCode  = convert_enum(ev->key.keysym.sym)};
        KeyUp(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_TEXTINPUT: {
        keyboard::text_input_event const event {.Text = ev->text.text};
        TextInput(event);
    } break;
    case SDL_TEXTEDITING: {
        keyboard::text_editing_event const event {
            .Text   = ev->edit.text,
            .Start  = ev->edit.start,
            .Length = ev->edit.length};
        TextEditing(event);
    } break;
    case SDL_MOUSEMOTION: {
        mouse::motion_event const event {
            .Position       = {ev->motion.x, ev->motion.y},
            .RelativeMotion = {ev->motion.xrel, ev->motion.yrel}};
        MouseMotion(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_MOUSEBUTTONDOWN: {
        mouse::button_event const event {
            .Button   = convert_mouse_button(ev->button.button),
            .Pressed  = ev->button.state == SDL_PRESSED,
            .Clicks   = ev->button.clicks,
            .Position = {ev->button.x, ev->button.y}};
        MouseButtonDown(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_MOUSEBUTTONUP: {
        mouse::button_event const event {
            .Button   = convert_mouse_button(ev->button.button),
            .Pressed  = ev->button.state == SDL_PRESSED,
            .Clicks   = ev->button.clicks,
            .Position = {ev->button.x, ev->button.y}};
        MouseButtonUp(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_MOUSEWHEEL: {
        mouse::wheel_event const event {
            .Scroll   = ev->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? point_i {-ev->wheel.x, -ev->wheel.y} : point_i {ev->wheel.x, ev->wheel.y},
            .Precise  = ev->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? point_f {-ev->wheel.preciseX, -ev->wheel.preciseY} : point_f {ev->wheel.preciseX, ev->wheel.preciseY},
            .Position = {ev->wheel.mouseX, ev->wheel.mouseY}};
        MouseWheel(event);
        CurrentInputMode = mode::KeyboardMouse;
    } break;
    case SDL_JOYAXISMOTION: {
        joystick::axis_event const event {
            .ID    = ev->jaxis.which,
            .Axis  = ev->jaxis.axis,
            .Value = ev->jaxis.value};
        JoystickAxisMotion(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_JOYHATMOTION: {
        joystick::hat_event const event {
            .ID    = ev->jhat.which,
            .Hat   = convert_joystick_hat(ev->jhat.hat),
            .Value = ev->jhat.value};
        JoystickHatMotion(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_JOYBUTTONDOWN: {
        joystick::button_event const event {
            .ID      = ev->jbutton.which,
            .Button  = ev->jbutton.button,
            .Pressed = ev->jbutton.state == SDL_PRESSED};
        JoystickButtonDown(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_JOYBUTTONUP: {
        joystick::button_event const event {
            .ID      = ev->jbutton.which,
            .Button  = ev->jbutton.button,
            .Pressed = ev->jbutton.state == SDL_PRESSED};
        JoystickButtonUp(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_CONTROLLERAXISMOTION: {
        controller::axis_event const event {
            .ID            = ev->caxis.which,
            .Controller    = _controllers[ev->cbutton.which],
            .Axis          = convert_enum(static_cast<SDL_GameControllerAxis>(ev->caxis.axis)),
            .Value         = ev->caxis.value,
            .RelativeValue = static_cast<f32>(ev->caxis.value) / std::numeric_limits<i16>::max()};
        ControllerAxisMotion(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_CONTROLLERBUTTONDOWN: {
        controller::button_event const event {
            .ID         = ev->cbutton.which,
            .Controller = _controllers[ev->cbutton.which],
            .Button     = convert_enum(static_cast<SDL_GameControllerButton>(ev->cbutton.button)),
            .Pressed    = ev->cbutton.state == SDL_PRESSED};
        ControllerButtonDown(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_CONTROLLERBUTTONUP: {
        controller::button_event const event {
            .ID         = ev->cbutton.which,
            .Controller = _controllers[ev->cbutton.which],
            .Button     = convert_enum(static_cast<SDL_GameControllerButton>(ev->cbutton.button)),
            .Pressed    = ev->cbutton.state == SDL_PRESSED};
        ControllerButtonUp(event);
        CurrentInputMode = mode::Controller;
    } break;
    case SDL_JOYDEVICEADDED: {
        i32 const id {ev->jdevice.which};
        JoystickAdded(id);
        if (SDL_IsGameController(id)) {
            _controllers[id] = std::shared_ptr<controller>(new controller {SDL_GameControllerOpen(id), id});
            ControllerAdded(id);
        }
    } break;
    case SDL_JOYDEVICEREMOVED: {
        i32 const id {ev->jdevice.which};
        JoystickRemoved(id);
        if (_controllers.contains(id)) {
            SDL_GameControllerClose(_controllers[id]->_controller);
            _controllers.erase(id);
            ControllerRemoved(id);
        }
    } break;
    }
}

}
