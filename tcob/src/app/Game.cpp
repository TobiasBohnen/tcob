// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/app/Game.hpp"

#include <memory>

#include "tcob/app/Platform.hpp"
#include "tcob/core/CommandQueue.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Stats.hpp"
#include "tcob/data/ConfigFile.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/RenderSystem.hpp"

namespace tcob {

constexpr milliseconds FIXED_FRAMES {1000.0f / 50.0f};
constexpr u8           MAX_FRAME_SKIP {10};

game::game(init const& init)
{
    // init platform
    auto plt {register_service<platform>(std::make_shared<platform>(this, init))};
    plt->get_window().Title(init.Name);

    // properties
    FrameLimit.Changed.connect([&](i32 value) {
        plt->get_config()[Cfg::Video::Name][Cfg::Video::frame_limit] = value;
        _frameLimit                                                  = milliseconds {1000.f / value};
    });
    FrameLimit = plt->get_config()[Cfg::Video::Name][Cfg::Video::frame_limit].as<i32>();
}

game::~game()
{
    remove_service<platform>();
}

void game::start()
{
    on_start();
    loop();
    Finish();
    finish();
}

void game::finish()
{
    // wait for command queue
    auto& cq {locate_service<command_queue>()};
    while (!cq.is_empty()) {
        cq.process();
    }

    // pop all scenes
    while (!_scenes.empty()) {
        pop_scene();
    }

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

        return command_status::Finished;
    }};
    locate_service<command_queue>().add(std::move(command));
}

void game::pop_current_scene()
{
    if (!_scenes.empty()) {
        locate_service<command_queue>().add({[&]() {
            pop_scene();
            return command_status::Finished;
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
    milliseconds now {clock::now().time_since_epoch()};
    milliseconds nextFixedUpdate {now};
    milliseconds lastUpdate {now};

    Start();

    auto& plt {locate_service<platform>()};

    do {
        plt.process_events();
        locate_service<command_queue>().process();

        if (_scenes.empty()) {
            queue_finish();
        }

        // fixed update
        u8 fixedUpdateLoops {0};
        while (clock::now().time_since_epoch() > nextFixedUpdate && fixedUpdateLoops < MAX_FRAME_SKIP) {
            FixedUpdate(FIXED_FRAMES);
            nextFixedUpdate += FIXED_FRAMES;
            fixedUpdateLoops++;
        }

        now = clock::now().time_since_epoch();

        milliseconds const deltaUpdate {now - lastUpdate};
        if (deltaUpdate >= _frameLimit) {
            // update
            PreUpdate(deltaUpdate);
            Update(deltaUpdate);
            PostUpdate(deltaUpdate);
            lastUpdate = now;

            // render
            if (plt.has_window()) {
                auto& window {plt.get_window()};
                if (window.Cursor()) {
                    window.Cursor()->ActiveMode = "default"; // set cursor to default mode if available
                }

                auto& dft {plt.get_default_target()};
                dft.set_size(window.Size());
                window.clear();
                Draw(window);
                window.draw_to(dft);
                window.swap_buffer();

                if (window.Cursor()) {
                    window.Cursor->update(deltaUpdate);
                }
            }

            locate_service<gfx::render_system>().get_stats().update(deltaUpdate);
        }

    } while (!_shouldQuit);
}

auto game::get_config_defaults() const -> data::config::object
{
    // set defaults
    data::config::object video {};
    video[Cfg::Video::fullscreen]             = true;
    video[Cfg::Video::use_desktop_resolution] = true;
    video[Cfg::Video::vsync]                  = false;
    video[Cfg::Video::frame_limit]            = 6000;
    video[Cfg::Video::render_system]          = "OPENGL45";

    data::config::object defaults {};
    defaults[Cfg::Video::Name] = video;
    return defaults;
}

////////////////////////////////////////////////////////////
}
