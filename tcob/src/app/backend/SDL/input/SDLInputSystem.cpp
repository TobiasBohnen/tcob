// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "SDLInputSystem.hpp"

#include <cassert>
#include <limits>
#include <memory>
#include <unordered_map>

#include "tcob/core/Point.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"

#include "SDLInputEnums.hpp"

namespace tcob::input {

sdl_controller::sdl_controller(SDL_Gamepad* controller, u32 id)
    : _controller {controller}
    , _id {id}
{
    assert(controller);
}

auto sdl_controller::has_rumble() const -> bool
{
    return SDL_GetBooleanProperty(SDL_GetGamepadProperties(_controller), SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false);
}

auto sdl_controller::rumble(u16 lowFrequencyRumble, u16 highFrequencyRumble, milliseconds duration) const -> bool
{
    return SDL_RumbleGamepad(_controller, lowFrequencyRumble, highFrequencyRumble, static_cast<u32>(duration.count())) == 0;
}

auto sdl_controller::has_rumble_triggers() const -> bool
{
    return SDL_GetBooleanProperty(SDL_GetGamepadProperties(_controller), SDL_PROP_GAMEPAD_CAP_TRIGGER_RUMBLE_BOOLEAN, false);
}

auto sdl_controller::rumble_triggers(u16 leftRumble, u16 rightRumble, milliseconds duration) const -> bool
{
    return SDL_RumbleGamepadTriggers(_controller, leftRumble, rightRumble, static_cast<u32>(duration.count())) == 0;
}

auto sdl_controller::id() const -> u32
{
    return _id;
}

auto sdl_controller::name() const -> string
{
    return SDL_GetGamepadName(_controller);
}

auto sdl_controller::is_button_pressed(button b) const -> bool
{
    return SDL_GetGamepadButton(_controller, convert_enum(b)) == 1;
}

auto sdl_controller::has_button(button b) const -> bool
{
    return SDL_GamepadHasButton(_controller, convert_enum(b));
}

auto sdl_controller::get_button_name(button b) const -> string
{
    return SDL_GetGamepadStringForButton(convert_enum(b));
}

auto sdl_controller::get_button_label(button b) const -> button_label
{
    return convert_enum(SDL_GetGamepadButtonLabel(_controller, convert_enum(b)));
}

auto sdl_controller::get_axis_value(axis a) const -> i16
{
    return SDL_GetGamepadAxis(_controller, convert_enum(a));
}

auto sdl_controller::has_axis(axis a) const -> bool
{
    return SDL_GamepadHasAxis(_controller, convert_enum(a));
}

auto sdl_controller::get_axis_name(axis a) const -> string
{
    return SDL_GetGamepadStringForAxis(convert_enum(a));
}

////////////////////////////////////////////////////////////

auto sdl_keyboard::get_scancode(key_code key) const -> scan_code
{
    return convert_enum(SDL_GetScancodeFromKey(convert_enum(key), nullptr));
}

auto sdl_keyboard::get_keycode(scan_code key) const -> key_code
{
    return convert_enum(SDL_GetKeyFromScancode(convert_enum(key), 0, false));
}

auto sdl_keyboard::is_key_down(scan_code key) const -> bool
{
    auto const* state {SDL_GetKeyboardState(nullptr)};
    return state[convert_enum(key)] != 0;
}

auto sdl_keyboard::is_key_down(key_code key) const -> bool
{
    return is_key_down(get_scancode(key));
}

auto sdl_keyboard::is_mod_down(key_mod mod) const -> bool
{
    auto const state {SDL_GetModState()};
    return state & convert_enum(mod);
}

auto sdl_keyboard::mods() const -> key_mods
{
    auto const state {SDL_GetModState()};
    return key_mods {convert_enum(state)};
}

////////////////////////////////////////////////////////////

auto sdl_mouse::get_position() const -> point_i
{
    f32 x {0}, y {0};
    SDL_GetMouseState(&x, &y);
    return {static_cast<i32>(x), static_cast<i32>(y)};
}

void sdl_mouse::set_position(point_i pos) const
{
    SDL_WarpMouseInWindow(nullptr, static_cast<f32>(pos.X), static_cast<f32>(pos.Y));
}

auto sdl_mouse::is_button_down(button button) const -> bool
{
    return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(convert_enum(button));
}

////////////////////////////////////////////////////////////

auto sdl_clipboard::has_text() const -> bool
{
    return SDL_HasClipboardText();
}

auto sdl_clipboard::get_text() const -> utf8_string
{
    auto*       c {SDL_GetClipboardText()};
    utf8_string retValue {c};
    SDL_free(c);
    return retValue;
}

void sdl_clipboard::set_text(utf8_string const& text)
{
    SDL_SetClipboardText(text.c_str());
}

////////////////////////////////////////////////////////////

sdl_input_system::sdl_input_system()
    : _mouse {std::make_shared<sdl_mouse>()}
    , _keyboard {std::make_shared<sdl_keyboard>()}
    , _clipboard {std::make_shared<sdl_clipboard>()}
{
    InputMode = mode::KeyboardMouse;
}

sdl_input_system::~sdl_input_system()
{
    for (auto const& [_, gc] : _controllers) {
        SDL_CloseGamepad(std::dynamic_pointer_cast<sdl_controller>(gc)->_controller);
    }

    _controllers.clear();
}

auto sdl_input_system::controllers() const -> std::unordered_map<i32, std::shared_ptr<controller>> const&
{
    return _controllers;
}

auto sdl_input_system::mouse() const -> std::shared_ptr<input::mouse>
{
    return _mouse;
}

auto sdl_input_system::keyboard() const -> std::shared_ptr<input::keyboard>
{
    return _keyboard;
}

auto sdl_input_system::clipboard() const -> std::shared_ptr<input::clipboard>
{
    return _clipboard;
}

void sdl_input_system::process_events(void* ev)
{
    auto* sev {static_cast<SDL_Event*>(ev)};

    static class sdl_keyboard key;
    static class sdl_mouse    sdl_mouse;

    switch (sev->type) {
    case SDL_EVENT_KEY_DOWN: {
        sdl_keyboard::event const event {
            .Keyboard = &key,
            .Pressed  = !sev->key.down,
            .Repeat   = sev->key.repeat != 0,
            .ScanCode = convert_enum(sev->key.scancode),
            .KeyMods  = key_mods {convert_enum(sev->key.mod)},
            .KeyCode  = convert_enum(sev->key.key)};
        KeyDown(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_KEY_UP: {
        sdl_keyboard::event const event {
            .Keyboard = &key,
            .Pressed  = sev->key.down,
            .Repeat   = sev->key.repeat != 0,
            .ScanCode = convert_enum(sev->key.scancode),
            .KeyMods  = key_mods {convert_enum(sev->key.mod)},
            .KeyCode  = convert_enum(sev->key.key)};
        KeyUp(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_TEXT_INPUT: {
        sdl_keyboard::text_input_event const event {.Text = sev->text.text};
        TextInput(event);
    } break;
    case SDL_EVENT_MOUSE_MOTION: {
        sdl_mouse::motion_event const event {
            .Mouse          = &sdl_mouse,
            .Position       = {static_cast<i32>(sev->motion.x), static_cast<i32>(sev->motion.y)},
            .RelativeMotion = {static_cast<i32>(sev->motion.xrel), static_cast<i32>(sev->motion.yrel)}};
        MouseMotion(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        sdl_mouse::button_event const event {
            .Mouse    = &sdl_mouse,
            .Button   = convert_mouse_button(sev->button.button),
            .Pressed  = sev->button.down,
            .Clicks   = sev->button.clicks,
            .Position = {static_cast<i32>(sev->button.x), static_cast<i32>(sev->button.y)}};
        MouseButtonDown(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        sdl_mouse::button_event const event {
            .Mouse    = &sdl_mouse,
            .Button   = convert_mouse_button(sev->button.button),
            .Pressed  = !sev->button.down,
            .Clicks   = sev->button.clicks,
            .Position = {static_cast<i32>(sev->button.x), static_cast<i32>(sev->button.y)}};
        MouseButtonUp(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_MOUSE_WHEEL: {
        sdl_mouse::wheel_event const event {
            .Mouse    = &sdl_mouse,
            .Scroll   = sev->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? point_f {-sev->wheel.x, -sev->wheel.y} : point_f {sev->wheel.x, sev->wheel.y},
            .Position = {static_cast<i32>(sev->wheel.mouse_x), static_cast<i32>(sev->wheel.mouse_y)}};
        MouseWheel(event);
        InputMode = mode::KeyboardMouse;
    } break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        sdl_controller::axis_event const event {
            .ID            = sev->gaxis.which,
            .Controller    = _controllers[sev->gaxis.which].get(),
            .Axis          = convert_enum(static_cast<SDL_GamepadAxis>(sev->gaxis.axis)),
            .Value         = sev->gaxis.value,
            .RelativeValue = static_cast<f32>(sev->gaxis.value) / std::numeric_limits<i16>::max()};
        ControllerAxisMotion(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
        sdl_controller::button_event const event {
            .ID         = sev->gbutton.which,
            .Controller = _controllers[sev->gbutton.which].get(),
            .Button     = convert_enum(static_cast<SDL_GamepadButton>(sev->gbutton.button)),
            .Pressed    = sev->gbutton.down};
        ControllerButtonDown(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        sdl_controller::button_event const event {
            .ID         = sev->gbutton.which,
            .Controller = _controllers[sev->gbutton.which].get(),
            .Button     = convert_enum(static_cast<SDL_GamepadButton>(sev->gbutton.button)),
            .Pressed    = !sev->gbutton.down};
        ControllerButtonUp(event);
        InputMode = mode::Controller;
    } break;
    case SDL_EVENT_GAMEPAD_ADDED: {
        u32 const id {sev->gdevice.which};
        _controllers[id] = std::make_shared<sdl_controller>(SDL_OpenGamepad(id), id);
        ControllerAdded(id);
    } break;
    case SDL_EVENT_GAMEPAD_REMOVED: {
        u32 const id {sev->gdevice.which};
        SDL_CloseGamepad(std::dynamic_pointer_cast<sdl_controller>(_controllers[id])->_controller);
        _controllers.erase(id);
        ControllerRemoved(id);
    } break;
    case SDL_EVENT_CLIPBOARD_UPDATE: {
        ClipboardUpdated();
    } break;
    }
}

}
