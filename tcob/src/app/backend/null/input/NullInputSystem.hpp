// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <unordered_map>

#include "tcob/core/Point.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"

////////////////////////////////////////////////////////////

namespace tcob::input::null {
////////////////////////////////////////////////////////////

class TCOB_API null_keyboard final : public keyboard {
public:
    auto get_scancode(key_code key) const -> scan_code override;
    auto get_keycode(scan_code key) const -> key_code override;

    auto is_key_down(scan_code key) const -> bool override;
    auto is_key_down(key_code key) const -> bool override;
    auto is_mod_down(key_mod mod) const -> bool override;

    auto mods() const -> key_mods override;
};

////////////////////////////////////////////////////////////

class TCOB_API null_mouse final : public mouse {
public:
    auto get_position() const -> point_i override; // TODO: set_get_
    void set_position(point_i pos) const override;
    auto is_button_down(button button) const -> bool override;
};

////////////////////////////////////////////////////////////

class TCOB_API null_controller final : public controller {
public:
    auto id() const -> u32 override;
    auto name() const -> string override;

    auto has_rumble() const -> bool override;
    auto rumble(u16 lowFrequencyRumble, u16 highFrequencyRumble, milliseconds duration) const -> bool override;

    auto has_rumble_triggers() const -> bool override;
    auto rumble_triggers(u16 leftRumble, u16 rightRumble, milliseconds duration) const -> bool override;

    auto is_button_pressed(button b) const -> bool override;
    auto has_button(button b) const -> bool override;
    auto get_button_name(button b) const -> string override;
    auto get_button_label(button b) const -> button_label override;

    auto get_axis_value(axis a) const -> i16 override;
    auto has_axis(axis a) const -> bool override;
    auto get_axis_name(axis a) const -> string override;
};

////////////////////////////////////////////////////////////

class TCOB_API null_clipboard final : public clipboard {
public:
    auto has_text() const -> bool override;
    auto get_text() const -> utf8_string override;
    void set_text(utf8_string const& text) override;
};

////////////////////////////////////////////////////////////

class TCOB_API null_input_system final : public system {
public:
    auto controllers() const -> std::unordered_map<i32, std::shared_ptr<controller>> const& override;

    auto mouse() const -> input::mouse& override;

    auto keyboard() const -> input::keyboard& override;

    auto clipboard() const -> input::clipboard& override;

    void process_events(void* ev) override;
};

////////////////////////////////////////////////////////////

}
