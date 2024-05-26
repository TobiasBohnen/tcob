// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/app/Game.hpp"

#include <memory>

#include "tcob/app/Platform.hpp"
#include "tcob/core/CommandQueue.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Stats.hpp"
#include "tcob/data/ConfigFile.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob {

constexpr milliseconds FIXED_FRAMES {1000.0f / 50.0f};
constexpr u8           MAX_FRAME_SKIP {10};

game::game(init const& init)
    : _name {init.Name}
{
    // init platform
    register_service<platform>(std::make_shared<platform>(this, init));

    // properties
    FrameLimit.Changed.connect([&](i32 value) { _frameLimit = milliseconds {1000.f / value}; });

    setup_rendersystem();
    locate_service<input::system>().KeyDown.connect<&game::on_key_down>(this);
}

game::~game()
{
    locate_service<platform>().remove_services();

    _window = nullptr;

    remove_service<platform>();
}

auto game::get_window() const -> gfx::window&
{
    return *_window;
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

    do {
        locate_service<platform>().process_events(_window.get());
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
            if (_window) {
                if (_window->Cursor()) {
                    _window->Cursor()->ActiveMode = "default"; // set cursor to default mode if available
                }

                _defaultTarget->set_size(_window->get_size());
                _window->clear();
                Draw(*_window);
                _window->draw_to(*_defaultTarget);
                _window->swap_buffer();

                if (_window->Cursor()) {
                    _window->Cursor->update(deltaUpdate);
                }
            }

            locate_service<stats>().update(deltaUpdate);
        }

    } while (!_shouldQuit);
}

void game::on_key_down(input::keyboard::event& ev)
{
    using namespace tcob::enum_ops;
    if (!ev.Repeat) {
        // Alt+Enter -> toggle fullscreen
        if (ev.ScanCode == input::scan_code::RETURN && (ev.KeyMods & input::key_mod::LeftAlt) == input::key_mod::LeftAlt) {
            _window->FullScreen = !_window->FullScreen();
        }
    }
}

void game::setup_rendersystem()
{
    auto& config {locate_service<data::config_file>()};

    // get config
    auto const video {config[Cfg::Video::Name].as<data::config::object>()};

    size_i const resolution {video[Cfg::Video::use_desktop_resolution].as<bool>()
                                 ? locate_service<platform>().get_display_size(0)
                                 : video[Cfg::Video::resolution].as<size_i>()};

    FrameLimit.Changed.connect([&](i32 value) {
        config[Cfg::Video::Name][Cfg::Video::frame_limit] = value;
    });
    FrameLimit = video[Cfg::Video::frame_limit].as<i32>();

    auto& renderSystem {locate_service<gfx::render_system>()};

    // create window (and context)
    _window = std::unique_ptr<gfx::window> {new gfx::window(renderSystem.create_window(resolution))};

    _window->FullScreen.Changed.connect([&](bool value) { config[Cfg::Video::Name][Cfg::Video::fullscreen] = value; });
    _window->VSync.Changed.connect([&](bool value) { config[Cfg::Video::Name][Cfg::Video::vsync] = value; });
    _window->Size.Changed.connect([&](size_i value) {
        config[Cfg::Video::Name][Cfg::Video::use_desktop_resolution] = value == locate_service<platform>().get_display_size(0);
        config[Cfg::Video::Name][Cfg::Video::resolution]             = value;
    });

    _window->FullScreen(video[Cfg::Video::fullscreen].as<bool>());
    _window->VSync(video[Cfg::Video::vsync].as<bool>());
    _window->Size(resolution);
    _window->Title(_name);

    _defaultTarget = std::make_unique<gfx::default_render_target>();

    _window->clear();
    _window->draw_to(*_defaultTarget);
    _window->swap_buffer();
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
