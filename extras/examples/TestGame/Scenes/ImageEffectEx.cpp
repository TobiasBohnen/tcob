// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageEffectEx.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/ImageFilters.hpp"
#include "tcob/gfx/Texture.hpp"
#include <iomanip>
#include <memory>

ImageEffectEx::ImageEffectEx(game& game)
    : scene(game)
{
    _mat0->Texture = _tex0;
    image img0 {*image::Load("res/tex/tee1.png")};
    //  image img0 {grayscale_filter {}(img)};
    auto  imgSize {img0.get_info().Size};
    _tex0->create(imgSize, 7, texture::format::RGBA8);

    _tex0->update_data(img0.get_data(), 0);
    _tex0->add_region("normal", {{0, 0, 1, 1}, 0});

    {
        edge_detect_filter filter;
        filter.IncludeAlpha = true;
        image filterImg {filter(img0)};
        _tex0->update_data(filterImg.get_data(), 1);
        _tex0->add_region("edgedetect", {{0, 0, 1, 1}, 1});
    }

    {
        blur_filter filter;
        image       filterImg {filter(img0)};
        _tex0->update_data(filterImg.get_data(), 2);
        _tex0->add_region("blur", {{0, 0, 1, 1}, 2});
    }

    {
        emboss_filter filter;
        image         filterImg {filter(img0)};
        _tex0->update_data(filterImg.get_data(), 3);
        _tex0->add_region("emboss", {{0, 0, 1, 1}, 3});
    }

    {
        edge_enhance_filter filter;
        filter.IncludeAlpha = true;
        image filterImg {filter(img0)};
        _tex0->update_data(filterImg.get_data(), 4);
        _tex0->add_region("edgeenhance", {{0, 0, 1, 1}, 4});
    }

    {
        motion_blur_filter filter;
        image              filterImg {filter(img0)};
        _tex0->update_data(filterImg.get_data(), 5);
        _tex0->add_region("motionblur", {{0, 0, 1, 1}, 5});
    }

    {
        sharpen_filter filter;
        image          filterImg {filter(img0)};
        _tex0->update_data(filterImg.get_data(), 6);
        _tex0->add_region("sharpen", {{0, 0, 1, 1}, 6});
    }
}

ImageEffectEx::~ImageEffectEx() = default;

void ImageEffectEx::on_start()
{
    auto sprite0           = _layer1.create_sprite();
    sprite0->Material      = _mat0;
    sprite0->TextureRegion = "normal";
    sprite0->Bounds        = {{0, 0}, {128, 192}};
    sprite0->Center        = rect_f {0, 0, 800, 600}.get_center();

    //////////
    auto sprite1           = _layer1.create_sprite();
    sprite1->Material      = _mat0;
    sprite1->TextureRegion = "blur";
    sprite1->Bounds        = {{0.f, 0.f}, {128, 192}};

    //////////
    auto sprite2           = _layer1.create_sprite();
    sprite2->Material      = _mat0;
    sprite2->TextureRegion = "edgedetect";
    sprite2->Bounds        = {{0.f, 200.f}, {128, 192}};

    //////////
    auto sprite3           = _layer1.create_sprite();
    sprite3->Material      = _mat0;
    sprite3->TextureRegion = "emboss";
    sprite3->Bounds        = {{0.f, 400.f}, {128, 192}};

    //////////
    auto sprite4           = _layer1.create_sprite();
    sprite4->Material      = _mat0;
    sprite4->TextureRegion = "edgeenhance";
    sprite4->Bounds        = {{600.f, 0.f}, {128, 192}};

    //////////
    auto sprite5           = _layer1.create_sprite();
    sprite5->Material      = _mat0;
    sprite5->TextureRegion = "motionblur";
    sprite5->Bounds        = {{600.f, 200.f}, {128, 192}};

    //////////
    auto sprite6           = _layer1.create_sprite();
    sprite6->Material      = _mat0;
    sprite6->TextureRegion = "sharpen";
    sprite6->Bounds        = {{600.f, 400.f}, {128, 192}};
}

void ImageEffectEx::on_draw_to(render_target& target)
{
    _layer1.draw_to(target);
}

void ImageEffectEx::on_update(milliseconds deltaTime)
{
    _layer1.update(deltaTime);
}

void ImageEffectEx::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << locate_service<stats>().get_average_FPS();
    stream << " best FPS:" << locate_service<stats>().get_best_FPS();
    stream << " worst FPS:" << locate_service<stats>().get_worst_FPS();

    get_window().Title = "TestGame " + stream.str();
}

void ImageEffectEx::on_key_down(keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
    }
}

void ImageEffectEx::on_mouse_motion(mouse::motion_event& ev)
{
}