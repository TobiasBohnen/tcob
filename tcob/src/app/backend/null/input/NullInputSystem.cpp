// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "NullInputSystem.hpp"

#include <cassert>
#include <memory>
#include <unordered_map>

#include "tcob/core/Point.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"

namespace tcob::input::null {

auto null_controller::has_rumble() const -> bool { return false; }
auto null_controller::rumble(u16, u16, milliseconds) const -> bool { return false; }
auto null_controller::has_rumble_triggers() const -> bool { return false; }
auto null_controller::rumble_triggers(u16, u16, milliseconds) const -> bool { return false; }
auto null_controller::id() const -> u32 { return 0; }
auto null_controller::name() const -> string { return "NULL"; }
auto null_controller::is_button_pressed(button) const -> bool { return false; }
auto null_controller::has_button(button) const -> bool { return false; }
auto null_controller::get_button_name(button) const -> string { return "NULL"; }
auto null_controller::get_button_label(button) const -> button_label { return button_label::Invalid; }
auto null_controller::get_axis_value(axis) const -> i16 { return 0; }
auto null_controller::has_axis(axis) const -> bool { return false; }
auto null_controller::get_axis_name(axis) const -> string { return "NULL"; }
auto null_keyboard::get_scancode(key_code) const -> scan_code { return scan_code::N; }
auto null_keyboard::get_keycode(scan_code) const -> key_code { return key_code::n; }
auto null_keyboard::is_key_down(scan_code) const -> bool { return false; }
auto null_keyboard::is_key_down(key_code) const -> bool { return false; }
auto null_keyboard::is_mod_down(key_mod) const -> bool { return false; }
auto null_keyboard::mods() const -> key_mods { return key_mods {key_mod::None}; }
auto null_mouse::get_position() const -> point_i { return point_i::Zero; }
void null_mouse::set_position(point_i) const { }
auto null_mouse::is_button_down(button) const -> bool { return false; }
auto null_clipboard::has_text() const -> bool { return false; }
auto null_clipboard::get_text() const -> utf8_string { return "NULL"; }
void null_clipboard::set_text(utf8_string const&) { }
auto null_input_system::controllers() const -> std::unordered_map<i32, std::shared_ptr<controller>> const&
{
    static std::unordered_map<i32, std::shared_ptr<controller>> c;
    return c;
}
auto null_input_system::mouse() const -> std::shared_ptr<input::mouse> { return std::make_shared<null_mouse>(); }
auto null_input_system::keyboard() const -> std::shared_ptr<input::keyboard> { return std::make_shared<null_keyboard>(); }
auto null_input_system::clipboard() const -> std::shared_ptr<input::clipboard> { return std::make_shared<null_clipboard>(); }
void null_input_system::process_events(void*) { }
}
