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
    auto& resMgr { game().resources() };

    auto& text0 { _texts.emplace_back(std::make_unique<Text>()) };
    text0->text("normal text");
    text0->bounds({ { 0.05f, 0.01f }, { 0.5f, 0.5f } });

    auto& text1 { _texts.emplace_back(std::make_unique<Text>()) };
    text1->text("outlined text");
    text1->bounds({ { 0.05f, 0.11f }, { 0.5f, 0.5f } });
    text1->outline_thickness(1.f);
    text1->outline_color(Colors::Black);

    auto& text2 { _texts.emplace_back(std::make_unique<Text>()) };
    text2->text("{COLOR:Red}c{COLOR:Blue}o{COLOR:Yellow}l{COLOR:LightBlue}o{COLOR:Cyan}r{COLOR:Orange}e{COLOR:Blue}d {COLOR:RebeccaPurple}text");
    text2->bounds({ { 0.05f, 0.21f }, { 0.5f, 0.5f } });

    auto& text3 { _texts.emplace_back(std::make_unique<Text>()) };
    text3->text("{ALPHA:1}t{ALPHA:0.9}r{ALPHA:0.8}a{ALPHA:0.7}n{ALPHA:0.6}s{ALPHA:0.5}p{ALPHA:0.4}a{ALPHA:0.3}r{ALPHA:0.2}e{ALPHA:0.1}n{ALPHA:1}t text");
    text3->bounds({ { 0.05f, 0.31f }, { 0.55f, 0.5f } });

    auto& text4 { _texts.emplace_back(std::make_unique<Text>()) };
    text4->text("effect 1 \n"
                "{EFFECT:1}"
                "blablabla"
                "{EFFECT:0}");
    text4->bounds({ { 0.75f, 0.01f }, { 0.55f, 0.5f } });
}

void TextEx::on_draw(RenderTarget& target)
{
    target.clear(Colors::Gray);

    for (auto& text : _texts) {
        text->draw(target);
    }
}

void TextEx::on_update(MilliSeconds deltaTime)
{
    for (auto& text : _texts) {
        text->update(deltaTime);
    }
}

void TextEx::on_fixed_update(MilliSeconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().stats().average_fps();
    stream << " best FPS:" << game().stats().best_fps();
    stream << " worst FPS:" << game().stats().worst_fps();

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