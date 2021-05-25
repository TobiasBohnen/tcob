#include "TextEx.hpp"
#include "../StartScene.hpp"
#include <iomanip>
TextEx::TextEx(Game& game)
    : Scene(game)
{
}

TextEx::~TextEx()
{
}

void TextEx::on_start()
{
    auto& resMgr { game().resource_library() };

    _text0.text("normal text");
    _text0.bounds({ { 0.05f, 0.01f }, { 0.5f, 0.5f } });

    _text1.text("outlined text");
    _text1.bounds({ { 0.05f, 0.11f }, { 0.5f, 0.5f } });
    _text1.outline_thickness(1.f);
    _text1.outline_color(Colors::Black);

    _text2.text("{COLOR:Red}c{COLOR:Blue}o{COLOR:Yellow}l{COLOR:LightBlue}o{COLOR:Cyan}r{COLOR:Orange}e{COLOR:Blue}d {COLOR:RebeccaPurple}text");
    _text2.bounds({ { 0.05f, 0.21f }, { 0.5f, 0.5f } });

    _text3.text("{ALPHA:1}t{ALPHA:0.9}r{ALPHA:0.8}a{ALPHA:0.7}n{ALPHA:0.6}s{ALPHA:0.5}p{ALPHA:0.4}a{ALPHA:0.3}r{ALPHA:0.2}e{ALPHA:0.1}n{ALPHA:1}t text");
    _text3.bounds({ { 0.05f, 0.31f }, { 0.55f, 0.5f } });
}

void TextEx::draw(RenderTarget& target)
{
    target.clear(Colors::Gray);

    _text0.draw(target);
    _text1.draw(target);
    _text2.draw(target);
    _text3.draw(target);
}

void TextEx::update(f64 deltaTime)
{
    _text0.update(deltaTime);
    _text1.update(deltaTime);
    _text2.update(deltaTime);
    _text3.update(deltaTime);
}

void TextEx::fixed_update(f64 deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().fps().average();
    stream << " best FPS:" << game().fps().best();
    stream << " worst FPS:" << game().fps().worst();

    game().window().title("TestGame " + stream.str());
}

void TextEx::on_key_down(const KeyboardEvent& ev)
{
    switch (ev.Code) {
    case Scancode::BACKSPACE:
        game().pop_current_scene();
        break;
    default:
        break;
    }
}

void TextEx::on_mouse_motion(const MouseMotionEvent& ev)
{
}