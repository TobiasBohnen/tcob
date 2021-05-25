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
    auto& resMgr { game().resource_library() };
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
    _text.update(0);
}

void StartScene::draw(RenderTarget& target)
{
    _text.draw(target);
}

void StartScene::update(f64 deltaTime)
{
}

void StartScene::fixed_update(f64 deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().fps().average();
    stream << " best FPS:" << game().fps().best();
    stream << " worst FPS:" << game().fps().worst();

    game().window().title("TestGame " + stream.str());
}

void StartScene::on_key_down(const KeyboardEvent& ev)
{
    switch (ev.Code) {
    case Scancode::D1:
        game().push_scene(std::make_shared<AutomationEx>(game()));
        break;
    case Scancode::D2:
        game().push_scene(std::make_shared<CanvasEx>(game()));
        break;
    case Scancode::D3:
        game().push_scene(std::make_shared<TextEx>(game()));
        break;
    case Scancode::D4:
        game().push_scene(std::make_shared<ControllerEx>(game()));
        break;
    case Scancode::Z:
        game().push_scene(std::make_shared<MiscScene>(game()));
        break;
    case Scancode::BACKSPACE:
        game().pop_current_scene();
        break;
    default:
        break;
    }

    game().fps().reset();
}

void StartScene::on_mouse_motion(const MouseMotionEvent& ev)
{
}