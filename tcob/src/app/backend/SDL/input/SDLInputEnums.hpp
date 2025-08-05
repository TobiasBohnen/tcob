// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"

#include <SDL3/SDL.h>

namespace tcob::input {

auto convert_mouse_button(i32 button) -> mouse::button;
auto convert_enum(mouse::button button) -> i32;

auto convert_enum(controller::button button) -> SDL_GamepadButton;
auto convert_enum(SDL_GamepadButton button) -> controller::button;

auto convert_enum(SDL_GamepadButtonLabel label) -> controller::button_label;

auto convert_enum(controller::axis axis) -> SDL_GamepadAxis;
auto convert_enum(SDL_GamepadAxis axis) -> controller::axis;

auto convert_enum(key_mod mod) -> SDL_Keymod;
auto convert_enum(SDL_Keymod mod) -> key_mod;

auto convert_enum(scan_code code) -> SDL_Scancode;
auto convert_enum(SDL_Scancode code) -> scan_code;

auto convert_enum(key_code code) -> SDL_Keycode;
auto convert_enum(SDL_Keycode code) -> key_code;

}
