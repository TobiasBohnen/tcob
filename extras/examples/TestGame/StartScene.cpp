// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include "./Scenes/AutomationEx.hpp"
#include "./Scenes/CanvasEx.hpp"
#include "./Scenes/ControllerEx.hpp"
#include "./Scenes/ImageEffectEx.hpp"
#include "./Scenes/Misc.hpp"
#include "./Scenes/PhysicsEx.hpp"
#include "./Scenes/SoundGeneratorEx.hpp"
#include "./Scenes/TextEx.hpp"
#include "./Scenes/TileMapEx.hpp"

#include <iomanip>
#include <ios>

static std::map<i32, SceneEntry> Scenes {};

start_scene::start_scene(game& g)
    : scene(g)
{
    Scenes[0] = {"Misc", [](game& g) { return std::make_shared<MiscScene>(g); }};
    Scenes[1] = {"AutomationEx", [](game& g) { return std::make_shared<AutomationEx>(g); }};
    Scenes[2] = {"CanvasEx", [](game& g) { return std::make_shared<CanvasEx>(g); }};
    Scenes[3] = {"ControllerEx", [](game& g) { return std::make_shared<ControllerEx>(g); }};
    Scenes[4] = {"ImageEffectEx", [](game& g) { return std::make_shared<ImageEffectEx>(g); }};
    Scenes[5] = {"PhysicsEx", [](game& g) { return std::make_shared<PhysicsEx>(g); }};
    Scenes[6] = {"SoundGeneratorEx", [](game& g) { return std::make_shared<SoundGeneratorEx>(g); }};
    Scenes[7] = {"TextEx", [](game& g) { return std::make_shared<TextEx>(g); }};
    Scenes[8] = {"TileMapEx", [](game& g) { return std::make_shared<TileMapEx>(g); }};
}

start_scene::~start_scene() = default;

void start_scene::on_start()
{
    auto& resMgr {get_game().get_library()};

    lua::script script;
    if (io::exists("bootstrap.lua")) {
        auto result {script.run_file<std::unordered_map<std::string, std::vector<std::string>>>("bootstrap.lua")};
        if (result.has_value()) {
            for (auto& [groupName, sources] : result.value()) {
                auto& group {resMgr.create_or_get_group(groupName)};
                for (auto& source : sources) {
                    group.mount(source);
                }
            }
        }
    }

    resMgr.load_all_groups();
    get_window().load_icon("res/tex/testing.webp");

    auto* resGrp {resMgr.get_group("res")};

    auto defaultCursor {resGrp->get<cursor>("default")};
    get_window().Cursor       = defaultCursor;
    defaultCursor->ActiveMode = "cursor1";

    _text         = std::make_shared<text>(resGrp->get<font>("raster-font1"));
    _text->Shader = resGrp->get<shader>("default2darray");

    std::string sceneText {};
    for (auto const& [k, v] : Scenes) {
        sceneText += std::to_string(k) + ": " + v.Name + "\n";
    }

    _text->Text   = sceneText;
    _text->Bounds = {{10, 10}, {750, 550}};
    _text->Style  = {.Color = colors::White, .Alignment = {horizontal_alignment::Left, vertical_alignment::Top}, .KerningEnabled = true};
}

void start_scene::on_draw_to(render_target& target)
{
    _text->draw_to(target);
}

void start_scene::on_update(milliseconds deltaTime)
{
    _text->update(deltaTime);
}

void start_scene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().get_stats()};
    stream << "avg FPS:" << stats.get_average_FPS();
    stream << " best FPS:" << stats.get_best_FPS();
    stream << " worst FPS:" << stats.get_worst_FPS();
    stream << " load complete:" << std::boolalpha << get_game().get_library().is_loading_complete();

    stream << "|" << get_game().get_library().get_loading_progress();

    get_window().Title = "TestGame " + stream.str();
}

void start_scene::on_key_down(keyboard::event& ev)
{
    i32 idx {-1};

    switch (ev.ScanCode) {
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        idx = static_cast<i32>(ev.KeyCode) - 48;
        break;
    }

    if (Scenes.contains(idx)) {
        get_game().push_scene(Scenes[idx].Func(get_game()));
    }

    locate_service<gfx::render_system>().get_stats().reset();
}

void start_scene::on_mouse_motion(mouse::motion_event& ev)
{
}
