// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/Input.hpp>

#include <cassert>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>

namespace tcob {

GameController::GameController(SDL_GameController* controller)
    : _controller { controller }
{
}

auto GameController::name() const -> std::string
{
    assert(_controller);
    return SDL_JoystickName(SDL_GameControllerGetJoystick(_controller));
}

auto GameController::rumble(u16 lowFrequencyRumble, u16 highFrequencyRumble, MilliSeconds duration) const -> bool
{
    assert(_controller);
    return SDL_GameControllerRumble(_controller, lowFrequencyRumble, highFrequencyRumble, static_cast<u32>(duration.count())) == 0;
}

auto GameController::is_button_pressed(GameControllerButton button) const -> bool
{
    assert(_controller);
    return SDL_GameControllerGetButton(_controller, static_cast<SDL_GameControllerButton>(button)) == 1; //TODO: add switch here
}

auto GameController::has_button(GameControllerButton button) const -> bool
{
    assert(_controller);
    return SDL_GameControllerHasButton(_controller, static_cast<SDL_GameControllerButton>(button)); //TODO: add switch here
}

auto GameController::ButtonName(GameControllerButton button) -> std::string
{
    return SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(button)); //TODO: add switch here
}

auto GameController::axis_value(GameControllerAxis axis) const -> i16
{
    assert(_controller);
    return SDL_GameControllerGetAxis(_controller, static_cast<SDL_GameControllerAxis>(axis)); //TODO: add switch here
}

auto GameController::has_axis(GameControllerAxis axis) const -> bool
{
    assert(_controller);
    return SDL_GameControllerHasAxis(_controller, static_cast<SDL_GameControllerAxis>(axis)); //TODO: add switch here
}

auto GameController::AxisName(GameControllerAxis axis) -> std::string
{
    return SDL_GameControllerGetStringForAxis(static_cast<SDL_GameControllerAxis>(axis)); //TODO: add switch here
}

auto GameController::is_valid() const -> bool
{
    return _controller;
}

////////////////////////////////////////////////////////////

Input::Input()
{
}

Input::~Input()
{
    for (auto [_, gc] : _controllers) {
        SDL_GameControllerClose(gc);
    }

    _controllers.clear();
}

auto Input::controller_at(u32 index) -> GameController
{
    if (_controllers.contains(index)) {
        return { _controllers[index] };
    }

    return { nullptr };
}

auto Input::controller_count() -> u32
{
    return static_cast<u32>(_controllers.size());
}

auto Input::mode() -> InputMode
{
    return _mode;
}

void Input::process_events(SDL_Event* ev)
{
    InputMode newMode { _mode };

    switch (ev->type) {
    case SDL_KEYDOWN: {
        KeyboardEvent event {
            .Pressed = ev->key.state == SDL_PRESSED,
            .Repeat = ev->key.repeat != 0,
            .Code = static_cast<tcob::Scancode>(ev->key.keysym.scancode), //TODO: add switch here
            .Key = static_cast<tcob::KeyCode>(ev->key.keysym.sym), //TODO: add switch here
            .Mod = static_cast<KeyMod>(ev->key.keysym.mod) //TODO: add switch here
        };
        KeyDown(event);
        newMode = InputMode::KeyboardMouse;
    } break;
    case SDL_KEYUP: {
        KeyboardEvent event {
            .Pressed = ev->key.state == SDL_PRESSED,
            .Repeat = ev->key.repeat != 0,
            .Code = static_cast<tcob::Scancode>(ev->key.keysym.scancode), //TODO: add switch here
            .Key = static_cast<tcob::KeyCode>(ev->key.keysym.sym), //TODO: add switch here
            .Mod = static_cast<KeyMod>(ev->key.keysym.mod) //TODO: add switch here
        };
        KeyUp(event);
        newMode = InputMode::KeyboardMouse;
    } break;
    case SDL_TEXTINPUT: {
        TextInputEvent event {
            .Text = ev->text.text
        };
        TextInput(event);
    } break;
    case SDL_TEXTEDITING: {
        TextEditingEvent event {
            .Text = ev->edit.text,
            .Start = ev->edit.start,
            .Length = ev->edit.length
        };
        TextEditing(event);
    } break;
    case SDL_MOUSEMOTION: {
        MouseMotionEvent event {
            .Position = { ev->motion.x, ev->motion.y },
            .RelativeMotion = { ev->motion.xrel, ev->motion.yrel }
        };
        MouseMotion(event);
        newMode = InputMode::KeyboardMouse;
    } break;
    case SDL_MOUSEBUTTONDOWN: {
        MouseButtonEvent event {
            .Button = static_cast<MouseButton>(ev->button.button), //TODO: add switch here
            .Pressed = ev->button.state == SDL_PRESSED,
            .Clicks = ev->button.clicks,
            .Position = { ev->button.x, ev->button.y }
        };
        MouseButtonDown(event);
        newMode = InputMode::KeyboardMouse;
    } break;
    case SDL_MOUSEBUTTONUP: {
        MouseButtonEvent event {
            .Button = static_cast<MouseButton>(ev->button.button), //TODO: add switch here
            .Pressed = ev->button.state == SDL_PRESSED,
            .Clicks = ev->button.clicks,
            .Position = { ev->button.x, ev->button.y }
        };
        MouseButtonUp(event);
        newMode = InputMode::KeyboardMouse;
    } break;
    case SDL_MOUSEWHEEL: {
        MouseWheelEvent event {
            .Scroll = { ev->wheel.x, ev->wheel.y },
            .Flipped = ev->wheel.direction == SDL_MOUSEWHEEL_FLIPPED
        };
        MouseWheel(event);
        newMode = InputMode::KeyboardMouse;
    } break;
    case SDL_JOYAXISMOTION: {
        JoyAxisEvent event {
            .JoystickID = ev->jaxis.which,
            .Axis = ev->jaxis.axis,
            .Value = ev->jaxis.value
        };
        JoyAxisMotion(event);
        newMode = InputMode::Joystick;
    } break;
    case SDL_JOYHATMOTION: {
        JoyHatEvent event {
            .JoystickID = ev->jhat.which,
            .Hat = static_cast<JoyHat>(ev->jhat.hat), //TODO: add switch here
            .Value = ev->jhat.value
        };
        JoyHatMotion(event);
        newMode = InputMode::Joystick;
    } break;
    case SDL_JOYBUTTONDOWN: {
        JoyButtonEvent event {
            .JoystickID = ev->jbutton.which,
            .Button = ev->jbutton.button,
            .Pressed = ev->jbutton.state == SDL_PRESSED
        };
        JoyButtonDown(event);
        newMode = InputMode::Joystick;
    } break;
    case SDL_JOYBUTTONUP: {
        JoyButtonEvent event {
            .JoystickID = ev->jbutton.which,
            .Button = ev->jbutton.button,
            .Pressed = ev->jbutton.state == SDL_PRESSED
        };
        JoyButtonUp(event);
        newMode = InputMode::Joystick;
    } break;
    case SDL_CONTROLLERAXISMOTION: {
        ControllerAxisEvent event {
            .JoystickID = ev->caxis.which,
            .Axis = static_cast<GameControllerAxis>(ev->caxis.axis), //TODO: add switch here
            .Value = ev->caxis.value
        };
        ControllerAxisMotion(event);
        newMode = InputMode::Controller;
    } break;
    case SDL_CONTROLLERBUTTONDOWN: {
        ControllerButtonEvent event {
            .JoystickID = ev->cbutton.which,
            .Button = static_cast<GameControllerButton>(ev->cbutton.button), //TODO: add switch here
            .Pressed = ev->cbutton.state == SDL_PRESSED
        };
        ControllerButtonDown(event);
        newMode = InputMode::Controller;
    } break;
    case SDL_CONTROLLERBUTTONUP: {
        ControllerButtonEvent event {
            .JoystickID = ev->cbutton.which,
            .Button = static_cast<GameControllerButton>(ev->cbutton.button), //TODO: add switch here
            .Pressed = ev->cbutton.state == SDL_PRESSED
        };
        ControllerButtonUp(event);
        newMode = InputMode::Controller;
    } break;
    case SDL_JOYDEVICEADDED: {
        i32 id { ev->jdevice.which };
        if (SDL_IsGameController(id)) {
            _controllers[id] = SDL_GameControllerOpen(id);
        }
    } break;
    case SDL_JOYDEVICEREMOVED: {
        i32 id { ev->jdevice.which };
        if (_controllers.contains(id)) {
            SDL_GameControllerClose(_controllers[id]);
            _controllers.erase(id);
        }
    } break;
    }

    if (_mode != newMode) {
        _mode = newMode;
        InputModeChanged(_mode);
    }
}

}