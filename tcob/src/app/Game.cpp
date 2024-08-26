// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/app/Game.hpp"

#if defined(__EMSCRIPTEN__)
    #include "emscripten.h"
#endif

#include <memory>

#include "tcob/app/Platform.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Stats.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/data/ConfigFile.hpp"
#include "tcob/gfx/RenderSystem.hpp"

namespace tcob {

constexpr milliseconds FIXED_FRAMES {1000.0f / 50.0f};
constexpr u8           MAX_FRAME_SKIP {10};

game::game(init const& gameInit)
{
    // init platform
    init i {gameInit};
    if (!i.ConfigDefaults) {
        i.ConfigDefaults = get_config_defaults();
    }
    auto plt {register_service<platform>(std::make_shared<platform>(false, i))};
    plt->get_window().Title(gameInit.Name);

    // properties
    FrameLimit.Changed.connect([&](i32 value) {
        plt->get_config()[Cfg::Video::Name][Cfg::Video::frame_limit] = value;
        _frameLimit                                                  = milliseconds {1000.f / value};
    });
    FrameLimit = plt->get_config()[Cfg::Video::Name][Cfg::Video::frame_limit].as<i32>();
}

game::~game()
{
    _mainLibrary.destroy_all_groups();
    remove_service<platform>();
}

void game::start()
{
    milliseconds now {clock::now().time_since_epoch()};
    _nextFixedUpdate = now;
    _lastUpdate      = now;

    Start();
    on_start();
#if !defined(__EMSCRIPTEN__)
    loop();
#else
    emscripten_set_main_loop_arg([](void* ptr) { reinterpret_cast<game*>(ptr)->step(); }, this, 0, true);
#endif

    finish();
}

void game::finish()
{
    // wait for command queue
    auto& tm {locate_service<task_manager>()};
    while (tm.process_queue() == queue_status::Running) { // TODO: abort?
        std::this_thread::yield();
    }

    // pop all scenes
    while (!_scenes.empty()) {
        pop_scene();
    }

    Finish();
    on_finish();
}

void game::push_scene(std::shared_ptr<scene> const& scene)
{
    auto command {[&, scene]() {
        if (!_scenes.empty()) {
            _scenes.top()->sleep();
        }
        if (scene) {
            scene->start();
            _scenes.push(scene);
        }

        return queue_status::Finished;
    }};
    locate_service<task_manager>().run_deferred(std::move(command));
}

void game::pop_current_scene()
{
    if (!_scenes.empty()) {
        locate_service<task_manager>().run_deferred({[&]() {
            pop_scene();
            return queue_status::Finished;
        }});
    }
}

void game::queue_finish()
{
    _shouldQuit = true;
}

void game::pop_scene()
{
    if (_scenes.empty()) { return; }

    _scenes.top()->finish();
    _scenes.pop();
    if (!_scenes.empty()) {
        _scenes.top()->wake_up();
    }
}

void game::loop()
{
    do {
        step();
    } while (!_shouldQuit);
}

void game::step()
{
    auto& plt {locate_service<platform>()};
    auto& tm {locate_service<task_manager>()};

    if (!plt.process_events()) { queue_finish(); }

    tm.process_queue();

    if (_scenes.empty()) { queue_finish(); }

    // fixed update
    u8 fixedUpdateLoops {0};
    while (clock::now().time_since_epoch() > _nextFixedUpdate && fixedUpdateLoops < MAX_FRAME_SKIP) {
        FixedUpdate(FIXED_FRAMES);
        _nextFixedUpdate += FIXED_FRAMES;
        fixedUpdateLoops++;
    }

    milliseconds now {clock::now().time_since_epoch()};

    milliseconds const deltaUpdate {now - _lastUpdate};
    if (deltaUpdate >= _frameLimit) {
        // update
        PreUpdate(deltaUpdate);
        Update(deltaUpdate);
        PostUpdate(deltaUpdate);
        _lastUpdate = now;

        // render
        if (plt.has_window()) {
            auto& window {plt.get_window()};
            auto& dft {plt.get_default_target()};
            dft.set_size(window.Size());
            window.clear();
            Draw(window);
            window.draw_to(dft);
            window.swap_buffer();

            if (window.Cursor()) {
                window.Cursor->ActiveMode = "default"; // set cursor to default mode if available
                window.Cursor->update(deltaUpdate);
            }
        }

        locate_service<gfx::render_system>().get_stats().update(deltaUpdate);
    }
}

auto game::get_config_defaults() const -> data::config::object
{
    // set defaults
    data::config::object video {};
    video[Cfg::Video::fullscreen]             = true;
    video[Cfg::Video::use_desktop_resolution] = true;
    video[Cfg::Video::vsync]                  = false;
    video[Cfg::Video::frame_limit]            = 6000;
#if defined(__EMSCRIPTEN__)
    video[Cfg::Video::render_system] = "OPENGLES30";
#else
    video[Cfg::Video::render_system] = "OPENGL45";
#endif

    data::config::object defaults {};
    defaults[Cfg::Video::Name] = video;
    return defaults;
}

auto game::get_library() -> assets::library&
{
    return _mainLibrary;
}

////////////////////////////////////////////////////////////
}
