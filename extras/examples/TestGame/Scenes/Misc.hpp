// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "../Common.hpp" // IWYU pragma: keep
#include <iomanip>
#include <iostream>

////////////////////////////////////////////////////////////

class simple_entity : public entity { // TODO: rename
public:
    std::shared_ptr<drawable> Drawable;

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;
};

////////////////////////////////////////////////////////////

class MiscScene : public scene {
public:
    MiscScene(game& game);
    ~MiscScene() override;

protected:
    void on_start() override;
    void on_finish() override;

    void on_draw_to(render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(keyboard::event& ev) override;
    void on_mouse_motion(mouse::motion_event& ev) override;

private:
    void draw_dice(auto&& dice)
    {
        auto               rolls {dice.roll_n(100'000)};
        std::map<i32, i32> hist;
        for (int n = 0; n < 100'000; ++n) {
            ++hist[rolls[n]];
        }
        for (auto p : hist) {
            std::cout << std::setfill('0') << std::setw(2) << p.first << ' ' << std::string(p.second / 250, '*') << '\n';
        }
    }

    void draw_rng(auto&& rng, f32 scale = 1.0)
    {
        std::map<f32, i32> hist;
        for (int n = 0; n < 10'000; ++n) {
            ++hist[static_cast<i32>(rng() * scale)];
        }
        for (auto p : hist) {
            if (p.second / 100 > 0) {
                std::cout << std::setfill('0') << std::setw(2) << p.first << ' ' << std::string(p.second / 100, '*') << '\n';
            }
        }
    }

    sprite_batch _layer0;

    std::shared_ptr<particle_system> _particleSystem0 {std::make_shared<particle_system>()};
    assets::asset_ptr<font>          _font;

    std::shared_ptr<text> _text;

    std::shared_ptr<static_sprite_batch> _layer1;

    std::shared_ptr<sprite> _aniTexSprite;

    polygon_renderer _poly {buffer_usage_hint::StaticDraw};

    tweening::queue  _rvc;
    std::atomic_bool _flag {false};

    size_i                       _numPoints {320, 240};
    std::vector<vec2>            _points;
    std::shared_ptr<point_cloud> _pointCloud {std::make_shared<point_cloud>(_numPoints.Width * _numPoints.Height)};
    smoothstep_tween<point_f>    _pointTween {milliseconds {2000}, {point_f {0, -50.f}, point_f {0, 50.f}}};
    usize                        _pointIdx {0};

    sound _sound_mp3;
    sound _sound_opus;
    sound _sound_wav;
    sound _sound_ogg;
    sound _sound_flac;
    sound _sound_it;

    assets::asset_ptr<sound_font> _soundFont0;
    std::unique_ptr<music>        _midi0;
    sound                         _midi1;

    std::shared_ptr<html::document> _htmlDoc;

    assets::asset_ptr<music> _music0;

    timer                        _timer;
    std::unique_ptr<lua::script> _soundScript;

    uniform_buffer _uniBuf {sizeof(vec4) + sizeof(f32)};
    uniform_buffer _uniPoints {sizeof(uvec2)};

    bool _buttonEnableSwitch {true};

    std::shared_ptr<camera>  _rq1Cam {std::make_shared<camera>()};
    std::shared_ptr<tilemap> _tileMap {};
};
