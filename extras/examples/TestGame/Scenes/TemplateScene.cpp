#include "TemplateScene.hpp"
#include "../StartScene.hpp"
#include <iomanip>
TemplateScene::TemplateScene(Game& game)
    : Scene(game)
{
}

TemplateScene::~TemplateScene()
{
}

void TemplateScene::on_start()
{
}

void TemplateScene::on_draw(RenderTarget& target)
{
    layer1.draw(target);
}

void TemplateScene::on_update(f64 deltaTime)
{
    layer1.update(deltaTime);
}

void TemplateScene::on_fixed_update(f64 deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().stats().average_fps();
    stream << " best FPS:" << game().stats().best_fps();
    stream << " worst FPS:" << game().stats().worst_fps();

    game().window().title("TestGame " + stream.str());
}

void TemplateScene::on_key_down(const KeyboardEvent& ev)
{
    switch (ev.Code) {
    case Scancode::BACKSPACE:
        game().pop_current_scene();
        break;
    default:
        break;
    }
}

void TemplateScene::on_mouse_motion(const MouseMotionEvent& ev)
{
}