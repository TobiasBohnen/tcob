// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/input/Input.hpp"

#include <cassert>

namespace tcob::input {

system::system()
{
    InputMode = mode::KeyboardMouse;
}

auto system::has_controller() const -> bool { return !controllers().empty(); }

auto system::first_controller() const -> controller&
{
    return *controllers().begin()->second;
}

////////////////////////////////////////////////////////////

key_mods::key_mods(key_mod mod)
    : _mod {mod}
{
}

auto key_mods::num_lock() const -> bool
{
    return is_down(key_mod::NumLock);
}

auto key_mods::caps_lock() const -> bool
{
    return is_down(key_mod::CapsLock);
}

auto key_mods::mode() const -> bool
{
    return is_down(key_mod::Mode);
}

auto key_mods::scroll() const -> bool
{
    return is_down(key_mod::Scroll);
}

auto key_mods::control() const -> bool
{
    return left_control() || right_control();
}

auto key_mods::left_control() const -> bool
{
    return is_down(key_mod::LeftControl);
}

auto key_mods::right_control() const -> bool
{
    return is_down(key_mod::RightControl);
}

auto key_mods::shift() const -> bool
{
    return left_shift() || right_shift();
}

auto key_mods::left_shift() const -> bool
{
    return is_down(key_mod::LeftShift);
}

auto key_mods::right_shift() const -> bool
{
    return is_down(key_mod::RightShift);
}

auto key_mods::alt() const -> bool
{
    return left_alt() || right_alt();
}

auto key_mods::left_alt() const -> bool
{
    return is_down(key_mod::LeftAlt);
}

auto key_mods::right_alt() const -> bool
{
    return is_down(key_mod::RightAlt);
}

auto key_mods::gui() const -> bool
{
    return left_gui() || right_gui();
}

auto key_mods::left_gui() const -> bool
{
    return is_down(key_mod::LeftGui);
}

auto key_mods::right_gui() const -> bool
{
    return is_down(key_mod::RightGui);
}

auto key_mods::is_down(key_mod mod) const -> bool
{
    using namespace tcob::enum_ops;
    return (_mod & mod) == mod;
}

}
