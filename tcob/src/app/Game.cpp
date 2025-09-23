// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/app/Game.hpp"

#if defined(__EMSCRIPTEN__)
    #include "emscripten.h"
#endif

#include <cassert>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

#include "tcob/app/Platform.hpp"
#include "tcob/app/Scene.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/assets/AssetLibrary.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"
#include "tcob/data/ConfigConversions.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/Stats.hpp"

namespace tcob {

constexpr milliseconds FIXED_FRAMES {1000.0f / 50.0f};
constexpr u8           MAX_FRAME_SKIP {10};

game::game(init const& gameInit)
{
    if (has_service<platform>()) { throw std::runtime_error("Only one instance of game is allowed at a time."); }

    // init platform
    init i {gameInit};
    if (!i.ConfigDefaults) {
        data::object obj;
        obj[Cfg::Video::Name] = gfx::video_config {};
        i.ConfigDefaults      = obj;
    }

    register_service<platform>(platform::Init(i));
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
    while (!tm.process_queue(milliseconds {9999}, true)) {
        std::this_thread::yield();
    }

    // pop all scenes
    while (!_scenes.empty()) {
        do_pop_current_scene();
    }

    Finish();
    on_finish();
}

void game::push_scene(std::shared_ptr<scene> const& scene)
{
    auto const command {[&, scene](def_task const& ctx) {
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
        locate_service<task_manager>().run_deferred({[this](def_task const& ctx) {
            do_pop_current_scene();
            ctx.Finished = true;
        }});
    }
}

void game::queue_finish()
{
    _forceQuit = true;
}

void game::do_pop_current_scene()
{
    if (_scenes.empty()) { return; }

    _scenes.top()->finish();
    _scenes.pop();
    if (!_scenes.empty()) {
        _scenes.top()->wake_up();
    }
}

auto game::should_quit() const -> bool
{
    return _forceQuit || _scenes.empty();
}

void game::loop()
{
    do {
        step();
    } while (!should_quit());
}

void game::step()
{
    auto& plt {locate_service<platform>()};
    auto& tm {locate_service<task_manager>()};

    if (!plt.process_events()) { queue_finish(); }

    if (plt.window_frozen()) {
        _lastUpdate      = clock::now().time_since_epoch();
        _nextFixedUpdate = _lastUpdate + FIXED_FRAMES;
    }

    // fixed update
    u8 fixedUpdateLoops {0};
    while (clock::now().time_since_epoch() > _nextFixedUpdate && fixedUpdateLoops < MAX_FRAME_SKIP) {
        FixedUpdate(FIXED_FRAMES);
        _nextFixedUpdate += FIXED_FRAMES;
        fixedUpdateLoops++;
    }

    milliseconds const now {clock::now().time_since_epoch()};
    milliseconds const deltaUpdate {now - _lastUpdate};
    milliseconds const frameLimit {1000.0f / static_cast<f32>(plt.FrameLimit)};

    tm.process_queue(deltaUpdate, false);

    if (deltaUpdate >= frameLimit) {
        // update
        PreUpdate(deltaUpdate);
        Update(deltaUpdate);
        PostUpdate(deltaUpdate);
        _lastUpdate = now;

        // render
        if (has_service<gfx::render_system>()) {
            auto& rs {locate_service<gfx::render_system>()};
            auto& window {rs.window()};

            window.clear();
            Draw(window);
            window.draw_to(rs.default_target());
            window.swap_buffer();

            if (*window.Cursor) {
                window.Cursor->update(deltaUpdate);
                window.Cursor->ActiveMode = "default"; // set cursor to default mode if available
            }

            rs.statistics().update(deltaUpdate);
        }
    }
}

auto game::library() -> assets::library&
{
    return _mainLibrary;
}

void game::on_key_down(input::keyboard::event const& ev)
{
    using namespace tcob::enum_ops;
    if (!ev.Repeat) {
        // Alt+Enter -> toggle fullscreen
        if (ev.ScanCode == input::scan_code::RETURN && ev.KeyMods.left_alt()) {
            auto& window {locate_service<gfx::render_system>().window()};
            window.FullScreen = !window.FullScreen;
        }
    }
}

////////////////////////////////////////////////////////////
}
