// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CanvasEx.hpp"
#include "tcob/core/Color.hpp"
#include <iomanip>
#include <iostream>

using namespace std;

CanvasEx::CanvasEx(game& game)
    : scene {game}
    , _ninePatchTween(milliseconds {1500}, {0.0f, 1.0f, 1.0f, 0.0f})
{
    _material->Texture = _canvas.get_texture();
}

CanvasEx::~CanvasEx() = default;

void CanvasEx::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << locate_service<stats>().get_average_FPS();
    stream << " best FPS:" << locate_service<stats>().get_best_FPS();
    stream << " worst FPS:" << locate_service<stats>().get_worst_FPS();

    get_window().Title = "TestGame " + stream.str();
}

void CanvasEx::on_start()
{
    prepare_canvas();
    _ninePatchTween.start(playback_style::Looped);
}

void CanvasEx::on_update(milliseconds deltaTime)
{
    paint_to_canvas();
    _layer1.update(deltaTime);
    _ninePatchTween.update(deltaTime);
}

void CanvasEx::on_draw_to(render_target& target)
{
    _layer1.draw_to(target);
}

void CanvasEx::on_key_down(keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case scan_code::R: {
        auto _ = get_window().copy_to_image().save("screen01.webp");
    } break;
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    case scan_code::X: {
        auto sprite {_layer1.get_sprite_at(0)};
        sprite->Bounds = sprite->Bounds->with_size({400, 200});
    } break;
    default:
        break;
    }
}

void CanvasEx::on_mouse_motion(mouse::motion_event& ev)
{
}

void CanvasEx::prepare_canvas()
{
    auto* resGrp {locate_service<assets::library>().get_group("res")};

    Image        = resGrp->get<texture>("testing").get_obj();
    ImagePattern = _canvas.create_image_pattern({10, 550 - 256}, {128, 128}, 0, Image, 1);
    NP1          = resGrp->get<texture>("np1").get_obj();

    std::vector<color_stop> colorStops {{0, colors::Red}, {0.25, colors::Gold}, {0.75, colors::Green}, {1, colors::White}};
    color_gradient          colorGradient {colorStops};
    LinearGradient  = _canvas.create_linear_gradient({0, 0}, {0, 200}, colorGradient);
    BoxGradient     = _canvas.create_box_gradient({550, 80, 100, 100}, 8, 75, colorGradient);
    RadialGradient0 = _canvas.create_radial_gradient(rect_f {450, 0, 250, 250}.get_center(), 0, 125, colorGradient);
    RadialGradient1 = _canvas.create_radial_gradient(rect_f {550, 480, 100, 100}.get_center(), 0, 125, {0.5f, 1.f}, colorGradient);
    auto sprite1 {_layer1.create_sprite()};
    sprite1->Material = _material;
    sprite1->Bounds   = {{0.f, 0.f}, {1000.f, 800.f}};
}

void CanvasEx::paint_to_canvas()
{
    _canvas.begin_frame({1000, 800}, 1);

    _canvas.set_stroke_width(10);
    _canvas.set_stroke_style(colors::Blue);
    _canvas.path_2d("m 108,229 -6,-2.5 -5,3.5 0.4,-6.5 -5.0,-4 6.0,-1.5 2.2,-6 3.5,5.5 6.5,0.3 -4.0,5 z");
    _canvas.stroke();
    _canvas.set_stroke_style(colors::Red);
    _canvas.path_2d("m 200,100 a 99,99 0 0 0 0,150");
    _canvas.stroke();
    _canvas.set_stroke_style(colors::YellowGreen);
    _canvas.path_2d("m 200,100 a 99,99 0 0 1 0,150");
    _canvas.stroke();
    /*
       _canvas.set_fill_style(colors::Green);
       _canvas.begin_path();
       _canvas.star({400, 300}, 50, 50 / 2, 5);
       _canvas.fill();

       _canvas.set_fill_style(colors::Red);
       _canvas.begin_path();
       _canvas.rect({400 - 25, 300 - 25, 50, 50});
       _canvas.fill();

           _canvas.set_fill_style(colors::Gainsboro);
           _canvas.fill_rect({0, 0, 800, 600});

           _canvas.set_stroke_width(5);
           _canvas.set_stroke_style(colors::Green);
           _canvas.stroke_wavy_line({10, 300}, {300, 150}, 10, 0.2f);

           _canvas.set_stroke_style(colors::Red);
           _canvas.stroke_line({10, 300}, {300, 150});

           _canvas.set_stroke_width(1);
           _canvas.set_stroke_style(colors::Green);
           _canvas.stroke_wavy_line({300, 50}, {900, 150}, 10, 0.5f);

           _canvas.set_stroke_style(colors::Red);
           _canvas.stroke_line({300, 50}, {900, 150});

           _canvas.set_stroke_width(5);
           _canvas.set_stroke_style(colors::Green);
           _canvas.stroke_wavy_line({300, 10}, {10, 10}, 10, 0.2f);

           _canvas.set_stroke_style(colors::Red);
           _canvas.stroke_line({10, 10}, {300, 10});
       */
    /*
    rect_f uvlt {rect_f::FromLTRB(20 / 60.f, 20 / 60.f, 40 / 60.f, 40 / 60.f)};
    _canvas.draw_nine_patch(NP1, "default", {10, 290, 300, 200}, {50, 50}, {50, 50}, uvlt);
    _canvas.draw_nine_patch(NP1, "default", {330, 10, 200, 300}, {50, 50}, {50, 50}, uvlt);
        _canvas.draw_nine_patch(NP1, "default", {10, 290, 300, 200}, {60, 340, 200, 100}, uvlt);
    _canvas.draw_nine_patch(NP1, "default", {330, 10, 200, 300}, {380, 60, 100, 200}, uvlt);
        _canvas.draw_nine_patch(NP1, "default", {10, 10, 200, 200}, {60, 60, 100, 100}, uvlt);

    _canvas.draw_nine_patch(NP1, "default", {10, 10, 100 + _ninePatchTween.Value * 150, 200}, {50, 50}, {50, 50}, uvlt);
    _canvas.draw_nine_patch(NP1, "default", {10, 250, 150, 100 + _ninePatchTween.Value * 150}, {50, 50}, {50, 50}, uvlt);



_canvas.set_fill_style(colors::RebeccaPurple);
_canvas.fill_rect({400, 0, 400, 600});

_canvas.set_fill_style(colors::Green);
_canvas.triangle({10, 300}, {300, 20}, {610, 300});
_canvas.fill();

for (int i = 0; i < 6; i++) {
    u8 col{static_cast<u8>(i * 42)};
    _canvas.set_fill_style(color{col, col, col, 255});
    f32 siz{static_cast<f32>(300 - (i * 50))};
    _canvas.star({400, 300}, siz, siz / 2, 9 - i);
    _canvas.fill();
}


 for (int i = 0; i < 6; i++) {
     u8 col{static_cast<u8>(i * 42)};
     _canvas.set_fill_style(color{col, col, col, 255});
     f32 siz{static_cast<f32>(300 - (i * 50))};
     _canvas.regular_polygon({400, 300}, {siz, siz}, i + 3);
     _canvas.fill();
 }

_canvas.set_fill_style(colors::Black);
_canvas.star({200, 300}, 100, 75, 6);
_canvas.fill();

_canvas.set_fill_style(colors::Gray);
_canvas.star({200, 300}, 100, 50, 6);
_canvas.fill();

_canvas.set_fill_style(colors::White);
_canvas.star({200, 300}, 100, 25, 6);
_canvas.fill();

_canvas.set_fill_style(colors::Black);
_canvas.regular_polygon({200, 300}, {100, 100}, 6);
_canvas.fill();
_canvas.set_fill_style(colors::White);
_canvas.regular_polygon({200, 300}, {75, 75}, 6);
_canvas.fill();
_canvas.set_fill_style(colors::Yellow);
_canvas.regular_polygon({200, 300}, {50, 100}, 6);
_canvas.fill();

  _canvas.set_stroke_width(10);
  _canvas.set_line_cap(line_cap::Butt);

  _canvas.set_stroke_style(colors::Green);
  _canvas.stroke_quad_bezier({0.f, 0.f}, {400.f, 300.f}, {0.f, 600.f});
  _canvas.stroke_quad_bezier({800.f, 0.f}, {400.f, 300.f}, {800.f, 600.f});
  _canvas.stroke_cubic_bezier({400.0f, 0.f}, {100.0f, 300.0f}, {700.0f, 300.0f}, {400.0f, 600.0f});

  _canvas.set_stroke_style(colors::White);
  _canvas.stroke_dashed_quad_bezier({0.f, 0.f}, {400.f, 300.f}, {0.f, 600.f}, 25);
  _canvas.stroke_dashed_quad_bezier({800.f, 0.f}, {400.f, 300.f}, {800.f, 600.f}, 25);
  _canvas.stroke_dashed_cubic_bezier({400.0f, 0.f}, {100.0f, 300.0f}, {700.0f, 300.0f}, {400.0f, 600.0f}, 25);
  _canvas.stroke_dashed_line({400.f, 000.f}, {400.f, 600.f}, 10);
  _canvas.stroke_dashed_circle({400.f, 300.f}, 150, 36);

  _canvas.set_fill_style(colors::Black);
  _canvas.fill_dotted_quad_bezier({0.f, 0.f}, {400.f, 300.f}, {0.f, 600.f}, 5, 25);
  _canvas.fill_dotted_quad_bezier({800.f, 0.f}, {400.f, 300.f}, {800.f, 600.f}, 5, 25);
  _canvas.fill_dotted_cubic_bezier({400.0f, 0.f}, {100.0f, 300.0f}, {700.0f, 300.0f}, {400.0f, 600.0f}, 5, 25);
  _canvas.fill_dotted_line({400.f, 000.f}, {400.f, 600.f}, 5, 10);
  _canvas.fill_dotted_circle({400.f, 300.f}, 150, 5, 36);

  _canvas.set_stroke_width(5);
  _canvas.set_stroke_style(colors::Red);
  _canvas.stroke_dotted_quad_bezier({0.f, 0.f}, {400.f, 300.f}, {0.f, 600.f}, 5, 25);
  _canvas.stroke_dotted_quad_bezier({800.f, 0.f}, {400.f, 300.f}, {800.f, 600.f}, 5, 25);
  _canvas.stroke_dotted_cubic_bezier({400.0f, 0.f}, {100.0f, 300.0f}, {700.0f, 300.0f}, {400.0f, 600.0f}, 5, 25);
  _canvas.stroke_dotted_line({400.f, 000.f}, {400.f, 600.f}, 5, 10);
  _canvas.stroke_dotted_circle({400.f, 300.f}, 150, 5, 36);


_canvas.set_fill_style(colors::Green);
_canvas.fill_circle({400, 300}, 50);

_canvas.set_stroke_style(colors::GoldenRod);
_canvas.set_stroke_width(20);
_canvas.stroke_circle({400, 300}, 150);

_canvas.set_fill_style(colors::OliveDrab);
_canvas.fill_ellipse({250, 300}, 50, 15);

_canvas.set_stroke_style(colors::Orchid);
_canvas.stroke_rounded_rect({500, 300, 100, 50}, 15);

_canvas.set_fill_style(colors::DarkBlue);
_canvas.fill_rounded_rect_varying({375, 150, 50, 100}, 15, 30, 45, 60);

_canvas.set_stroke_style(colors::GreenYellow);
_canvas.set_line_cap(line_cap::Round);
std::vector<point_f> lines{{20, 20}, {780, 580}};
_canvas.stroke_lines(lines);

_canvas.set_fill_style(colors::Black);
_canvas.fill_arc({350, 300}, 50, 15, 270, winding::CW);

_canvas.set_scissor({300, 300, 50, 50});
_canvas.set_fill_style(colors::Tan);
_canvas.fill_rect({0, 0, 400, 600});
_canvas.reset_scissor();

_canvas.set_fill_style(colors::LightSeaGreen);
_canvas.fill_rect({600, 400, 25, 25});
_canvas.rotate_at(45, {612.5f, 412.5f});
_canvas.set_fill_style(colors::DarkSeaGreen);
_canvas.fill_rect({600, 400, 25, 50});
_canvas.reset_transform();

_canvas.set_fill_style(colors::LightBlue);
_canvas.fill_rect({190, 350, 100, 100});

_canvas.skew_at(45, 0, {240, 400});
_canvas.set_fill_style(colors::DarkBlue);
_canvas.fill_rect({190, 350, 100, 100});
_canvas.reset_transform();

_canvas.set_fill_style(colors::LightBlue);
_canvas.fill_rect({450, 20, 100, 80});

_canvas.set_font_face(Font);
_canvas.set_fill_style(colors::White);

_canvas.draw_textbox({{400, 0}, {400, 600}}, "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.");

_canvas.set_fill_style(colors::Teal);
_canvas.fill_arc({350, 300}, 50, 15, 270, winding::CCW);

_canvas.set_fill_style(LinearGradient);
_canvas.fill_rect({0, 0, 75, 200});

_canvas.set_global_alpha(0.5);
_canvas.set_fill_style(BoxGradient);
_canvas.fill_rect({525, 55, 150, 150});

_canvas.set_fill_style(RadialGradient1);
_canvas.fill_rect({550, 480, 100, 100});

_canvas.draw_image(Image, {10, 550 - 256, 128, 128});
_canvas.set_global_alpha(1.0);
//_canvas.fill_style(ImagePattern)
// _canvas.fill_rect({ 10, 550 - 256, 128, 128 })

_canvas.set_fill_style(colors::Green);
_canvas.fill_rect({80, 60, 100, 100});
*/

    _canvas.end_frame();
}
