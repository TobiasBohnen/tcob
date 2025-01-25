// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/input/Input.hpp"

#include <SDL.h>

namespace tcob::input {

auto convert_mouse_button(i32 button) -> mouse::button;
auto convert_enum(mouse::button button) -> i32;

auto convert_joystick_hat(i32 hat) -> joystick::hat;

auto convert_enum(controller::button button) -> SDL_GameControllerButton;
auto convert_enum(SDL_GameControllerButton button) -> controller::button;

auto convert_enum(controller::axis axis) -> SDL_GameControllerAxis;
auto convert_enum(SDL_GameControllerAxis axis) -> controller::axis;

auto convert_enum(key_mod mod) -> SDL_Keymod;
auto convert_enum(SDL_Keymod mod) -> key_mod;

auto convert_enum(scan_code code) -> SDL_Scancode;
auto convert_enum(SDL_Scancode code) -> scan_code;

auto convert_enum(key_code code) -> SDL_Keycode;
auto convert_enum(SDL_Keycode code) -> key_code;

}
