#include "CanvasEx.hpp"
#include "../StartScene.hpp"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <locale>

using namespace std;

CanvasEx::CanvasEx(Game& game)
    : Scene { game }
    , _api { game, "tcob" }
{
    _layer1 = std::make_shared<SpriteBatch>();
}

CanvasEx::~CanvasEx()
{
}

void CanvasEx::fixed_update(f64 deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().stats().average_fps();
    stream << " best FPS:" << game().stats().best_fps();
    stream << " worst FPS:" << game().stats().worst_fps();

    game().window().title("TestGame " + stream.str());
}

void CanvasEx::on_start()
{
    auto& resMgr { game().resources() };

    auto shader { resMgr.get<ShaderProgram>("res", "default2d") };
    _rtt.create({ 800, 600 });
    auto rttMat = _rtt.material();
    rttMat->Shader = shader;

    auto& sprite1 { _layer1->create_sprite() };
    sprite1.material(rttMat);
    sprite1.size({ 800.f / 600.f, 1.f });
    sprite1.position({ 0, 0 });

    // prepare
    _script = _api.create_script();

    prepare_canvas();
}

void CanvasEx::update(double deltaTime)
{
    paint_to_canvas();
    _layer1->update(deltaTime);
}

void CanvasEx::draw(RenderTarget& target)
{
    _layer1->draw(target);
}

void CanvasEx::on_key_down(const KeyboardEvent& ev)
{
    switch (ev.Code) {
    case Scancode::R:
        game().window().create_screenshot().save("screen01.webp");
        break;
    case Scancode::BACKSPACE:
        game().pop_current_scene();
        break;
    default:
        break;
    }
}

void CanvasEx::on_mouse_motion(const MouseMotionEvent& ev)
{
}

void CanvasEx::prepare_canvas()
{
    auto _ {
        _script->run_script(
            "local c = tcob.Canvas "
            "local Colors = tcob.Colors "
            "Font = c:add_font('res', 'defaultFont') "
            "Image = c:add_image('res/testing.webp') "
            "ImagePattern = c:create_image_pattern({ 10, 550 - 256 }, { 128, 128 }, 0, image, 1) "

            "local colors = {{ 0, Colors.Red }, { 0.25, Colors.Gold }, { 0.75, Colors.Green }, { 1, Colors.White } } "
            "LinearGradient = c:create_linear_gradient({ 0, 0 }, { 0, 200 }, colors) "
            "BoxGradient = c:create_box_gradient({ 550, 80, 100, 100 }, 8, 75, colors) "
            "RadialGradient = c:create_radial_gradient({ 600, 530 }, 5, 75, colors) ")
    };
}

void CanvasEx::paint_to_canvas()
{
    _rtt.clear({ 0, 0, 0, 0 });
    _rtt.setup_render();

    auto _ {
        _script->run_script(
            "local c = tcob.Canvas "
            "local Colors = tcob.Colors "
            "c:begin_frame({ 800, 600 }, 1) "
            "c:fill_style(Colors.Gainsboro) "
            "c:fill_rect({ 0, 0, 400, 600 }) "
            "c:fill_style(Colors.RebeccaPurple) "
            "c:fill_rect({ 400, 0, 400, 600 }) "

            "c:fill_style(Colors.Green) "
            "c:fill_circle({ 400, 300 }, 50) "

            "c:stroke_style(Colors.GoldenRod) "
            "c:stroke_width(20) "
            "c:stroke_circle({ 400, 300 }, 150) "

            "c:fill_style(Colors.OliveDrab) "
            "c:fill_ellipse({ 250, 300 }, 50, 15) "

            "c:stroke_style(Colors.Orchid) "
            "c:stroke_rounded_rect({ 500, 300, 100, 50 }, 15) "

            "c:fill_style(Colors.DarkBlue) "
            "c:fill_rounded_rect_varying({ 375, 150, 50, 100 }, 15, 30, 45, 60) "

            "c:stroke_style(Colors.White) "
            "c:line_cap('Round') "
            "c:stroke_lines({ { 20, 20 }, { 780, 580 } }) "

            "c:fill_style(Colors.Black) "
            "c:fill_arc({ 350, 300 }, 50, 15, 270, 'CW') "

            "c:scissor({ 300, 300, 50, 50 }) "
            "c:fill_style(Colors.Tan) "
            "c:fill_rect({ 0, 0, 400, 600 }) "
            "c:reset_scissor() "

            "c:fill_style(Colors.LightSeaGreen) "
            "c:fill_rect({ 600, 400, 25, 25 }) "
            "c:rotate(45, { 612.5, 412.5 }) "
            "c:fill_style(Colors.DarkSeaGreen) "
            "c:fill_rect({ 600, 400, 25, 50 }) "
            "c:reset_transform() "

            "c:fill_style(Colors.LightBlue) "
            "c:fill_rect({ 190, 350, 100, 100 }) "

            "c:skew_x(45, { 240, 400 }) "
            "c:fill_style(Colors.DarkBlue) "
            "c:fill_rect({ 190, 350, 100, 100 }) "
            "c:reset_transform() "

            "c:fill_style(Colors.LightBlue) "
            "c:fill_rect({ 450, 20, 100, 100 }) "

            "c:font_face(Font) "
            "c:text_outline_color(Colors.Black) "
            "c:text_outline_thickness(0.5) "
            "c:fill_style(Colors.White) "

            "c:draw_textbox({ 250, 20 }, { 100, 100 }, 'okaydokey') "

            "c:fill_style(Colors.Teal) "
            "c:fill_arc({ 350, 300 }, 50, 15, 270, 'CCW') "

            "c:fill_style(LinearGradient) "
            "c:fill_rect({ 0, 0, 75, 200 }) "

            "c:global_alpha(0.5) "
            "c:fill_style(BoxGradient) "
            "c:fill_rect({ 525, 55, 150, 150 }) "
            "c:global_alpha(1.0) "

            "c:fill_style(RadialGradient) "
            "c:fill_rect({ 550, 480, 100, 100 }) "

            "c:fill_style(ImagePattern) "
            "c:fill_rect({ 10, 550 - 256, 128, 128 }) "

            "c:fill_style(Colors.Green) "
            "c:fill_rect({ 80, 60, 100, 100 }) "
            "c:end_frame() "

            )
    };
    _rtt.finish_render();
}
