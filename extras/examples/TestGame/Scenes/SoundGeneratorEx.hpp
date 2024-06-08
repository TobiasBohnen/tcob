// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "../Common.hpp"

class SoundGeneratorEx : public scene {
public:
    SoundGeneratorEx(game& game);
    ~SoundGeneratorEx() override;

protected:
    void on_start() override;

    void on_draw_to(render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(keyboard::event& ev) override;
    void on_mouse_motion(mouse::motion_event& ev) override;

private:
    void         draw_wave();
    sprite_batch _layer1;

    sound_generator                    _gen1;
    sound_wave                         _wave1;
    buffer                             _audioData;
    sound                              _sound1;
    bool                               _drawWave {true};
    assets::manual_asset_ptr<material> _material {};

    canvas _canvas;
};
