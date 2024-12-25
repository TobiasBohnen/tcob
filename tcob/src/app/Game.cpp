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
#include "tcob/data/ConfigConversions.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderSystem.hpp"

namespace tcob {

constexpr milliseconds FIXED_FRAMES {1000.0f / 50.0f};
constexpr u8           MAX_FRAME_SKIP {10};

game::game(init const& gameInit)
{
    // init platform
    init i {gameInit};
    if (!i.ConfigDefaults) {
        data::config::object obj;
        obj[Cfg::Video::Name] = gfx::video_config {};
        i.ConfigDefaults      = obj;
    }
    auto plt {register_service<platform>(std::make_shared<platform>(false, i))};

    // properties
    FrameLimit.Changed.connect([&](i32 value) {
        plt->config()[Cfg::Video::Name][Cfg::Video::frame_limit] = value;
        _frameLimit                                              = milliseconds {1000.f / value};
    });
    FrameLimit = plt->config()[Cfg::Video::Name][Cfg::Video::frame_limit].as<i32>();

    locate_service<input::system>().KeyDown.connect<&game::on_key_down>(this);
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
    while (!tm.process_queue()) { // TODO: abort?
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
    auto command {[&, scene](def_task& ctx) {
        if (!_scenes.empty()) {
            _scenes.top()->sleep();
        }
        if (scene) {
            scene->start();
            _scenes.push(scene);
        }

        ctx.Finished = true;
    }};
    locate_service<task_manager>().run_deferred(std::move(command));
}

void game::pop_current_scene()
{
    if (!_scenes.empty()) {
        locate_service<task_manager>().run_deferred({[&](def_task& ctx) {
            pop_scene();
            ctx.Finished = true;
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

    milliseconds const now {clock::now().time_since_epoch()};
    milliseconds const deltaUpdate {now - _lastUpdate};

    if (deltaUpdate >= _frameLimit) {
        // update
        PreUpdate(deltaUpdate);
        Update(deltaUpdate);
        PostUpdate(deltaUpdate);
        _lastUpdate = now;

        // render
        if (has_service<gfx::render_system>()) {
            auto& rs {locate_service<gfx::render_system>()};
            auto& window {rs.get_window()};

            window.clear();
            Draw(window);
            window.draw_to(rs.default_target());
            window.swap_buffer();

            if (window.Cursor()) {
                window.Cursor->ActiveMode = "default"; // set cursor to default mode if available
                window.Cursor->update(deltaUpdate);
            }

            rs.stats().update(deltaUpdate);
        }
    }
}

auto game::get_library() -> assets::library&
{
    return _mainLibrary;
}

void game::on_key_down(input::keyboard::event const& ev)
{
    using namespace tcob::enum_ops;
    if (!ev.Repeat) {
        // Alt+Enter -> toggle fullscreen
        if (ev.ScanCode == input::scan_code::RETURN && (ev.KeyMods & input::key_mod::LeftAlt) == input::key_mod::LeftAlt) {
            auto& window {locate_service<gfx::render_system>().get_window()};
            window.FullScreen = !window.FullScreen();
        }
    }
}

////////////////////////////////////////////////////////////
}
