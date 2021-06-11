#include "StartScene.hpp"
#include "Scenes/AutomationEx.hpp"
#include "Scenes/CanvasEx.hpp"
#include "Scenes/ControllerEx.hpp"
#include "Scenes/Misc.hpp"
#include "Scenes/TextEx.hpp"

#include <iomanip>

StartScene::StartScene(Game& game)
    : Scene(game)
{
}

StartScene::~StartScene()
{
}

void StartScene::on_start()
{
    auto& resMgr { game().resources() };
    resMgr.load_all_groups();
    game().window().load_icon("res/testing.webp");
    auto cursor = resMgr.get<Cursor>("res", "default");
    game().window().cursor(cursor);
    cursor->active_mode("cursor1");

    auto font { resMgr.get<Font>("res", "defaultFont") };
    _text.font(font);
    _text.text("1: Automation \n"
               "2: Canvas \n"
               "3: Text \n"
               "4: Controller \n"
               "z: Misc \n");
    _text.size({ 0.75f, 1.75f });
    _text.position({ 0.05f, 0.1f });
    _text.horizontal_alignment(TextAlignment::Left);
    _text.color(Colors::White);
    _text.outline_thickness(0.5f);
    _text.outline_color(Colors::Black);
    _text.update(MilliSeconds { 0 });
}

void StartScene::on_draw(RenderTarget& target)
{
    _text.draw(target);
}

void StartScene::on_update(MilliSeconds deltaTime)
{
}

void StartScene::on_fixed_update(MilliSeconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().stats().average_fps();
    stream << " best FPS:" << game().stats().best_fps();
    stream << " worst FPS:" << game().stats().worst_fps();

    game().window().title("TestGame " + stream.str());
}

void StartScene::on_key_down(const KeyboardEvent& ev)
{
    switch (ev.Code) {
    case Scancode::D1:
        game().push_scene<AutomationEx>();
        break;
    case Scancode::D2:
        game().push_scene<CanvasEx>();
        break;
    case Scancode::D3:
        game().push_scene<TextEx>();
        break;
    case Scancode::D4:
        game().push_scene<ControllerEx>();
        break;
    case Scancode::Z:
        game().push_scene<MiscScene>();
        break;
    case Scancode::BACKSPACE:
        game().pop_current_scene();
        break;
    default:
        break;
    }

    game().stats().reset();
}

void StartScene::on_mouse_motion(const MouseMotionEvent& ev)
{
}