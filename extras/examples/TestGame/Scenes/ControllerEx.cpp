#include "ControllerEx.hpp"
#include "../StartScene.hpp"
#include <iomanip>

using namespace std::chrono_literals;

ControllerEx::ControllerEx(Game& game)
    : Scene(game)
{
}

ControllerEx::~ControllerEx()
{
}

void ControllerEx::on_start()
{
    if (game().input().controller_count() == 0) {
        game().pop_current_scene();
        return;
    }
    auto& resMgr { game().resources() };

    _text.text("1: High freq \n"
               "2: Low freq \n"
               "3: both \n");
    _text.bounds({ { 0.05f, 0.1f }, { 0.75f, 1.75f } });
    _text.outline_thickness(0.5f);
    _text.outline_color(Colors::Black);

    _controllerDesc.outline_thickness(0.5f);
    _controllerDesc.outline_color(Colors::Black);
    _controllerDesc.bounds({ { 0.55f, 0.1f }, { 1.25f, 0.75f } });
    _controllerDesc.pivot({ 0.55f, 0.1f }, false);
    std::stringstream stream;
    stream << "Controller count: " << game().input().controller_count() << "\n";
    stream << "Name of controller 0:" << game().input().controller_at(0).name();
    _controllerDesc.text(stream.str());

    _controllerDesc.scale({ 0.75f, 0.75f });
}

void ControllerEx::on_draw(RenderTarget& target)
{
    _text.draw(target);
    _controllerDesc.draw(target);
}

void ControllerEx::on_update(MilliSeconds deltaTime)
{
    _text.update(deltaTime);
    _controllerDesc.update(deltaTime);
}

void ControllerEx::on_fixed_update(MilliSeconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().stats().average_fps();
    stream << " best FPS:" << game().stats().best_fps();
    stream << " worst FPS:" << game().stats().worst_fps();
    stream << " input mode:" << static_cast<i32>(game().input().mode());

    game().window().title("TestGame " + stream.str());
}

void ControllerEx::on_key_down(const KeyboardEvent& ev)
{
    switch (ev.Code) {
    case Scancode::D1:
        game().input().controller_at(0).rumble(0, 0xFFFF, 1s);
        break;
    case Scancode::D2:
        game().input().controller_at(0).rumble(0xFFFF, 0, 1s);
        break;
    case Scancode::D3:
        game().input().controller_at(0).rumble(0xFFFF, 0xFFFF, 1s);
        break;
    case Scancode::BACKSPACE:
        game().pop_current_scene();
        break;
    default:
        break;
    }
}

void ControllerEx::on_mouse_motion(const MouseMotionEvent& ev)
{
}