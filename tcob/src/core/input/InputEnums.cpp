// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "InputEnums.hpp"

#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"

namespace tcob::input {

auto convert_mouse_button(i32 button) -> mouse::button
{
    switch (button) {
    case SDL_BUTTON_LEFT:   return mouse::button::Left;
    case SDL_BUTTON_RIGHT:  return mouse::button::Right;
    case SDL_BUTTON_MIDDLE: return mouse::button::Middle;
    case SDL_BUTTON_X1:     return mouse::button::X1;
    case SDL_BUTTON_X2:     return mouse::button::X2;
    }

    return static_cast<mouse::button>(button);
}

auto convert_enum(mouse::button button) -> i32
{
    switch (button) {
    case mouse::button::None:   return 0;
    case mouse::button::Left:   return SDL_BUTTON_LEFT;
    case mouse::button::Middle: return SDL_BUTTON_MIDDLE;
    case mouse::button::Right:  return SDL_BUTTON_RIGHT;
    case mouse::button::X1:     return SDL_BUTTON_X1;
    case mouse::button::X2:     return SDL_BUTTON_X2;
    }

    return 0;
}

auto convert_enum(controller::button button) -> SDL_GamepadButton
{
    switch (button) {
    case controller::button::Invalid:       return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_INVALID;
    case controller::button::A:             return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_SOUTH;
    case controller::button::B:             return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_EAST;
    case controller::button::X:             return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_WEST;
    case controller::button::Y:             return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_NORTH;
    case controller::button::Back:          return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_BACK;
    case controller::button::Guide:         return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_GUIDE;
    case controller::button::Start:         return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_START;
    case controller::button::LeftStick:     return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_STICK;
    case controller::button::RightStick:    return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_STICK;
    case controller::button::LeftShoulder:  return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
    case controller::button::RightShoulder: return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
    case controller::button::DPadUp:        return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_UP;
    case controller::button::DPadDown:      return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_DOWN;
    case controller::button::DPadLeft:      return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_LEFT;
    case controller::button::DPadRight:     return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
    case controller::button::Misc1:         return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC1;
    case controller::button::Misc2:         return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC2;
    case controller::button::Misc3:         return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC3;
    case controller::button::Misc4:         return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC4;
    case controller::button::Misc5:         return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC5;
    case controller::button::Misc6:         return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC6;
    case controller::button::RightPaddle1:  return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1;
    case controller::button::LeftPaddle1:   return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_PADDLE1;
    case controller::button::RightPaddle2:  return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2;
    case controller::button::LeftPaddle2:   return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_PADDLE2;
    case controller::button::Touchpad:      return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_TOUCHPAD;
    }

    return SDL_GamepadButton::SDL_GAMEPAD_BUTTON_INVALID;
}

auto convert_enum(SDL_GamepadButton button) -> controller::button
{
    switch (button) {
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_INVALID:        return controller::button::Invalid;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_SOUTH:          return controller::button::A;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_EAST:           return controller::button::B;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_WEST:           return controller::button::X;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_NORTH:          return controller::button::Y;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_BACK:           return controller::button::Back;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_GUIDE:          return controller::button::Guide;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_START:          return controller::button::Start;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_STICK:     return controller::button::LeftStick;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_STICK:    return controller::button::RightStick;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  return controller::button::LeftShoulder;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return controller::button::RightShoulder;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_UP:        return controller::button::DPadUp;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_DOWN:      return controller::button::DPadDown;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_LEFT:      return controller::button::DPadLeft;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     return controller::button::DPadRight;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC1:          return controller::button::Misc1;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC2:          return controller::button::Misc2;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC3:          return controller::button::Misc3;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC4:          return controller::button::Misc4;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC5:          return controller::button::Misc5;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_MISC6:          return controller::button::Misc6;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1:  return controller::button::RightPaddle1;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_PADDLE1:   return controller::button::LeftPaddle1;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2:  return controller::button::RightPaddle2;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_PADDLE2:   return controller::button::LeftPaddle2;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_TOUCHPAD:       return controller::button::Touchpad;
    case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_COUNT:          break;
    }

    return controller::button::Invalid;
}

auto convert_enum(SDL_GamepadButtonLabel label) -> controller::button_label
{
    switch (label) {
    case SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN:  return controller::button_label::Invalid;
    case SDL_GAMEPAD_BUTTON_LABEL_A:        return controller::button_label::A;
    case SDL_GAMEPAD_BUTTON_LABEL_B:        return controller::button_label::B;
    case SDL_GAMEPAD_BUTTON_LABEL_X:        return controller::button_label::X;
    case SDL_GAMEPAD_BUTTON_LABEL_Y:        return controller::button_label::Y;
    case SDL_GAMEPAD_BUTTON_LABEL_CROSS:    return controller::button_label::Cross;
    case SDL_GAMEPAD_BUTTON_LABEL_CIRCLE:   return controller::button_label::Circle;
    case SDL_GAMEPAD_BUTTON_LABEL_SQUARE:   return controller::button_label::Square;
    case SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE: return controller::button_label::Triangle;
    }

    return controller::button_label::Invalid;
}

auto convert_enum(controller::axis axis) -> SDL_GamepadAxis
{
    switch (axis) {
    case controller::axis::Invalid:      return SDL_GamepadAxis::SDL_GAMEPAD_AXIS_INVALID;
    case controller::axis::LeftX:        return SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFTX;
    case controller::axis::LeftY:        return SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFTY;
    case controller::axis::RightX:       return SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHTX;
    case controller::axis::RightY:       return SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHTY;
    case controller::axis::LeftTrigger:  return SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFT_TRIGGER;
    case controller::axis::RightTrigger: return SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHT_TRIGGER;
    }

    return SDL_GamepadAxis::SDL_GAMEPAD_AXIS_INVALID;
}

auto convert_enum(SDL_GamepadAxis axis) -> controller::axis
{
    switch (axis) {
    case SDL_GAMEPAD_AXIS_INVALID:       return controller::axis::Invalid;
    case SDL_GAMEPAD_AXIS_LEFTX:         return controller::axis::LeftX;
    case SDL_GAMEPAD_AXIS_LEFTY:         return controller::axis::LeftY;
    case SDL_GAMEPAD_AXIS_RIGHTX:        return controller::axis::RightX;
    case SDL_GAMEPAD_AXIS_RIGHTY:        return controller::axis::RightY;
    case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:  return controller::axis::LeftTrigger;
    case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: return controller::axis::RightTrigger;
    case SDL_GAMEPAD_AXIS_COUNT:         return controller::axis::Invalid;
    }

    return controller::axis::Invalid;
}

auto convert_enum(key_mod mod) -> SDL_Keymod
{
    switch (mod) {
    case key_mod::None:         return SDL_KMOD_NONE;
    case key_mod::LeftShift:    return SDL_KMOD_LSHIFT;
    case key_mod::RightShift:   return SDL_KMOD_RSHIFT;
    case key_mod::LeftControl:  return SDL_KMOD_LCTRL;
    case key_mod::RightControl: return SDL_KMOD_RCTRL;
    case key_mod::LeftAlt:      return SDL_KMOD_LALT;
    case key_mod::RightAlt:     return SDL_KMOD_RALT;
    case key_mod::LeftGui:      return SDL_KMOD_LGUI;
    case key_mod::RightGui:     return SDL_KMOD_RGUI;
    case key_mod::NumLock:      return SDL_KMOD_NUM;
    case key_mod::CapsLock:     return SDL_KMOD_CAPS;
    case key_mod::Mode:         return SDL_KMOD_MODE;
    case key_mod::Control:      return SDL_KMOD_CTRL;
    case key_mod::Shift:        return SDL_KMOD_SHIFT;
    case key_mod::Alt:          return SDL_KMOD_ALT;
    case key_mod::Gui:          return SDL_KMOD_GUI;
    case key_mod::Scroll:       return SDL_KMOD_SCROLL;
    }

    return SDL_KMOD_NONE;
}

auto convert_enum(SDL_Keymod mod) -> key_mod
{
    switch (mod) {
    case SDL_KMOD_NONE:   return key_mod::None;
    case SDL_KMOD_LSHIFT: return key_mod::LeftShift;
    case SDL_KMOD_RSHIFT: return key_mod::RightShift;
    case SDL_KMOD_LCTRL:  return key_mod::LeftControl;
    case SDL_KMOD_RCTRL:  return key_mod::RightControl;
    case SDL_KMOD_LALT:   return key_mod::LeftAlt;
    case SDL_KMOD_RALT:   return key_mod::RightAlt;
    case SDL_KMOD_LGUI:   return key_mod::LeftGui;
    case SDL_KMOD_RGUI:   return key_mod::RightGui;
    case SDL_KMOD_NUM:    return key_mod::NumLock;
    case SDL_KMOD_CAPS:   return key_mod::CapsLock;
    case SDL_KMOD_MODE:   return key_mod::Mode;
    case SDL_KMOD_SCROLL: return key_mod::Scroll;
    case SDL_KMOD_CTRL:   return key_mod::Control;
    case SDL_KMOD_SHIFT:  return key_mod::Shift;
    case SDL_KMOD_ALT:    return key_mod::Alt;
    case SDL_KMOD_GUI:    return key_mod::Gui;
    }

    return static_cast<key_mod>(mod);
}

auto convert_enum(scan_code code) -> SDL_Scancode
{
    switch (code) {
    case scan_code::UNKNOWN:            return SDL_SCANCODE_UNKNOWN;
    case scan_code::A:                  return SDL_SCANCODE_A;
    case scan_code::B:                  return SDL_SCANCODE_B;
    case scan_code::C:                  return SDL_SCANCODE_C;
    case scan_code::D:                  return SDL_SCANCODE_D;
    case scan_code::E:                  return SDL_SCANCODE_E;
    case scan_code::F:                  return SDL_SCANCODE_F;
    case scan_code::G:                  return SDL_SCANCODE_G;
    case scan_code::H:                  return SDL_SCANCODE_H;
    case scan_code::I:                  return SDL_SCANCODE_I;
    case scan_code::J:                  return SDL_SCANCODE_J;
    case scan_code::K:                  return SDL_SCANCODE_K;
    case scan_code::L:                  return SDL_SCANCODE_L;
    case scan_code::M:                  return SDL_SCANCODE_M;
    case scan_code::N:                  return SDL_SCANCODE_N;
    case scan_code::O:                  return SDL_SCANCODE_O;
    case scan_code::P:                  return SDL_SCANCODE_P;
    case scan_code::Q:                  return SDL_SCANCODE_Q;
    case scan_code::R:                  return SDL_SCANCODE_R;
    case scan_code::S:                  return SDL_SCANCODE_S;
    case scan_code::T:                  return SDL_SCANCODE_T;
    case scan_code::U:                  return SDL_SCANCODE_U;
    case scan_code::V:                  return SDL_SCANCODE_V;
    case scan_code::W:                  return SDL_SCANCODE_W;
    case scan_code::X:                  return SDL_SCANCODE_X;
    case scan_code::Y:                  return SDL_SCANCODE_Y;
    case scan_code::Z:                  return SDL_SCANCODE_Z;
    case scan_code::D1:                 return SDL_SCANCODE_1;
    case scan_code::D2:                 return SDL_SCANCODE_2;
    case scan_code::D3:                 return SDL_SCANCODE_3;
    case scan_code::D4:                 return SDL_SCANCODE_4;
    case scan_code::D5:                 return SDL_SCANCODE_5;
    case scan_code::D6:                 return SDL_SCANCODE_6;
    case scan_code::D7:                 return SDL_SCANCODE_7;
    case scan_code::D8:                 return SDL_SCANCODE_8;
    case scan_code::D9:                 return SDL_SCANCODE_9;
    case scan_code::D0:                 return SDL_SCANCODE_0;
    case scan_code::RETURN:             return SDL_SCANCODE_RETURN;
    case scan_code::ESCAPE:             return SDL_SCANCODE_ESCAPE;
    case scan_code::BACKSPACE:          return SDL_SCANCODE_BACKSPACE;
    case scan_code::TAB:                return SDL_SCANCODE_TAB;
    case scan_code::SPACE:              return SDL_SCANCODE_SPACE;
    case scan_code::MINUS:              return SDL_SCANCODE_MINUS;
    case scan_code::EQUALS:             return SDL_SCANCODE_EQUALS;
    case scan_code::LEFTBRACKET:        return SDL_SCANCODE_LEFTBRACKET;
    case scan_code::RIGHTBRACKET:       return SDL_SCANCODE_RIGHTBRACKET;
    case scan_code::BACKSLASH:          return SDL_SCANCODE_BACKSLASH;
    case scan_code::NONUSHASH:          return SDL_SCANCODE_NONUSHASH;
    case scan_code::SEMICOLON:          return SDL_SCANCODE_SEMICOLON;
    case scan_code::APOSTROPHE:         return SDL_SCANCODE_APOSTROPHE;
    case scan_code::GRAVE:              return SDL_SCANCODE_GRAVE;
    case scan_code::COMMA:              return SDL_SCANCODE_COMMA;
    case scan_code::PERIOD:             return SDL_SCANCODE_PERIOD;
    case scan_code::SLASH:              return SDL_SCANCODE_SLASH;
    case scan_code::CAPSLOCK:           return SDL_SCANCODE_CAPSLOCK;
    case scan_code::F1:                 return SDL_SCANCODE_F1;
    case scan_code::F2:                 return SDL_SCANCODE_F2;
    case scan_code::F3:                 return SDL_SCANCODE_F3;
    case scan_code::F4:                 return SDL_SCANCODE_F4;
    case scan_code::F5:                 return SDL_SCANCODE_F5;
    case scan_code::F6:                 return SDL_SCANCODE_F6;
    case scan_code::F7:                 return SDL_SCANCODE_F7;
    case scan_code::F8:                 return SDL_SCANCODE_F8;
    case scan_code::F9:                 return SDL_SCANCODE_F9;
    case scan_code::F10:                return SDL_SCANCODE_F10;
    case scan_code::F11:                return SDL_SCANCODE_F11;
    case scan_code::F12:                return SDL_SCANCODE_F12;
    case scan_code::PRINTSCREEN:        return SDL_SCANCODE_PRINTSCREEN;
    case scan_code::SCROLLLOCK:         return SDL_SCANCODE_SCROLLLOCK;
    case scan_code::PAUSE:              return SDL_SCANCODE_PAUSE;
    case scan_code::INSERT:             return SDL_SCANCODE_INSERT;
    case scan_code::HOME:               return SDL_SCANCODE_HOME;
    case scan_code::PAGEUP:             return SDL_SCANCODE_PAGEUP;
    case scan_code::DEL:                return SDL_SCANCODE_DELETE;
    case scan_code::END:                return SDL_SCANCODE_END;
    case scan_code::PAGEDOWN:           return SDL_SCANCODE_PAGEDOWN;
    case scan_code::RIGHT:              return SDL_SCANCODE_RIGHT;
    case scan_code::LEFT:               return SDL_SCANCODE_LEFT;
    case scan_code::DOWN:               return SDL_SCANCODE_DOWN;
    case scan_code::UP:                 return SDL_SCANCODE_UP;
    case scan_code::NUMLOCKCLEAR:       return SDL_SCANCODE_NUMLOCKCLEAR;
    case scan_code::KP_DIVIDE:          return SDL_SCANCODE_KP_DIVIDE;
    case scan_code::KP_MULTIPLY:        return SDL_SCANCODE_KP_MULTIPLY;
    case scan_code::KP_MINUS:           return SDL_SCANCODE_KP_MINUS;
    case scan_code::KP_PLUS:            return SDL_SCANCODE_KP_PLUS;
    case scan_code::KP_ENTER:           return SDL_SCANCODE_KP_ENTER;
    case scan_code::KP_1:               return SDL_SCANCODE_KP_1;
    case scan_code::KP_2:               return SDL_SCANCODE_KP_2;
    case scan_code::KP_3:               return SDL_SCANCODE_KP_3;
    case scan_code::KP_4:               return SDL_SCANCODE_KP_4;
    case scan_code::KP_5:               return SDL_SCANCODE_KP_5;
    case scan_code::KP_6:               return SDL_SCANCODE_KP_6;
    case scan_code::KP_7:               return SDL_SCANCODE_KP_7;
    case scan_code::KP_8:               return SDL_SCANCODE_KP_8;
    case scan_code::KP_9:               return SDL_SCANCODE_KP_9;
    case scan_code::KP_0:               return SDL_SCANCODE_KP_0;
    case scan_code::KP_PERIOD:          return SDL_SCANCODE_KP_PERIOD;
    case scan_code::NONUSBACKSLASH:     return SDL_SCANCODE_NONUSBACKSLASH;
    case scan_code::APPLICATION:        return SDL_SCANCODE_APPLICATION;
    case scan_code::POWER:              return SDL_SCANCODE_POWER;
    case scan_code::KP_EQUALS:          return SDL_SCANCODE_KP_EQUALS;
    case scan_code::F13:                return SDL_SCANCODE_F13;
    case scan_code::F14:                return SDL_SCANCODE_F14;
    case scan_code::F15:                return SDL_SCANCODE_F15;
    case scan_code::F16:                return SDL_SCANCODE_F16;
    case scan_code::F17:                return SDL_SCANCODE_F17;
    case scan_code::F18:                return SDL_SCANCODE_F18;
    case scan_code::F19:                return SDL_SCANCODE_F19;
    case scan_code::F20:                return SDL_SCANCODE_F20;
    case scan_code::F21:                return SDL_SCANCODE_F21;
    case scan_code::F22:                return SDL_SCANCODE_F22;
    case scan_code::F23:                return SDL_SCANCODE_F23;
    case scan_code::F24:                return SDL_SCANCODE_F24;
    case scan_code::EXECUTE:            return SDL_SCANCODE_EXECUTE;
    case scan_code::HELP:               return SDL_SCANCODE_HELP;
    case scan_code::MENU:               return SDL_SCANCODE_MENU;
    case scan_code::SELECT:             return SDL_SCANCODE_SELECT;
    case scan_code::STOP:               return SDL_SCANCODE_STOP;
    case scan_code::AGAIN:              return SDL_SCANCODE_AGAIN;
    case scan_code::UNDO:               return SDL_SCANCODE_UNDO;
    case scan_code::CUT:                return SDL_SCANCODE_CUT;
    case scan_code::COPY:               return SDL_SCANCODE_COPY;
    case scan_code::PASTE:              return SDL_SCANCODE_PASTE;
    case scan_code::FIND:               return SDL_SCANCODE_FIND;
    case scan_code::MUTE:               return SDL_SCANCODE_MUTE;
    case scan_code::VOLUMEUP:           return SDL_SCANCODE_VOLUMEUP;
    case scan_code::VOLUMEDOWN:         return SDL_SCANCODE_VOLUMEDOWN;
    case scan_code::KP_COMMA:           return SDL_SCANCODE_KP_COMMA;
    case scan_code::KP_EQUALSAS400:     return SDL_SCANCODE_KP_EQUALSAS400;
    case scan_code::INTERNATIONAL1:     return SDL_SCANCODE_INTERNATIONAL1;
    case scan_code::INTERNATIONAL2:     return SDL_SCANCODE_INTERNATIONAL2;
    case scan_code::INTERNATIONAL3:     return SDL_SCANCODE_INTERNATIONAL3;
    case scan_code::INTERNATIONAL4:     return SDL_SCANCODE_INTERNATIONAL4;
    case scan_code::INTERNATIONAL5:     return SDL_SCANCODE_INTERNATIONAL5;
    case scan_code::INTERNATIONAL6:     return SDL_SCANCODE_INTERNATIONAL6;
    case scan_code::INTERNATIONAL7:     return SDL_SCANCODE_INTERNATIONAL7;
    case scan_code::INTERNATIONAL8:     return SDL_SCANCODE_INTERNATIONAL8;
    case scan_code::INTERNATIONAL9:     return SDL_SCANCODE_INTERNATIONAL9;
    case scan_code::LANG1:              return SDL_SCANCODE_LANG1;
    case scan_code::LANG2:              return SDL_SCANCODE_LANG2;
    case scan_code::LANG3:              return SDL_SCANCODE_LANG3;
    case scan_code::LANG4:              return SDL_SCANCODE_LANG4;
    case scan_code::LANG5:              return SDL_SCANCODE_LANG5;
    case scan_code::LANG6:              return SDL_SCANCODE_LANG6;
    case scan_code::LANG7:              return SDL_SCANCODE_LANG7;
    case scan_code::LANG8:              return SDL_SCANCODE_LANG8;
    case scan_code::LANG9:              return SDL_SCANCODE_LANG9;
    case scan_code::ALTERASE:           return SDL_SCANCODE_ALTERASE;
    case scan_code::SYSREQ:             return SDL_SCANCODE_SYSREQ;
    case scan_code::CANCEL:             return SDL_SCANCODE_CANCEL;
    case scan_code::CLEAR:              return SDL_SCANCODE_CLEAR;
    case scan_code::PRIOR:              return SDL_SCANCODE_PRIOR;
    case scan_code::RETURN2:            return SDL_SCANCODE_RETURN2;
    case scan_code::SEPARATOR:          return SDL_SCANCODE_SEPARATOR;
    case scan_code::KEY_OUT:            return SDL_SCANCODE_OUT;
    case scan_code::OPER:               return SDL_SCANCODE_OPER;
    case scan_code::CLEARAGAIN:         return SDL_SCANCODE_CLEARAGAIN;
    case scan_code::CRSEL:              return SDL_SCANCODE_CRSEL;
    case scan_code::EXSEL:              return SDL_SCANCODE_EXSEL;
    case scan_code::KP_00:              return SDL_SCANCODE_KP_00;
    case scan_code::KP_000:             return SDL_SCANCODE_KP_000;
    case scan_code::THOUSANDSSEPARATOR: return SDL_SCANCODE_THOUSANDSSEPARATOR;
    case scan_code::DECIMALSEPARATOR:   return SDL_SCANCODE_DECIMALSEPARATOR;
    case scan_code::CURRENCYUNIT:       return SDL_SCANCODE_CURRENCYUNIT;
    case scan_code::CURRENCYSUBUNIT:    return SDL_SCANCODE_CURRENCYSUBUNIT;
    case scan_code::KP_LEFTPAREN:       return SDL_SCANCODE_KP_LEFTPAREN;
    case scan_code::KP_RIGHTPAREN:      return SDL_SCANCODE_KP_RIGHTPAREN;
    case scan_code::KP_LEFTBRACE:       return SDL_SCANCODE_KP_LEFTBRACE;
    case scan_code::KP_RIGHTBRACE:      return SDL_SCANCODE_KP_RIGHTBRACE;
    case scan_code::KP_TAB:             return SDL_SCANCODE_KP_TAB;
    case scan_code::KP_BACKSPACE:       return SDL_SCANCODE_KP_BACKSPACE;
    case scan_code::KP_A:               return SDL_SCANCODE_KP_A;
    case scan_code::KP_B:               return SDL_SCANCODE_KP_B;
    case scan_code::KP_C:               return SDL_SCANCODE_KP_C;
    case scan_code::KP_D:               return SDL_SCANCODE_KP_D;
    case scan_code::KP_E:               return SDL_SCANCODE_KP_E;
    case scan_code::KP_F:               return SDL_SCANCODE_KP_F;
    case scan_code::KP_XOR:             return SDL_SCANCODE_KP_XOR;
    case scan_code::KP_POWER:           return SDL_SCANCODE_KP_POWER;
    case scan_code::KP_PERCENT:         return SDL_SCANCODE_KP_PERCENT;
    case scan_code::KP_LESS:            return SDL_SCANCODE_KP_LESS;
    case scan_code::KP_GREATER:         return SDL_SCANCODE_KP_GREATER;
    case scan_code::KP_AMPERSAND:       return SDL_SCANCODE_KP_AMPERSAND;
    case scan_code::KP_DBLAMPERSAND:    return SDL_SCANCODE_KP_DBLAMPERSAND;
    case scan_code::KP_VERTICALBAR:     return SDL_SCANCODE_KP_VERTICALBAR;
    case scan_code::KP_DBLVERTICALBAR:  return SDL_SCANCODE_KP_DBLVERTICALBAR;
    case scan_code::KP_COLON:           return SDL_SCANCODE_KP_COLON;
    case scan_code::KP_HASH:            return SDL_SCANCODE_KP_HASH;
    case scan_code::KP_SPACE:           return SDL_SCANCODE_KP_SPACE;
    case scan_code::KP_AT:              return SDL_SCANCODE_KP_AT;
    case scan_code::KP_EXCLAM:          return SDL_SCANCODE_KP_EXCLAM;
    case scan_code::KP_MEMSTORE:        return SDL_SCANCODE_KP_MEMSTORE;
    case scan_code::KP_MEMRECALL:       return SDL_SCANCODE_KP_MEMRECALL;
    case scan_code::KP_MEMCLEAR:        return SDL_SCANCODE_KP_MEMCLEAR;
    case scan_code::KP_MEMADD:          return SDL_SCANCODE_KP_MEMADD;
    case scan_code::KP_MEMSUBTRACT:     return SDL_SCANCODE_KP_MEMSUBTRACT;
    case scan_code::KP_MEMMULTIPLY:     return SDL_SCANCODE_KP_MEMMULTIPLY;
    case scan_code::KP_MEMDIVIDE:       return SDL_SCANCODE_KP_MEMDIVIDE;
    case scan_code::KP_PLUSMINUS:       return SDL_SCANCODE_KP_PLUSMINUS;
    case scan_code::KP_CLEAR:           return SDL_SCANCODE_KP_CLEAR;
    case scan_code::KP_CLEARENTRY:      return SDL_SCANCODE_KP_CLEARENTRY;
    case scan_code::KP_BINARY:          return SDL_SCANCODE_KP_BINARY;
    case scan_code::KP_OCTAL:           return SDL_SCANCODE_KP_OCTAL;
    case scan_code::KP_DECIMAL:         return SDL_SCANCODE_KP_DECIMAL;
    case scan_code::KP_HEXADECIMAL:     return SDL_SCANCODE_KP_HEXADECIMAL;
    case scan_code::LCTRL:              return SDL_SCANCODE_LCTRL;
    case scan_code::LSHIFT:             return SDL_SCANCODE_LSHIFT;
    case scan_code::LALT:               return SDL_SCANCODE_LALT;
    case scan_code::LGUI:               return SDL_SCANCODE_LGUI;
    case scan_code::RCTRL:              return SDL_SCANCODE_RCTRL;
    case scan_code::RSHIFT:             return SDL_SCANCODE_RSHIFT;
    case scan_code::RALT:               return SDL_SCANCODE_RALT;
    case scan_code::RGUI:               return SDL_SCANCODE_RGUI;
    case scan_code::MODE:               return SDL_SCANCODE_MODE;
    }

    return SDL_SCANCODE_UNKNOWN;
}

auto convert_enum(SDL_Scancode code) -> scan_code
{
    switch (code) {
    case SDL_SCANCODE_UNKNOWN:              return scan_code::UNKNOWN;
    case SDL_SCANCODE_A:                    return scan_code::A;
    case SDL_SCANCODE_B:                    return scan_code::B;
    case SDL_SCANCODE_C:                    return scan_code::C;
    case SDL_SCANCODE_D:                    return scan_code::D;
    case SDL_SCANCODE_E:                    return scan_code::E;
    case SDL_SCANCODE_F:                    return scan_code::F;
    case SDL_SCANCODE_G:                    return scan_code::G;
    case SDL_SCANCODE_H:                    return scan_code::H;
    case SDL_SCANCODE_I:                    return scan_code::I;
    case SDL_SCANCODE_J:                    return scan_code::J;
    case SDL_SCANCODE_K:                    return scan_code::K;
    case SDL_SCANCODE_L:                    return scan_code::L;
    case SDL_SCANCODE_M:                    return scan_code::M;
    case SDL_SCANCODE_N:                    return scan_code::N;
    case SDL_SCANCODE_O:                    return scan_code::O;
    case SDL_SCANCODE_P:                    return scan_code::P;
    case SDL_SCANCODE_Q:                    return scan_code::Q;
    case SDL_SCANCODE_R:                    return scan_code::R;
    case SDL_SCANCODE_S:                    return scan_code::S;
    case SDL_SCANCODE_T:                    return scan_code::T;
    case SDL_SCANCODE_U:                    return scan_code::U;
    case SDL_SCANCODE_V:                    return scan_code::V;
    case SDL_SCANCODE_W:                    return scan_code::W;
    case SDL_SCANCODE_X:                    return scan_code::X;
    case SDL_SCANCODE_Y:                    return scan_code::Y;
    case SDL_SCANCODE_Z:                    return scan_code::Z;
    case SDL_SCANCODE_1:                    return scan_code::D1;
    case SDL_SCANCODE_2:                    return scan_code::D2;
    case SDL_SCANCODE_3:                    return scan_code::D3;
    case SDL_SCANCODE_4:                    return scan_code::D4;
    case SDL_SCANCODE_5:                    return scan_code::D5;
    case SDL_SCANCODE_6:                    return scan_code::D6;
    case SDL_SCANCODE_7:                    return scan_code::D7;
    case SDL_SCANCODE_8:                    return scan_code::D8;
    case SDL_SCANCODE_9:                    return scan_code::D9;
    case SDL_SCANCODE_0:                    return scan_code::D0;
    case SDL_SCANCODE_RETURN:               return scan_code::RETURN;
    case SDL_SCANCODE_ESCAPE:               return scan_code::ESCAPE;
    case SDL_SCANCODE_BACKSPACE:            return scan_code::BACKSPACE;
    case SDL_SCANCODE_TAB:                  return scan_code::TAB;
    case SDL_SCANCODE_SPACE:                return scan_code::SPACE;
    case SDL_SCANCODE_MINUS:                return scan_code::MINUS;
    case SDL_SCANCODE_EQUALS:               return scan_code::EQUALS;
    case SDL_SCANCODE_LEFTBRACKET:          return scan_code::LEFTBRACKET;
    case SDL_SCANCODE_RIGHTBRACKET:         return scan_code::RIGHTBRACKET;
    case SDL_SCANCODE_BACKSLASH:            return scan_code::BACKSLASH;
    case SDL_SCANCODE_NONUSHASH:            return scan_code::NONUSHASH;
    case SDL_SCANCODE_SEMICOLON:            return scan_code::SEMICOLON;
    case SDL_SCANCODE_APOSTROPHE:           return scan_code::APOSTROPHE;
    case SDL_SCANCODE_GRAVE:                return scan_code::GRAVE;
    case SDL_SCANCODE_COMMA:                return scan_code::COMMA;
    case SDL_SCANCODE_PERIOD:               return scan_code::PERIOD;
    case SDL_SCANCODE_SLASH:                return scan_code::SLASH;
    case SDL_SCANCODE_CAPSLOCK:             return scan_code::CAPSLOCK;
    case SDL_SCANCODE_F1:                   return scan_code::F1;
    case SDL_SCANCODE_F2:                   return scan_code::F2;
    case SDL_SCANCODE_F3:                   return scan_code::F3;
    case SDL_SCANCODE_F4:                   return scan_code::F4;
    case SDL_SCANCODE_F5:                   return scan_code::F5;
    case SDL_SCANCODE_F6:                   return scan_code::F6;
    case SDL_SCANCODE_F7:                   return scan_code::F7;
    case SDL_SCANCODE_F8:                   return scan_code::F8;
    case SDL_SCANCODE_F9:                   return scan_code::F9;
    case SDL_SCANCODE_F10:                  return scan_code::F10;
    case SDL_SCANCODE_F11:                  return scan_code::F11;
    case SDL_SCANCODE_F12:                  return scan_code::F12;
    case SDL_SCANCODE_PRINTSCREEN:          return scan_code::PRINTSCREEN;
    case SDL_SCANCODE_SCROLLLOCK:           return scan_code::SCROLLLOCK;
    case SDL_SCANCODE_PAUSE:                return scan_code::PAUSE;
    case SDL_SCANCODE_INSERT:               return scan_code::INSERT;
    case SDL_SCANCODE_HOME:                 return scan_code::HOME;
    case SDL_SCANCODE_PAGEUP:               return scan_code::PAGEUP;
    case SDL_SCANCODE_DELETE:               return scan_code::DEL;
    case SDL_SCANCODE_END:                  return scan_code::END;
    case SDL_SCANCODE_PAGEDOWN:             return scan_code::PAGEDOWN;
    case SDL_SCANCODE_RIGHT:                return scan_code::RIGHT;
    case SDL_SCANCODE_LEFT:                 return scan_code::LEFT;
    case SDL_SCANCODE_DOWN:                 return scan_code::DOWN;
    case SDL_SCANCODE_UP:                   return scan_code::UP;
    case SDL_SCANCODE_NUMLOCKCLEAR:         return scan_code::NUMLOCKCLEAR;
    case SDL_SCANCODE_KP_DIVIDE:            return scan_code::KP_DIVIDE;
    case SDL_SCANCODE_KP_MULTIPLY:          return scan_code::KP_MULTIPLY;
    case SDL_SCANCODE_KP_MINUS:             return scan_code::KP_MINUS;
    case SDL_SCANCODE_KP_PLUS:              return scan_code::KP_PLUS;
    case SDL_SCANCODE_KP_ENTER:             return scan_code::KP_ENTER;
    case SDL_SCANCODE_KP_1:                 return scan_code::KP_1;
    case SDL_SCANCODE_KP_2:                 return scan_code::KP_2;
    case SDL_SCANCODE_KP_3:                 return scan_code::KP_3;
    case SDL_SCANCODE_KP_4:                 return scan_code::KP_4;
    case SDL_SCANCODE_KP_5:                 return scan_code::KP_5;
    case SDL_SCANCODE_KP_6:                 return scan_code::KP_6;
    case SDL_SCANCODE_KP_7:                 return scan_code::KP_7;
    case SDL_SCANCODE_KP_8:                 return scan_code::KP_8;
    case SDL_SCANCODE_KP_9:                 return scan_code::KP_9;
    case SDL_SCANCODE_KP_0:                 return scan_code::KP_0;
    case SDL_SCANCODE_KP_PERIOD:            return scan_code::KP_PERIOD;
    case SDL_SCANCODE_NONUSBACKSLASH:       return scan_code::NONUSBACKSLASH;
    case SDL_SCANCODE_APPLICATION:          return scan_code::APPLICATION;
    case SDL_SCANCODE_POWER:                return scan_code::POWER;
    case SDL_SCANCODE_KP_EQUALS:            return scan_code::KP_EQUALS;
    case SDL_SCANCODE_F13:                  return scan_code::F13;
    case SDL_SCANCODE_F14:                  return scan_code::F14;
    case SDL_SCANCODE_F15:                  return scan_code::F15;
    case SDL_SCANCODE_F16:                  return scan_code::F16;
    case SDL_SCANCODE_F17:                  return scan_code::F17;
    case SDL_SCANCODE_F18:                  return scan_code::F18;
    case SDL_SCANCODE_F19:                  return scan_code::F19;
    case SDL_SCANCODE_F20:                  return scan_code::F20;
    case SDL_SCANCODE_F21:                  return scan_code::F21;
    case SDL_SCANCODE_F22:                  return scan_code::F22;
    case SDL_SCANCODE_F23:                  return scan_code::F23;
    case SDL_SCANCODE_F24:                  return scan_code::F24;
    case SDL_SCANCODE_EXECUTE:              return scan_code::EXECUTE;
    case SDL_SCANCODE_HELP:                 return scan_code::HELP;
    case SDL_SCANCODE_MENU:                 return scan_code::MENU;
    case SDL_SCANCODE_SELECT:               return scan_code::SELECT;
    case SDL_SCANCODE_STOP:                 return scan_code::STOP;
    case SDL_SCANCODE_AGAIN:                return scan_code::AGAIN;
    case SDL_SCANCODE_UNDO:                 return scan_code::UNDO;
    case SDL_SCANCODE_CUT:                  return scan_code::CUT;
    case SDL_SCANCODE_COPY:                 return scan_code::COPY;
    case SDL_SCANCODE_PASTE:                return scan_code::PASTE;
    case SDL_SCANCODE_FIND:                 return scan_code::FIND;
    case SDL_SCANCODE_MUTE:                 return scan_code::MUTE;
    case SDL_SCANCODE_VOLUMEUP:             return scan_code::VOLUMEUP;
    case SDL_SCANCODE_VOLUMEDOWN:           return scan_code::VOLUMEDOWN;
    case SDL_SCANCODE_KP_COMMA:             return scan_code::KP_COMMA;
    case SDL_SCANCODE_KP_EQUALSAS400:       return scan_code::KP_EQUALSAS400;
    case SDL_SCANCODE_INTERNATIONAL1:       return scan_code::INTERNATIONAL1;
    case SDL_SCANCODE_INTERNATIONAL2:       return scan_code::INTERNATIONAL2;
    case SDL_SCANCODE_INTERNATIONAL3:       return scan_code::INTERNATIONAL3;
    case SDL_SCANCODE_INTERNATIONAL4:       return scan_code::INTERNATIONAL4;
    case SDL_SCANCODE_INTERNATIONAL5:       return scan_code::INTERNATIONAL5;
    case SDL_SCANCODE_INTERNATIONAL6:       return scan_code::INTERNATIONAL6;
    case SDL_SCANCODE_INTERNATIONAL7:       return scan_code::INTERNATIONAL7;
    case SDL_SCANCODE_INTERNATIONAL8:       return scan_code::INTERNATIONAL8;
    case SDL_SCANCODE_INTERNATIONAL9:       return scan_code::INTERNATIONAL9;
    case SDL_SCANCODE_LANG1:                return scan_code::LANG1;
    case SDL_SCANCODE_LANG2:                return scan_code::LANG2;
    case SDL_SCANCODE_LANG3:                return scan_code::LANG3;
    case SDL_SCANCODE_LANG4:                return scan_code::LANG4;
    case SDL_SCANCODE_LANG5:                return scan_code::LANG5;
    case SDL_SCANCODE_LANG6:                return scan_code::LANG6;
    case SDL_SCANCODE_LANG7:                return scan_code::LANG7;
    case SDL_SCANCODE_LANG8:                return scan_code::LANG8;
    case SDL_SCANCODE_LANG9:                return scan_code::LANG9;
    case SDL_SCANCODE_ALTERASE:             return scan_code::ALTERASE;
    case SDL_SCANCODE_SYSREQ:               return scan_code::SYSREQ;
    case SDL_SCANCODE_CANCEL:               return scan_code::CANCEL;
    case SDL_SCANCODE_CLEAR:                return scan_code::CLEAR;
    case SDL_SCANCODE_PRIOR:                return scan_code::PRIOR;
    case SDL_SCANCODE_RETURN2:              return scan_code::RETURN2;
    case SDL_SCANCODE_SEPARATOR:            return scan_code::SEPARATOR;
    case SDL_SCANCODE_OUT:                  return scan_code::KEY_OUT;
    case SDL_SCANCODE_OPER:                 return scan_code::OPER;
    case SDL_SCANCODE_CLEARAGAIN:           return scan_code::CLEARAGAIN;
    case SDL_SCANCODE_CRSEL:                return scan_code::CRSEL;
    case SDL_SCANCODE_EXSEL:                return scan_code::EXSEL;
    case SDL_SCANCODE_KP_00:                return scan_code::KP_00;
    case SDL_SCANCODE_KP_000:               return scan_code::KP_000;
    case SDL_SCANCODE_THOUSANDSSEPARATOR:   return scan_code::THOUSANDSSEPARATOR;
    case SDL_SCANCODE_DECIMALSEPARATOR:     return scan_code::DECIMALSEPARATOR;
    case SDL_SCANCODE_CURRENCYUNIT:         return scan_code::CURRENCYUNIT;
    case SDL_SCANCODE_CURRENCYSUBUNIT:      return scan_code::CURRENCYSUBUNIT;
    case SDL_SCANCODE_KP_LEFTPAREN:         return scan_code::KP_LEFTPAREN;
    case SDL_SCANCODE_KP_RIGHTPAREN:        return scan_code::KP_RIGHTPAREN;
    case SDL_SCANCODE_KP_LEFTBRACE:         return scan_code::KP_LEFTBRACE;
    case SDL_SCANCODE_KP_RIGHTBRACE:        return scan_code::KP_RIGHTBRACE;
    case SDL_SCANCODE_KP_TAB:               return scan_code::KP_TAB;
    case SDL_SCANCODE_KP_BACKSPACE:         return scan_code::KP_BACKSPACE;
    case SDL_SCANCODE_KP_A:                 return scan_code::KP_A;
    case SDL_SCANCODE_KP_B:                 return scan_code::KP_B;
    case SDL_SCANCODE_KP_C:                 return scan_code::KP_C;
    case SDL_SCANCODE_KP_D:                 return scan_code::KP_D;
    case SDL_SCANCODE_KP_E:                 return scan_code::KP_E;
    case SDL_SCANCODE_KP_F:                 return scan_code::KP_F;
    case SDL_SCANCODE_KP_XOR:               return scan_code::KP_XOR;
    case SDL_SCANCODE_KP_POWER:             return scan_code::KP_POWER;
    case SDL_SCANCODE_KP_PERCENT:           return scan_code::KP_PERCENT;
    case SDL_SCANCODE_KP_LESS:              return scan_code::KP_LESS;
    case SDL_SCANCODE_KP_GREATER:           return scan_code::KP_GREATER;
    case SDL_SCANCODE_KP_AMPERSAND:         return scan_code::KP_AMPERSAND;
    case SDL_SCANCODE_KP_DBLAMPERSAND:      return scan_code::KP_DBLAMPERSAND;
    case SDL_SCANCODE_KP_VERTICALBAR:       return scan_code::KP_VERTICALBAR;
    case SDL_SCANCODE_KP_DBLVERTICALBAR:    return scan_code::KP_DBLVERTICALBAR;
    case SDL_SCANCODE_KP_COLON:             return scan_code::KP_COLON;
    case SDL_SCANCODE_KP_HASH:              return scan_code::KP_HASH;
    case SDL_SCANCODE_KP_SPACE:             return scan_code::KP_SPACE;
    case SDL_SCANCODE_KP_AT:                return scan_code::KP_AT;
    case SDL_SCANCODE_KP_EXCLAM:            return scan_code::KP_EXCLAM;
    case SDL_SCANCODE_KP_MEMSTORE:          return scan_code::KP_MEMSTORE;
    case SDL_SCANCODE_KP_MEMRECALL:         return scan_code::KP_MEMRECALL;
    case SDL_SCANCODE_KP_MEMCLEAR:          return scan_code::KP_MEMCLEAR;
    case SDL_SCANCODE_KP_MEMADD:            return scan_code::KP_MEMADD;
    case SDL_SCANCODE_KP_MEMSUBTRACT:       return scan_code::KP_MEMSUBTRACT;
    case SDL_SCANCODE_KP_MEMMULTIPLY:       return scan_code::KP_MEMMULTIPLY;
    case SDL_SCANCODE_KP_MEMDIVIDE:         return scan_code::KP_MEMDIVIDE;
    case SDL_SCANCODE_KP_PLUSMINUS:         return scan_code::KP_PLUSMINUS;
    case SDL_SCANCODE_KP_CLEAR:             return scan_code::KP_CLEAR;
    case SDL_SCANCODE_KP_CLEARENTRY:        return scan_code::KP_CLEARENTRY;
    case SDL_SCANCODE_KP_BINARY:            return scan_code::KP_BINARY;
    case SDL_SCANCODE_KP_OCTAL:             return scan_code::KP_OCTAL;
    case SDL_SCANCODE_KP_DECIMAL:           return scan_code::KP_DECIMAL;
    case SDL_SCANCODE_KP_HEXADECIMAL:       return scan_code::KP_HEXADECIMAL;
    case SDL_SCANCODE_LCTRL:                return scan_code::LCTRL;
    case SDL_SCANCODE_LSHIFT:               return scan_code::LSHIFT;
    case SDL_SCANCODE_LALT:                 return scan_code::LALT;
    case SDL_SCANCODE_LGUI:                 return scan_code::LGUI;
    case SDL_SCANCODE_RCTRL:                return scan_code::RCTRL;
    case SDL_SCANCODE_RSHIFT:               return scan_code::RSHIFT;
    case SDL_SCANCODE_RALT:                 return scan_code::RALT;
    case SDL_SCANCODE_RGUI:                 return scan_code::RGUI;
    case SDL_SCANCODE_MODE:                 return scan_code::MODE;
    case SDL_SCANCODE_WAKE:
    case SDL_SCANCODE_CHANNEL_INCREMENT:
    case SDL_SCANCODE_CHANNEL_DECREMENT:
    case SDL_SCANCODE_MEDIA_PAUSE:
    case SDL_SCANCODE_MEDIA_RECORD:
    case SDL_SCANCODE_MEDIA_PLAY_PAUSE:
    case SDL_SCANCODE_AC_NEW:
    case SDL_SCANCODE_AC_OPEN:
    case SDL_SCANCODE_AC_CLOSE:
    case SDL_SCANCODE_AC_EXIT:
    case SDL_SCANCODE_AC_SAVE:
    case SDL_SCANCODE_AC_PRINT:
    case SDL_SCANCODE_AC_PROPERTIES:
    case SDL_SCANCODE_RESERVED:
    case SDL_SCANCODE_SLEEP:
    case SDL_SCANCODE_MEDIA_PLAY:
    case SDL_SCANCODE_MEDIA_FAST_FORWARD:
    case SDL_SCANCODE_MEDIA_REWIND:
    case SDL_SCANCODE_MEDIA_NEXT_TRACK:
    case SDL_SCANCODE_MEDIA_PREVIOUS_TRACK:
    case SDL_SCANCODE_MEDIA_STOP:
    case SDL_SCANCODE_MEDIA_EJECT:
    case SDL_SCANCODE_MEDIA_SELECT:
    case SDL_SCANCODE_AC_SEARCH:
    case SDL_SCANCODE_AC_HOME:
    case SDL_SCANCODE_AC_BACK:
    case SDL_SCANCODE_AC_FORWARD:
    case SDL_SCANCODE_AC_STOP:
    case SDL_SCANCODE_AC_REFRESH:
    case SDL_SCANCODE_AC_BOOKMARKS:
    case SDL_SCANCODE_SOFTLEFT:
    case SDL_SCANCODE_SOFTRIGHT:
    case SDL_SCANCODE_CALL:
    case SDL_SCANCODE_ENDCALL:
    case SDL_SCANCODE_COUNT:                break;
    }
    return scan_code::UNKNOWN;
}

auto convert_enum(key_code code) -> SDL_Keycode
{
    switch (code) {
    case key_code::UNKNOWN:            return SDLK_UNKNOWN;
    case key_code::RETURN:             return SDLK_RETURN;
    case key_code::ESCAPE:             return SDLK_ESCAPE;
    case key_code::BACKSPACE:          return SDLK_BACKSPACE;
    case key_code::TAB:                return SDLK_TAB;
    case key_code::SPACE:              return SDLK_SPACE;
    case key_code::EXCLAIM:            return SDLK_EXCLAIM;
    case key_code::QUOTEDBL:           return SDLK_DBLAPOSTROPHE;
    case key_code::HASH:               return SDLK_HASH;
    case key_code::PERCENT:            return SDLK_PERCENT;
    case key_code::DOLLAR:             return SDLK_DOLLAR;
    case key_code::AMPERSAND:          return SDLK_AMPERSAND;
    case key_code::QUOTE:              return SDLK_APOSTROPHE;
    case key_code::LEFTPAREN:          return SDLK_LEFTPAREN;
    case key_code::RIGHTPAREN:         return SDLK_RIGHTPAREN;
    case key_code::ASTERISK:           return SDLK_ASTERISK;
    case key_code::PLUS:               return SDLK_PLUS;
    case key_code::COMMA:              return SDLK_COMMA;
    case key_code::MINUS:              return SDLK_MINUS;
    case key_code::PERIOD:             return SDLK_PERIOD;
    case key_code::SLASH:              return SDLK_SLASH;
    case key_code::D0:                 return SDLK_0;
    case key_code::D1:                 return SDLK_1;
    case key_code::D2:                 return SDLK_2;
    case key_code::D3:                 return SDLK_3;
    case key_code::D4:                 return SDLK_4;
    case key_code::D5:                 return SDLK_5;
    case key_code::D6:                 return SDLK_6;
    case key_code::D7:                 return SDLK_7;
    case key_code::D8:                 return SDLK_8;
    case key_code::D9:                 return SDLK_9;
    case key_code::COLON:              return SDLK_COLON;
    case key_code::SEMICOLON:          return SDLK_SEMICOLON;
    case key_code::LESS:               return SDLK_LESS;
    case key_code::EQUALS:             return SDLK_EQUALS;
    case key_code::GREATER:            return SDLK_GREATER;
    case key_code::QUESTION:           return SDLK_QUESTION;
    case key_code::AT:                 return SDLK_AT;
    case key_code::LEFTBRACKET:        return SDLK_LEFTBRACKET;
    case key_code::BACKSLASH:          return SDLK_BACKSLASH;
    case key_code::RIGHTBRACKET:       return SDLK_RIGHTBRACKET;
    case key_code::CARET:              return SDLK_CARET;
    case key_code::UNDERSCORE:         return SDLK_UNDERSCORE;
    case key_code::BACKQUOTE:          return SDLK_GRAVE;

    // Alphabet keys
    case key_code::a:                  return SDLK_A;
    case key_code::b:                  return SDLK_B;
    case key_code::c:                  return SDLK_C;
    case key_code::d:                  return SDLK_D;
    case key_code::e:                  return SDLK_E;
    case key_code::f:                  return SDLK_F;
    case key_code::g:                  return SDLK_G;
    case key_code::h:                  return SDLK_H;
    case key_code::i:                  return SDLK_I;
    case key_code::j:                  return SDLK_J;
    case key_code::k:                  return SDLK_K;
    case key_code::l:                  return SDLK_L;
    case key_code::m:                  return SDLK_M;
    case key_code::n:                  return SDLK_N;
    case key_code::o:                  return SDLK_O;
    case key_code::p:                  return SDLK_P;
    case key_code::q:                  return SDLK_Q;
    case key_code::r:                  return SDLK_R;
    case key_code::s:                  return SDLK_S;
    case key_code::t:                  return SDLK_T;
    case key_code::u:                  return SDLK_U;
    case key_code::v:                  return SDLK_V;
    case key_code::w:                  return SDLK_W;
    case key_code::x:                  return SDLK_X;
    case key_code::y:                  return SDLK_Y;
    case key_code::z:                  return SDLK_Z;

    // Function keys
    case key_code::CAPSLOCK:           return SDLK_CAPSLOCK;
    case key_code::F1:                 return SDLK_F1;
    case key_code::F2:                 return SDLK_F2;
    case key_code::F3:                 return SDLK_F3;
    case key_code::F4:                 return SDLK_F4;
    case key_code::F5:                 return SDLK_F5;
    case key_code::F6:                 return SDLK_F6;
    case key_code::F7:                 return SDLK_F7;
    case key_code::F8:                 return SDLK_F8;
    case key_code::F9:                 return SDLK_F9;
    case key_code::F10:                return SDLK_F10;
    case key_code::F11:                return SDLK_F11;
    case key_code::F12:                return SDLK_F12;

    // Control and other keys
    case key_code::PRINTSCREEN:        return SDLK_PRINTSCREEN;
    case key_code::SCROLLLOCK:         return SDLK_SCROLLLOCK;
    case key_code::PAUSE:              return SDLK_PAUSE;
    case key_code::INSERT:             return SDLK_INSERT;
    case key_code::HOME:               return SDLK_HOME;
    case key_code::PAGEUP:             return SDLK_PAGEUP;
    case key_code::DEL:                return SDLK_DELETE;
    case key_code::END:                return SDLK_END;
    case key_code::PAGEDOWN:           return SDLK_PAGEDOWN;
    case key_code::RIGHT:              return SDLK_RIGHT;
    case key_code::LEFT:               return SDLK_LEFT;
    case key_code::DOWN:               return SDLK_DOWN;
    case key_code::UP:                 return SDLK_UP;

    // Numpad keys
    case key_code::NUMLOCKCLEAR:       return SDLK_NUMLOCKCLEAR;
    case key_code::KP_DIVIDE:          return SDLK_KP_DIVIDE;
    case key_code::KP_MULTIPLY:        return SDLK_KP_MULTIPLY;
    case key_code::KP_MINUS:           return SDLK_KP_MINUS;
    case key_code::KP_PLUS:            return SDLK_KP_PLUS;
    case key_code::KP_ENTER:           return SDLK_KP_ENTER;
    case key_code::KP_1:               return SDLK_KP_1;
    case key_code::KP_2:               return SDLK_KP_2;
    case key_code::KP_3:               return SDLK_KP_3;
    case key_code::KP_4:               return SDLK_KP_4;
    case key_code::KP_5:               return SDLK_KP_5;
    case key_code::KP_6:               return SDLK_KP_6;
    case key_code::KP_7:               return SDLK_KP_7;
    case key_code::KP_8:               return SDLK_KP_8;
    case key_code::KP_9:               return SDLK_KP_9;
    case key_code::KP_0:               return SDLK_KP_0;
    case key_code::KP_PERIOD:          return SDLK_KP_PERIOD;
    case key_code::KP_COMMA:           return SDLK_KP_COMMA;
    case key_code::KP_EQUALS:          return SDLK_KP_EQUALS;
    case key_code::KP_EQUALSAS400:     return SDLK_KP_EQUALSAS400;

    // Special keys
    case key_code::APPLICATION:        return SDLK_APPLICATION;
    case key_code::POWER:              return SDLK_POWER;
    case key_code::F13:                return SDLK_F13;
    case key_code::F14:                return SDLK_F14;
    case key_code::F15:                return SDLK_F15;
    case key_code::F16:                return SDLK_F16;
    case key_code::F17:                return SDLK_F17;
    case key_code::F18:                return SDLK_F18;
    case key_code::F19:                return SDLK_F19;
    case key_code::F20:                return SDLK_F20;
    case key_code::F21:                return SDLK_F21;
    case key_code::F22:                return SDLK_F22;
    case key_code::F23:                return SDLK_F23;
    case key_code::F24:                return SDLK_F24;
    case key_code::EXECUTE:            return SDLK_EXECUTE;
    case key_code::HELP:               return SDLK_HELP;
    case key_code::MENU:               return SDLK_MENU;
    case key_code::SELECT:             return SDLK_SELECT;
    case key_code::STOP:               return SDLK_STOP;
    case key_code::AGAIN:              return SDLK_AGAIN;
    case key_code::UNDO:               return SDLK_UNDO;
    case key_code::CUT:                return SDLK_CUT;
    case key_code::COPY:               return SDLK_COPY;
    case key_code::PASTE:              return SDLK_PASTE;
    case key_code::FIND:               return SDLK_FIND;
    case key_code::MUTE:               return SDLK_MUTE;
    case key_code::VOLUMEUP:           return SDLK_VOLUMEUP;
    case key_code::VOLUMEDOWN:         return SDLK_VOLUMEDOWN;
    case key_code::ALTERASE:           return SDLK_ALTERASE;
    case key_code::SYSREQ:             return SDLK_SYSREQ;
    case key_code::CANCEL:             return SDLK_CANCEL;
    case key_code::CLEAR:              return SDLK_CLEAR;
    case key_code::PRIOR:              return SDLK_PRIOR;
    case key_code::RETURN2:            return SDLK_RETURN2;
    case key_code::SEPARATOR:          return SDLK_SEPARATOR;
    case key_code::KEY_OUT:            return SDLK_OUT;
    case key_code::OPER:               return SDLK_OPER;
    case key_code::CLEARAGAIN:         return SDLK_CLEARAGAIN;
    case key_code::CRSEL:              return SDLK_CRSEL;
    case key_code::EXSEL:              return SDLK_EXSEL;
    case key_code::KP_00:              return SDLK_KP_00;
    case key_code::KP_000:             return SDLK_KP_000;
    case key_code::THOUSANDSSEPARATOR: return SDLK_THOUSANDSSEPARATOR;
    case key_code::DECIMALSEPARATOR:   return SDLK_DECIMALSEPARATOR;
    case key_code::CURRENCYUNIT:       return SDLK_CURRENCYUNIT;
    case key_code::CURRENCYSUBUNIT:    return SDLK_CURRENCYSUBUNIT;
    case key_code::KP_LEFTPAREN:       return SDLK_KP_LEFTPAREN;
    case key_code::KP_RIGHTPAREN:      return SDLK_KP_RIGHTPAREN;
    case key_code::KP_LEFTBRACE:       return SDLK_KP_LEFTBRACE;
    case key_code::KP_RIGHTBRACE:      return SDLK_KP_RIGHTBRACE;
    case key_code::KP_TAB:             return SDLK_KP_TAB;
    case key_code::KP_BACKSPACE:       return SDLK_KP_BACKSPACE;
    case key_code::KP_A:               return SDLK_KP_A;
    case key_code::KP_B:               return SDLK_KP_B;
    case key_code::KP_C:               return SDLK_KP_C;
    case key_code::KP_D:               return SDLK_KP_D;
    case key_code::KP_E:               return SDLK_KP_E;
    case key_code::KP_F:               return SDLK_KP_F;
    case key_code::KP_XOR:             return SDLK_KP_XOR;
    case key_code::KP_POWER:           return SDLK_KP_POWER;
    case key_code::KP_PERCENT:         return SDLK_KP_PERCENT;
    case key_code::KP_LESS:            return SDLK_KP_LESS;
    case key_code::KP_GREATER:         return SDLK_KP_GREATER;
    case key_code::KP_AMPERSAND:       return SDLK_KP_AMPERSAND;
    case key_code::KP_DBLAMPERSAND:    return SDLK_KP_DBLAMPERSAND;
    case key_code::KP_VERTICALBAR:     return SDLK_KP_VERTICALBAR;
    case key_code::KP_DBLVERTICALBAR:  return SDLK_KP_DBLVERTICALBAR;
    case key_code::KP_COLON:           return SDLK_KP_COLON;
    case key_code::KP_HASH:            return SDLK_KP_HASH;
    case key_code::KP_SPACE:           return SDLK_KP_SPACE;
    case key_code::KP_AT:              return SDLK_KP_AT;
    case key_code::KP_EXCLAM:          return SDLK_KP_EXCLAM;
    case key_code::KP_MEMSTORE:        return SDLK_KP_MEMSTORE;
    case key_code::KP_MEMRECALL:       return SDLK_KP_MEMRECALL;
    case key_code::KP_MEMCLEAR:        return SDLK_KP_MEMCLEAR;
    case key_code::KP_MEMADD:          return SDLK_KP_MEMADD;
    case key_code::KP_MEMSUBTRACT:     return SDLK_KP_MEMSUBTRACT;
    case key_code::KP_MEMMULTIPLY:     return SDLK_KP_MEMMULTIPLY;
    case key_code::KP_MEMDIVIDE:       return SDLK_KP_MEMDIVIDE;
    case key_code::KP_PLUSMINUS:       return SDLK_KP_PLUSMINUS;
    case key_code::KP_CLEAR:           return SDLK_KP_CLEAR;
    case key_code::KP_CLEARENTRY:      return SDLK_KP_CLEARENTRY;
    case key_code::KP_BINARY:          return SDLK_KP_BINARY;
    case key_code::KP_OCTAL:           return SDLK_KP_OCTAL;
    case key_code::KP_DECIMAL:         return SDLK_KP_DECIMAL;
    case key_code::KP_HEXADECIMAL:     return SDLK_KP_HEXADECIMAL;
    case key_code::LCTRL:              return SDLK_LCTRL;
    case key_code::LSHIFT:             return SDLK_LSHIFT;
    case key_code::LALT:               return SDLK_LALT;
    case key_code::LGUI:               return SDLK_LGUI;
    case key_code::RCTRL:              return SDLK_RCTRL;
    case key_code::RSHIFT:             return SDLK_RSHIFT;
    case key_code::RALT:               return SDLK_RALT;
    case key_code::RGUI:               return SDLK_RGUI;
    case key_code::MODE:               return SDLK_MODE;
    }

    return SDLK_UNKNOWN;
}

auto convert_enum(SDL_Keycode code) -> key_code
{
    switch (code) {
    case SDLK_UNKNOWN:            return key_code::UNKNOWN;
    case SDLK_RETURN:             return key_code::RETURN;
    case SDLK_ESCAPE:             return key_code::ESCAPE;
    case SDLK_BACKSPACE:          return key_code::BACKSPACE;
    case SDLK_TAB:                return key_code::TAB;
    case SDLK_SPACE:              return key_code::SPACE;
    case SDLK_EXCLAIM:            return key_code::EXCLAIM;
    case SDLK_DBLAPOSTROPHE:      return key_code::QUOTEDBL;
    case SDLK_HASH:               return key_code::HASH;
    case SDLK_PERCENT:            return key_code::PERCENT;
    case SDLK_DOLLAR:             return key_code::DOLLAR;
    case SDLK_AMPERSAND:          return key_code::AMPERSAND;
    case SDLK_APOSTROPHE:         return key_code::QUOTE;
    case SDLK_LEFTPAREN:          return key_code::LEFTPAREN;
    case SDLK_RIGHTPAREN:         return key_code::RIGHTPAREN;
    case SDLK_ASTERISK:           return key_code::ASTERISK;
    case SDLK_PLUS:               return key_code::PLUS;
    case SDLK_COMMA:              return key_code::COMMA;
    case SDLK_MINUS:              return key_code::MINUS;
    case SDLK_PERIOD:             return key_code::PERIOD;
    case SDLK_SLASH:              return key_code::SLASH;
    case SDLK_0:                  return key_code::D0;
    case SDLK_1:                  return key_code::D1;
    case SDLK_2:                  return key_code::D2;
    case SDLK_3:                  return key_code::D3;
    case SDLK_4:                  return key_code::D4;
    case SDLK_5:                  return key_code::D5;
    case SDLK_6:                  return key_code::D6;
    case SDLK_7:                  return key_code::D7;
    case SDLK_8:                  return key_code::D8;
    case SDLK_9:                  return key_code::D9;
    case SDLK_COLON:              return key_code::COLON;
    case SDLK_SEMICOLON:          return key_code::SEMICOLON;
    case SDLK_LESS:               return key_code::LESS;
    case SDLK_EQUALS:             return key_code::EQUALS;
    case SDLK_GREATER:            return key_code::GREATER;
    case SDLK_QUESTION:           return key_code::QUESTION;
    case SDLK_AT:                 return key_code::AT;
    case SDLK_LEFTBRACKET:        return key_code::LEFTBRACKET;
    case SDLK_BACKSLASH:          return key_code::BACKSLASH;
    case SDLK_RIGHTBRACKET:       return key_code::RIGHTBRACKET;
    case SDLK_CARET:              return key_code::CARET;
    case SDLK_UNDERSCORE:         return key_code::UNDERSCORE;
    case SDLK_GRAVE:              return key_code::BACKQUOTE;

    // Alphabet keys
    case SDLK_A:                  return key_code::a;
    case SDLK_B:                  return key_code::b;
    case SDLK_C:                  return key_code::c;
    case SDLK_D:                  return key_code::d;
    case SDLK_E:                  return key_code::e;
    case SDLK_F:                  return key_code::f;
    case SDLK_G:                  return key_code::g;
    case SDLK_H:                  return key_code::h;
    case SDLK_I:                  return key_code::i;
    case SDLK_J:                  return key_code::j;
    case SDLK_K:                  return key_code::k;
    case SDLK_L:                  return key_code::l;
    case SDLK_M:                  return key_code::m;
    case SDLK_N:                  return key_code::n;
    case SDLK_O:                  return key_code::o;
    case SDLK_P:                  return key_code::p;
    case SDLK_Q:                  return key_code::q;
    case SDLK_R:                  return key_code::r;
    case SDLK_S:                  return key_code::s;
    case SDLK_T:                  return key_code::t;
    case SDLK_U:                  return key_code::u;
    case SDLK_V:                  return key_code::v;
    case SDLK_W:                  return key_code::w;
    case SDLK_X:                  return key_code::x;
    case SDLK_Y:                  return key_code::y;
    case SDLK_Z:                  return key_code::z;

    // Function keys
    case SDLK_CAPSLOCK:           return key_code::CAPSLOCK;
    case SDLK_F1:                 return key_code::F1;
    case SDLK_F2:                 return key_code::F2;
    case SDLK_F3:                 return key_code::F3;
    case SDLK_F4:                 return key_code::F4;
    case SDLK_F5:                 return key_code::F5;
    case SDLK_F6:                 return key_code::F6;
    case SDLK_F7:                 return key_code::F7;
    case SDLK_F8:                 return key_code::F8;
    case SDLK_F9:                 return key_code::F9;
    case SDLK_F10:                return key_code::F10;
    case SDLK_F11:                return key_code::F11;
    case SDLK_F12:                return key_code::F12;

    // Control and other keys
    case SDLK_PRINTSCREEN:        return key_code::PRINTSCREEN;
    case SDLK_SCROLLLOCK:         return key_code::SCROLLLOCK;
    case SDLK_PAUSE:              return key_code::PAUSE;
    case SDLK_INSERT:             return key_code::INSERT;
    case SDLK_HOME:               return key_code::HOME;
    case SDLK_PAGEUP:             return key_code::PAGEUP;
    case SDLK_DELETE:             return key_code::DEL;
    case SDLK_END:                return key_code::END;
    case SDLK_PAGEDOWN:           return key_code::PAGEDOWN;
    case SDLK_RIGHT:              return key_code::RIGHT;
    case SDLK_LEFT:               return key_code::LEFT;
    case SDLK_DOWN:               return key_code::DOWN;
    case SDLK_UP:                 return key_code::UP;

    // Numpad keys
    case SDLK_NUMLOCKCLEAR:       return key_code::NUMLOCKCLEAR;
    case SDLK_KP_DIVIDE:          return key_code::KP_DIVIDE;
    case SDLK_KP_MULTIPLY:        return key_code::KP_MULTIPLY;
    case SDLK_KP_MINUS:           return key_code::KP_MINUS;
    case SDLK_KP_PLUS:            return key_code::KP_PLUS;
    case SDLK_KP_ENTER:           return key_code::KP_ENTER;
    case SDLK_KP_1:               return key_code::KP_1;
    case SDLK_KP_2:               return key_code::KP_2;
    case SDLK_KP_3:               return key_code::KP_3;
    case SDLK_KP_4:               return key_code::KP_4;
    case SDLK_KP_5:               return key_code::KP_5;
    case SDLK_KP_6:               return key_code::KP_6;
    case SDLK_KP_7:               return key_code::KP_7;
    case SDLK_KP_8:               return key_code::KP_8;
    case SDLK_KP_9:               return key_code::KP_9;
    case SDLK_KP_0:               return key_code::KP_0;
    case SDLK_KP_PERIOD:          return key_code::KP_PERIOD;
    case SDLK_KP_COMMA:           return key_code::KP_COMMA;
    case SDLK_KP_EQUALS:          return key_code::KP_EQUALS;
    case SDLK_KP_EQUALSAS400:     return key_code::KP_EQUALSAS400;

    // Special keys
    case SDLK_APPLICATION:        return key_code::APPLICATION;
    case SDLK_POWER:              return key_code::POWER;
    case SDLK_F13:                return key_code::F13;
    case SDLK_F14:                return key_code::F14;
    case SDLK_F15:                return key_code::F15;
    case SDLK_F16:                return key_code::F16;
    case SDLK_F17:                return key_code::F17;
    case SDLK_F18:                return key_code::F18;
    case SDLK_F19:                return key_code::F19;
    case SDLK_F20:                return key_code::F20;
    case SDLK_F21:                return key_code::F21;
    case SDLK_F22:                return key_code::F22;
    case SDLK_F23:                return key_code::F23;
    case SDLK_F24:                return key_code::F24;
    case SDLK_EXECUTE:            return key_code::EXECUTE;
    case SDLK_HELP:               return key_code::HELP;
    case SDLK_MENU:               return key_code::MENU;
    case SDLK_SELECT:             return key_code::SELECT;
    case SDLK_STOP:               return key_code::STOP;
    case SDLK_AGAIN:              return key_code::AGAIN;
    case SDLK_UNDO:               return key_code::UNDO;
    case SDLK_CUT:                return key_code::CUT;
    case SDLK_COPY:               return key_code::COPY;
    case SDLK_PASTE:              return key_code::PASTE;
    case SDLK_FIND:               return key_code::FIND;
    case SDLK_MUTE:               return key_code::MUTE;
    case SDLK_VOLUMEUP:           return key_code::VOLUMEUP;
    case SDLK_VOLUMEDOWN:         return key_code::VOLUMEDOWN;
    case SDLK_ALTERASE:           return key_code::ALTERASE;
    case SDLK_SYSREQ:             return key_code::SYSREQ;
    case SDLK_CANCEL:             return key_code::CANCEL;
    case SDLK_CLEAR:              return key_code::CLEAR;
    case SDLK_PRIOR:              return key_code::PRIOR;
    case SDLK_RETURN2:            return key_code::RETURN2;
    case SDLK_SEPARATOR:          return key_code::SEPARATOR;
    case SDLK_OUT:                return key_code::KEY_OUT;
    case SDLK_OPER:               return key_code::OPER;
    case SDLK_CLEARAGAIN:         return key_code::CLEARAGAIN;
    case SDLK_CRSEL:              return key_code::CRSEL;
    case SDLK_EXSEL:              return key_code::EXSEL;
    case SDLK_KP_00:              return key_code::KP_00;
    case SDLK_KP_000:             return key_code::KP_000;
    case SDLK_THOUSANDSSEPARATOR: return key_code::THOUSANDSSEPARATOR;
    case SDLK_DECIMALSEPARATOR:   return key_code::DECIMALSEPARATOR;
    case SDLK_CURRENCYUNIT:       return key_code::CURRENCYUNIT;
    case SDLK_CURRENCYSUBUNIT:    return key_code::CURRENCYSUBUNIT;
    case SDLK_KP_LEFTPAREN:       return key_code::KP_LEFTPAREN;
    case SDLK_KP_RIGHTPAREN:      return key_code::KP_RIGHTPAREN;
    case SDLK_KP_LEFTBRACE:       return key_code::KP_LEFTBRACE;
    case SDLK_KP_RIGHTBRACE:      return key_code::KP_RIGHTBRACE;
    case SDLK_KP_TAB:             return key_code::KP_TAB;
    case SDLK_KP_BACKSPACE:       return key_code::KP_BACKSPACE;
    case SDLK_KP_A:               return key_code::KP_A;
    case SDLK_KP_B:               return key_code::KP_B;
    case SDLK_KP_C:               return key_code::KP_C;
    case SDLK_KP_D:               return key_code::KP_D;
    case SDLK_KP_E:               return key_code::KP_E;
    case SDLK_KP_F:               return key_code::KP_F;
    case SDLK_KP_XOR:             return key_code::KP_XOR;
    case SDLK_KP_POWER:           return key_code::KP_POWER;
    case SDLK_KP_PERCENT:         return key_code::KP_PERCENT;
    case SDLK_KP_LESS:            return key_code::KP_LESS;
    case SDLK_KP_GREATER:         return key_code::KP_GREATER;
    case SDLK_KP_AMPERSAND:       return key_code::KP_AMPERSAND;
    case SDLK_KP_DBLAMPERSAND:    return key_code::KP_DBLAMPERSAND;
    case SDLK_KP_VERTICALBAR:     return key_code::KP_VERTICALBAR;
    case SDLK_KP_DBLVERTICALBAR:  return key_code::KP_DBLVERTICALBAR;
    case SDLK_KP_COLON:           return key_code::KP_COLON;
    case SDLK_KP_HASH:            return key_code::KP_HASH;
    case SDLK_KP_SPACE:           return key_code::KP_SPACE;
    case SDLK_KP_AT:              return key_code::KP_AT;
    case SDLK_KP_EXCLAM:          return key_code::KP_EXCLAM;
    case SDLK_KP_MEMSTORE:        return key_code::KP_MEMSTORE;
    case SDLK_KP_MEMRECALL:       return key_code::KP_MEMRECALL;
    case SDLK_KP_MEMCLEAR:        return key_code::KP_MEMCLEAR;
    case SDLK_KP_MEMADD:          return key_code::KP_MEMADD;
    case SDLK_KP_MEMSUBTRACT:     return key_code::KP_MEMSUBTRACT;
    case SDLK_KP_MEMMULTIPLY:     return key_code::KP_MEMMULTIPLY;
    case SDLK_KP_MEMDIVIDE:       return key_code::KP_MEMDIVIDE;
    case SDLK_KP_PLUSMINUS:       return key_code::KP_PLUSMINUS;
    case SDLK_KP_CLEAR:           return key_code::KP_CLEAR;
    case SDLK_KP_CLEARENTRY:      return key_code::KP_CLEARENTRY;
    case SDLK_KP_BINARY:          return key_code::KP_BINARY;
    case SDLK_KP_OCTAL:           return key_code::KP_OCTAL;
    case SDLK_KP_DECIMAL:         return key_code::KP_DECIMAL;
    case SDLK_KP_HEXADECIMAL:     return key_code::KP_HEXADECIMAL;
    case SDLK_LCTRL:              return key_code::LCTRL;
    case SDLK_LSHIFT:             return key_code::LSHIFT;
    case SDLK_LALT:               return key_code::LALT;
    case SDLK_LGUI:               return key_code::LGUI;
    case SDLK_RCTRL:              return key_code::RCTRL;
    case SDLK_RSHIFT:             return key_code::RSHIFT;
    case SDLK_RALT:               return key_code::RALT;
    case SDLK_RGUI:               return key_code::RGUI;
    case SDLK_MODE:               return key_code::MODE;
    }
    return key_code::UNKNOWN;
}
}
