// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gfx;
using namespace tcob::gfx::ui;
using namespace tcob::input;

auto create_form0(window* wnd) -> std::shared_ptr<form>;
auto create_form1(window* wnd) -> std::shared_ptr<form>;

auto create_form_displays(window* wnd) -> std::shared_ptr<form>;
auto create_form_tabcontainer(window* wnd) -> std::shared_ptr<form>;
auto create_form_accordion(window* wnd) -> std::shared_ptr<form>;
auto create_form_colorpicker(window* wnd) -> std::shared_ptr<form>;