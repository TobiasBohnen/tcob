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

class skin {
public:
    skin(std::string const& color, assets::group const& resGrp);

    nine_patch Button00;
    nine_patch Button01;
    nine_patch Button02;
    nine_patch Button03;
    nine_patch Button04;
    nine_patch Button05;
    nine_patch Button06;
    nine_patch Button13;
    nine_patch BoxCheckmark;
    nine_patch BoxCross;
    nine_patch BoxTick;
    nine_patch Circle;
    nine_patch Checkmark;
    nine_patch Cross;
    nine_patch SliderUp;
    nine_patch SliderDown;
    nine_patch SliderRight;
    nine_patch Panel;
};

auto create_skinned_styles(assets::group const& resGrp) -> style_collection;
