#include "Misc.hpp"
#include "../StartScene.hpp"

#include <iomanip>

using namespace std;

MiscScene::MiscScene(Game& game)
    : Scene(game)
{
}

MiscScene::~MiscScene()
{
}

f32 pointSize = 5;
i32 numPoints = 100;

void MiscScene::on_start()
{
    auto& resMgr = game().resource_library();

    Random rand;
    /*
    for (int i = 0; i < 1000; i++) {
        float x = rand(10.f);
        float y = rand(10.f);

        if (i >= 400) {
            auto& sprite0 = layer1.create_sprite();
            sprite0.material(resMgr.get<Material>("res", "spriteMat"));
            sprite0.size({ 0.25f, 0.25f });
            sprite0.position({ x, y });
        } else {
            auto& sprite1 = layer1.create_sprite();
            sprite1.material(resMgr.get<Material>("res", "spriteMat1"));
            sprite1.size({ 0.25f, 0.25f });
            sprite1.position({ x, y });
        }
    }
*/
    tileMap.material(resMgr.get<Material>("res", "arrayMat"));
    tileMap.tile_size({ 0.2f, 0.2f });

    int idx = 0;
    tileMap.tile_set({

        { 1, "ice1" },
        { 2, "etched0" },
        { 3, "frozen4" },
        { 4, "sandstone_floor5" },
        { 5, "snake-c0" },
        { 6, "tomb1" },
        { 7, "white_marble3" },
        { 8, "white_marble8" },
        { 9, "pebble_brown3" },
        { 10, "pebble_brown1" },
        { 11, "pebble_brown2" },
        { 12, "volcanic_floor2" },
        { 13, "ice2" },
        { 14, "white_marble6" },
        { 15, "mosaic4" },
        { 16, "infernal06" },
        { 17, "infernal05" },
        { 18, "infernal04" },
        { 19, "infernal03" },
        { 20, "floor_vines0" }

    });

    std::array<u16, 200 * 200> tiles1;
    for (int i = 0; i < 200 * 200; i++) {
        tiles1[i] = rand(0, 20);
    }
    tileMap.add_layer<200, 200>(tiles1);

    std::array<u16, 10> tiles2;
    for (int i = 0; i < 10; i++) {
        tiles2[i] = 1;
    }
    tileMap.add_layer<2, 5>(tiles2);

    std::array<u16, 10> tiles3;
    for (int i = 0; i < 10; i++) {
        tiles3[i] = 2;
    }
    tileMap.add_layer<2, 5>(tiles3, { 4, 0 });

    auto shader = resMgr.get<ShaderProgram>("res", "default2d");

    rtt.create({ 400, 400 });
    rtt.camera().look_at({ 0.5f, 0.5f });
    rtt.clear(Colors::Black);
    auto rttMat = rtt.material();
    rttMat->Shader = shader;

    auto& sprite1 = layer1.create_sprite();
    sprite1.material(rttMat);
    sprite1.size({ 1.f, 1.f });
    sprite1.position({ 1, 1 });
    sprite1.color(Colors::Red);
    sprite1.transparency(0.125f);
    rttID = sprite1.id();

    partSystem1 = *resMgr.get<ParticleSystem>("res", "system1");

    partSystem1.add_affector([](Particle& p) {
        if (p.Stage == 0) {
            if (p.life_ratio() <= 0.95f) {
                auto pp { p.direction() };
                p.direction(pp - 180);
                p.Stage = 1;
                p.color(Colors::Red);
            }
        }
        if (p.Stage == 1) {
            if (p.life_ratio() <= 0.75f) {
                auto pp { p.direction() };
                p.direction(pp - 45);
                p.Stage = 2;
                p.color(Colors::Yellow);
            }
        }
        if (p.Stage == 2) {
            if (p.life_ratio() <= 0.50f) {
                auto pp { p.direction() };
                p.direction(pp + 90);
                p.Stage = 3;
                p.color(Colors::Blue);
            }
        }
    });
    partSystem1.add_affector([](Particle& p) {
        p.transparency(1 - p.life_ratio());
    });
    partSystem1.position({ 0.1f, 0.1f });

    font = resMgr.get<Font>("res", "defaultFont");
    text.font(font);
    text.text("{Alpha:1.0}Lorem ipsum dolor sit amet\n, {Color:Red}consetetur sadipscing elitr, sed diam{Alpha:0.5} nonumy eirmod "
              "{Color:Blue}tempor invidunt ut labore et dolore {Color:Gray}magna aliquyam erat, {Alpha:1.0}sed diam voluptua. "
              "At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren,  {Alpha:0.5}no sea takimata "
              "sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr,  {Alpha:1}sed diam nonumy "
              "eirmod tempor invidunt ut labore et dolore magna aliquyam erat, {Color:Yellow}sed diam voluptua. At vero eos et accusam "
              "et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.");
    //text.text("ö±\nad to");
    text.size({ 0.75f, 1.75f });
    text.position({ 0.25f, 0.1f });
    text.horizontal_alignment(TextAlignment::Left);
    text.color(Colors::White);
    text.outline_thickness(0.5f);
    text.outline_color(Colors::Black);

    auto& sprite0 = layer1.create_sprite();
    sprite0.material(resMgr.get<Material>("res", "spriteMat1"), "default");
    sprite0.size({ 0.75f, 0.75f });
    sprite0.position({ 0.25f, 0.1f });
    //sprite0.transparency(0.5f);
    scrollID = sprite0.id();

    for (f32 y = 0; y < pointSize * numPoints; y += pointSize) {
        for (f32 x = 0; x < pointSize * numPoints; x += pointSize) {
            Vertex v {
                .Position = { (x + pointSize / 2) / 800.f, (y + pointSize / 2) / 800.f },
                .Color = { 255, 255, 255, 255 },
                .TexCoords = { x / (pointSize * numPoints), y / (pointSize * numPoints), 0 }
            };
            pointcloud.add(v);
        }
    }

    pointcloud.material(resMgr.get<Material>("res", "pointMat2"));
    pointcloud.material()->Shader->set_uniform("numPoints", numPoints);
    pointcloud.point_size(pointSize * 0.75f);

    ninepatch.material(resMgr.get<Material>("res", "spriteMat"));
    ninepatch.size({ 0.5f, 0.5f });
    ninepatch.position({ 0.25f, 0.25f });
    ninepatch.define_center({ 0.15f, 0.15f }, { 0.15f, 0.15f }, { 0.15f, 0.15f }, { 0.15f, 0.15f });

    webp = *resMgr.get<WebpAnimation>("res", "test");
    webp.size({ 0.5f, 0.5f });
    webp.position({ 0.75f, 0.25f });

    sound.open("res/audio/door.wav");
}

void MiscScene::draw(RenderTarget& target)
{
    tileMap.draw(target);
    layer1.draw(target);
    partSystem1.draw(target);
    ninepatch.draw(target);
    text.draw(target);
    webp.draw(target);
}

void MiscScene::update(f64 deltaTime)
{
    text.update(deltaTime);
    tileMap.update(deltaTime);
    layer1.update(deltaTime);
    _rvc.update(deltaTime);
    partSystem1.update(deltaTime);
    pointcloud.update(deltaTime);
    ninepatch.update(deltaTime);
    webp.update(deltaTime);
}

void MiscScene::fixed_update(f64 deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().fps().average();
    stream << " best FPS:" << game().fps().best();
    stream << " worst FPS:" << game().fps().worst();
    //stream << "|" << sound.playback_position();

    game().window().title("TestGame " + stream.str());
}

auto get_point_direction(i32 x, i32 y) -> PointF
{
    f32 dirInDegrees;
    f32 halfnum { numPoints / 2.f };

    if (x < halfnum) {
        if (y < halfnum) {
            dirInDegrees = 315;
        } else {
            dirInDegrees = 225;
        }
    } else {
        if (y < halfnum) {
            dirInDegrees = 45;
        } else {
            dirInDegrees = 135;
        }
    }
    const f64 radians { (dirInDegrees - 90) / 360 * TAU };
    f32 modx { std::abs(x - halfnum) / halfnum };
    f32 mody { std::abs(y - halfnum) / halfnum };

    return {
        0.005f * static_cast<f32>(std::cos(radians)) * modx,
        0.005f * static_cast<f32>(std::sin(radians)) * mody
    };
}

void MiscScene::on_key_down(const KeyboardEvent& ev)
{
    float moveFactor = 0.05f;
    auto& window = game().window();
    auto& camera = window.camera();
    auto cursor = window.cursor();

    if (ev.Code == Scancode::D1) {
        partSystem1.restart();
    } else if (ev.Code == Scancode::D2) {
        game().fps().reset();
    } else if (ev.Code == Scancode::D3) {
        auto& resMgr = game().resource_library();
        auto map { resMgr.resource_state("res") };
        std::cout << "created: " << map[ResourceState::Created] << std::endl;
        std::cout << "loaded: " << map[ResourceState::Loaded] << std::endl;
    } else if (ev.Code == Scancode::D4) {
        Random r;
        std::map<f64, i32> hist;
        for (int n = 0; n < 20000; ++n) {
            ++hist[std::round(r(-1, 6) * 10) / 10];
        }
        for (auto p : hist) {
            std::cout << p.first << ' ' << std::string(p.second / 100, '*') << '\n';
        }
    } else if (ev.Code == Scancode::D5) {
        webp.start(true);
    } else if (ev.Code == Scancode::D6) {
        webp.restart();
    } else if (ev.Code == Scancode::D7) {
        webp.toggle_pause();
    } else if (ev.Code == Scancode::RIGHT) {
        ninepatch.size({ ninepatch.size().Width + 0.02f, ninepatch.size().Height });
    } else if (ev.Code == Scancode::LEFT) {
        ninepatch.size({ ninepatch.size().Width - 0.02f, ninepatch.size().Height });
    } else if (ev.Code == Scancode::UP) {
        ninepatch.size({ ninepatch.size().Width, ninepatch.size().Height + 0.02f });
    } else if (ev.Code == Scancode::DOWN) {
        ninepatch.size({ ninepatch.size().Width, ninepatch.size().Height - 0.02f });
    } else if (ev.Code == Scancode::A) {
        camera.move_by({ -moveFactor, 0 });
    } else if (ev.Code == Scancode::D) {
        camera.move_by({ moveFactor, 0 });
    } else if (ev.Code == Scancode::S) {
        camera.move_by({ 0.0f, moveFactor });
    } else if (ev.Code == Scancode::W) {
        camera.move_by({ 0.0f, -moveFactor });
    } else if (ev.Code == Scancode::Q) {
        camera.zoom_by({ 1.25f, 1.25f });
        pointcloud.point_size(std::ceil(camera.zoom().Width * pointSize));
        std::cout << pointcloud.point_size() << std::endl;
    } else if (ev.Code == Scancode::E) {
        camera.zoom_by({ 0.8f, 0.8f });

        pointcloud.point_size(std::ceil(camera.zoom().Height * pointSize));
        std::cout << pointcloud.point_size() << std::endl;
    } else if (ev.Code == Scancode::F) {
        for (i32 y = 0; y < numPoints; y++) {
            for (i32 x = 0; x < numPoints; x++) {
                i32 idx { x + y * numPoints };
                Vertex v { pointcloud.get(idx) };

                PointF direction { get_point_direction(x, y) };
                v.Position[0] += direction.X;
                v.Position[1] += direction.Y;

                pointcloud.set(idx, v);
            }
        }
    } else if (ev.Code == Scancode::G) {
        for (i32 y = 0; y < numPoints; y++) {
            for (i32 x = 0; x < numPoints; x++) {
                i32 idx { x + y * numPoints };
                Vertex v { pointcloud.get(idx) };

                PointF direction { get_point_direction(x, y) };
                v.Position[0] -= direction.X;
                v.Position[1] -= direction.Y;

                pointcloud.set(idx, v);
            }
        }
    } else if (ev.Code == Scancode::O) {
        sound.play();
    } else if (ev.Code == Scancode::P) {
        sound.stop();
    } else if (ev.Code == Scancode::T) {
        for (u32 i = 0; i < 5; i++) {
            tileMap.modify_layer(1, { 0, i }, 2);
        }
    } else if (ev.Code == Scancode::Z) {
        for (int i = 1; i <= 30; i++) {
            tileMap.modify_tile_set(i, "pebble_brown3");
        }
    } else if (ev.Code == Scancode::M) {
        game().window().create_screenshot().save_async("screen1async.webp");
    } else if (ev.Code == Scancode::BACKSPACE) {
        game().pop_current_scene();
    }
}

void MiscScene::on_mouse_motion(const MouseMotionEvent& ev)
{
}

void MiscScene::on_controller_axis_motion(const ControllerAxisEvent& ev)
{
    if (ev.Axis == GameControllerAxis::LeftX)
        std::cout << static_cast<i32>(ev.Axis) << ":" << ev.Value << std::endl;
}