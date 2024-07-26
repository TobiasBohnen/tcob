// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AutomationEx.hpp"
#include <iomanip>

AutomationEx::AutomationEx(game& game)
    : scene(game)
{
}

AutomationEx::~AutomationEx() = default;

milliseconds duration {5000};

void AutomationEx::on_start()
{
    auto* resGrp {get_game().get_library().get_group("res")};

    auto          circleMat {resGrp->get<material>("mat-circle")};
    f32           endX {800 - 60};
    point_f       position {0, -65};
    point_f const posOffset {position.X, -position.Y};
    {
        position += posOffset;
        auto auto0 {make_shared_tween<linear_tween<point_f>>(duration, position, point_f {endX, position.Y})};
        create_circle(circleMat, position, colors::Blue, auto0);
    }

    {
        position += posOffset;
        auto auto0 {make_shared_tween<smoothstep_tween<point_f>>(duration, position, point_f {endX, position.Y})};
        create_circle(circleMat, position, colors::Red, auto0);
    }

    {
        position += posOffset;
        auto auto0 {make_shared_tween<smootherstep_tween<point_f>>(duration, position, point_f {endX, position.Y})};
        create_circle(circleMat, position, colors::Yellow, auto0);
    }

    {
        position += posOffset;
        auto auto0 {make_shared_tween<sine_wave_tween<point_f>>(duration, position, point_f {endX, position.Y}, 0.5f, 0.f)};
        create_circle(circleMat, position, colors::Orange, auto0);
    }

    {
        position += posOffset;
        auto auto0 {make_shared_tween<power_tween<point_f>>(duration, position, point_f {endX, position.Y}, 0.75f)};
        create_circle(circleMat, position, colors::LawnGreen, auto0);
    }

    {
        position += posOffset;
        auto auto0 {make_shared_tween<inverse_power_tween<point_f>>(duration, position, point_f {endX, position.Y}, 2.f)};
        create_circle(circleMat, position, colors::Brown, auto0);
    }

    {
        position += posOffset;
        auto auto0 {make_shared_tween<power_tween<point_f>>(duration, position, point_f {endX, position.Y}, 2.f)};
        create_circle(circleMat, position, colors::Green, auto0);
    }

    {
        position += posOffset;
        auto auto0 {make_shared_tween<inverse_power_tween<point_f>>(duration, position, point_f {endX, position.Y}, 0.5f)};
        create_circle(circleMat, position, colors::Orchid, auto0);
    }

    {
        position += posOffset;
        auto auto0 {make_shared_tween<square_wave_tween<point_f>>(duration, position, point_f {endX, position.Y}, 1.f, 0.f)};
        create_circle(circleMat, position, colors::RebeccaPurple, auto0);
    }

    ////////////////////////////////////////////////////////////
    for (auto& tween : _tweens) {
        tween->start(playback_style::AlternatedLooped);
    }
}

void AutomationEx::on_draw_to(render_target& target)
{
    _layer1.draw_to(target);
}

void AutomationEx::on_update(milliseconds deltaTime)
{
    for (auto& tween : _tweens) {
        tween->update(deltaTime);
    }
    _layer1.update(deltaTime);
}

void AutomationEx::on_fixed_update(milliseconds deltaTime)
{

    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().get_stats()};
    stream << "avg FPS:" << stats.get_average_FPS();
    stream << " best FPS:" << stats.get_best_FPS();
    stream << " worst FPS:" << stats.get_worst_FPS();

    get_window().Title = "TestGame " + stream.str();
}

void AutomationEx::on_key_down(keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case scan_code::D1:
        for (auto& auto0 : _tweens) {
            auto0->Interval = duration / 2;
        }
        break;
    case scan_code::D2:
        for (auto& auto0 : _tweens) {
            auto0->Interval = duration / 5;
        }
        break;
    case scan_code::D3:
        for (auto& auto0 : _tweens) {
            auto0->Interval = duration / 10;
        }
        break;
    case scan_code::D4:
        for (auto& auto0 : _tweens) {
            auto0->Interval = duration / 50;
        }
        break;
    case scan_code::D5:
        for (auto& auto0 : _tweens) {
            auto0->Interval = duration / 100;
        }
        break;
    case scan_code::D6:
        for (auto& auto0 : _tweens) {
            auto0->Interval = milliseconds {0};
        }
        break;
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
    }
}

void AutomationEx::on_mouse_motion(mouse::motion_event& ev)
{
}
