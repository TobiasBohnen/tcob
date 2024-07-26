// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TextEx.hpp"
#include <iomanip>

using namespace std::chrono_literals;

TextEx::TextEx(game& game)
    : scene(game)
{
}

TextEx::~TextEx() = default;

void TextEx::on_start()
{
    auto* resGrp {locate_service<assets::library>().get_group("res")};
    auto  rfont {resGrp->get<font>("raster-font1")};
    auto  tfont {resGrp->get<font>("DejaVuSans32")};

    auto& text0 {_texts.emplace_back(std::make_unique<text>(rfont))};
    text0->Text   = "normal text";
    text0->Bounds = {{30, 6}, {240, 45}};

    auto& text2 {_texts.emplace_back(std::make_unique<text>(rfont))};
    text2->Text   = "{COLOR:Red}c{COLOR:Blue}o{COLOR:Yellow}l{COLOR:LightBlue}o{COLOR:Cyan}r{COLOR:Orange}e{COLOR:Blue}d {COLOR:RebeccaPurple}text";
    text2->Bounds = {{30, 126}, {300, 300}};

    auto& text3 {_texts.emplace_back(std::make_unique<text>(rfont))};
    text3->Text   = "{ALPHA:1}t{ALPHA:0.9}r{ALPHA:0.8}a{ALPHA:0.7}n{ALPHA:0.6}s{ALPHA:0.5}p{ALPHA:0.4}a{ALPHA:0.3}r{ALPHA:0.2}e{ALPHA:0.1}n{ALPHA:1}t text";
    text3->Bounds = {{30, 186}, {330, 300}};

    auto& text4 {_texts.emplace_back(std::make_unique<text>(tfont))};
    text4->Text   = "{EFFECT:1} Typing\n"
                    "{EFFECT:2} FadeIn\n"
                    "{EFFECT:3} FadeOut\n"
                    "{EFFECT:4} Blink\n"
                    "{EFFECT:5} Shake\n"
                    "{EFFECT:6} TypingShake\n"
                    "{EFFECT:7} {COLOR:blue}Wave{COLOR:lightblue}Wave\n";
    text4->Bounds = {{420, 6}, {330, 1500}};
    text4->Shader = resGrp->get<shader>("defaultFontShader");

    auto& effects {text4->get_effects()};
    effects.add(1, make_unique_quad_tween<typing_effect>(3s, {}));
    effects.add(2, make_unique_quad_tween<fade_in_effect>(3s, {}));
    effects.add(3, make_unique_quad_tween<fade_out_effect>(3s, {}));
    effects.add(4, make_unique_quad_tween<blink_effect>(3s, {colors::Orange, colors::Teal, 5.f}));

    auto eff5 {make_shared_quad_tween<shake_effect>(3s, {3, 1.f, rng {12345}})};
    eff5->Interval = 25ms;
    effects.add(5, eff5);

    auto eff6 {make_shared_quad_tween<typing_effect, shake_effect>(3s, {}, {3, 1.f, rng {12345}})};
    eff6->Interval = 100ms;
    effects.add(6, eff6);

    effects.add(7, make_unique_quad_tween<wave_effect>(3s, {30, 4.f}));

    effects.start_all(playback_style::Looped);
}

void TextEx::on_draw_to(render_target& target)
{
    target.clear(color::FromRGBA(0x2a2a2aff));

    for (auto& text : _texts) {
        text->draw_to(target);
    }
}

void TextEx::on_update(milliseconds deltaTime)
{
    for (auto& text : _texts) {
        text->update(deltaTime);
    }
}

void TextEx::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().get_stats()};
    stream << "avg FPS:" << stats.get_average_FPS();
    stream << " best FPS:" << stats.get_best_FPS();
    stream << " worst FPS:" << stats.get_worst_FPS();

    get_window().Title = "TestGame " + stream.str();
}

void TextEx::on_key_down(keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
    }
}

void TextEx::on_mouse_motion(mouse::motion_event& ev)
{
}