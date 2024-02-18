// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "../Common.hpp"

class CanvasEx : public scene {
public:
    CanvasEx(game& game);
    ~CanvasEx() override;

protected:
    void on_start() override;

    void on_draw_to(render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(keyboard::event& ev) override;
    void on_mouse_motion(mouse::motion_event& ev) override;

private:
    void         prepare_canvas();
    void         paint_to_canvas();
    texture*     Image{0};
    texture*     NP1{0};
    canvas_paint ImagePattern;
    canvas_paint LinearGradient;
    canvas_paint BoxGradient;
    canvas_paint RadialGradient0;
    canvas_paint RadialGradient1;

    canvas _canvas;

    sprite_batch                       _layer1;
    assets::manual_asset_ptr<material> _material{};
    tweening::triange_wave_tween<f32>  _ninePatchTween;
};
