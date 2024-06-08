// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TemplateScene.hpp"
#include <iomanip>

TemplateScene::TemplateScene(game& game)
    : scene(game)
{
}

TemplateScene::~TemplateScene() = default;

void TemplateScene::on_start()
{
}

void TemplateScene::on_draw_to(render_target& target)
{
    layer1.draw_to(target);
}

void TemplateScene::on_update(milliseconds deltaTime)
{
    layer1.update(deltaTime);
}

void TemplateScene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << locate_service<stats>().get_average_FPS();
    stream << " best FPS:" << locate_service<stats>().get_best_FPS();
    stream << " worst FPS:" << locate_service<stats>().get_worst_FPS();

    get_window().Title = "TestGame " + stream.str();
}

void TemplateScene::on_key_down(keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
    }
}

void TemplateScene::on_mouse_motion(mouse::motion_event& ev)
{
}