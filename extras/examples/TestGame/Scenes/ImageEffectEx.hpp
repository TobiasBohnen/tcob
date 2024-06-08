// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"
#include "tcob/gfx/Material.hpp"
#include <memory>

class ImageEffectEx : public scene {
public:
    ImageEffectEx(game& game);
    ~ImageEffectEx() override;

protected:
    void on_start() override;

    void on_draw_to(render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(keyboard::event& ev) override;
    void on_mouse_motion(mouse::motion_event& ev) override;

private:
    sprite_batch               _layer1;
    manual_asset_ptr<material> _mat0 {};
    manual_asset_ptr<texture>  _tex0 {};
};
