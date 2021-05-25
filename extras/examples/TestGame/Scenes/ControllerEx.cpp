#include "ControllerEx.hpp"
#include "../StartScene.hpp"
#include <iomanip>
ControllerEx::ControllerEx(Game& game)
    : Scene(game)
{
}

ControllerEx::~ControllerEx()
{
}

void ControllerEx::on_start()
{
    auto& resMgr { game().resource_library() };
    auto font { resMgr.get<Font>("res", "DejaVuSans32") };

    _text.font(font);
    _text.text("1: High freq \n"
               "2: Low freq \n"
               "3: both \n");
    _text.bounds({ { 0.05f, 0.1f }, { 0.75f, 1.75f } });
    _text.outline_thickness(0.5f);
    _text.outline_color(Colors::Black);
    _text.update(0);

    _controllerDesc.font(font);
    _controllerDesc.outline_thickness(0.5f);
    _controllerDesc.outline_color(Colors::Black);
    _controllerDesc.bounds({ { 0.55f, 0.1f }, { 1.25f, 0.75f } });
    _controllerDesc.pivot({ 0.55f, 0.1f }, false);
    std::stringstream stream;
    stream << "Controller count: " << game().input().contoller_count() << "\n";
    stream << "Name of controller 0:" << game().input().controller_at(0).name();
    _controllerDesc.text(stream.str());
    _controllerDesc.update(0);
    _controllerDesc.scale({ 0.75f, 0.75f });
}

void ControllerEx::draw(RenderTarget& target)
{
    _text.draw(target);
    _controllerDesc.draw(target);
}

void ControllerEx::update(f64 deltaTime)
{
}

void ControllerEx::fixed_update(f64 deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().fps().average();
    stream << " best FPS:" << game().fps().best();
    stream << " worst FPS:" << game().fps().worst();

    game().window().title("TestGame " + stream.str());
}

void ControllerEx::on_key_down(const KeyboardEvent& ev)
{
    switch (ev.Code) {
    case Scancode::D1:
        game().input().controller_at(0).rumble(0, 0xFFFF, 1000);
        break;
    case Scancode::D2:
        game().input().controller_at(0).rumble(0xFFFF, 0, 1000);
        break;
    case Scancode::D3:
        game().input().controller_at(0).rumble(0xFFFF, 0xFFFF, 1000);
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