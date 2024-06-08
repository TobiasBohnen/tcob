// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "../Common.hpp"

class AutomationEx : public scene {
public:
    AutomationEx(game& game);
    ~AutomationEx() override;

protected:
    void on_start() override;

    void on_draw_to(render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(keyboard::event& ev) override;
    void on_mouse_motion(mouse::motion_event& ev) override;

private:
    template <typename func1>
    void create_circle(asset_ptr<material> const& circleMat, point_f position, color color, func1 auto0)
    {
        auto circle {_layer1.create_sprite()};
        circle->Material = circleMat;
        circle->Bounds   = {position, {60.f, 60.f}};
        circle->Color    = color;

        auto0->Value.Changed.connect([circle](point_f point) { circle->Bounds = circle->Bounds->with_position(point); });

        _tweens.push_back(auto0);
    }

    sprite_batch                             _layer1;
    std::vector<std::shared_ptr<tween_base>> _tweens;
};
