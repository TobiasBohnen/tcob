// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/game/Game.hpp>

#include <SDL2/SDL.h>
#include <chrono>

#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/game/Scene.hpp>
#include <tcob/gfx/Canvas.hpp>
#include <tcob/gfx/gl/GLCapabilities.hpp>
#include <tcob/gfx/gl/GLWindow.hpp>
#include <tcob/sfx/AudioSystem.hpp>

using hrc = std::chrono::high_resolution_clock;

namespace tcob {
constexpr f32 FIXED_TIME_STEPS { 1000.f / 50.f };
constexpr u8 MAX_FRAME_SKIP = 10;

Game::Game(const std::string& path, const std::string& name, bool createWindow)
    : _name { name }
{
    SDL_Init(SDL_INIT_EVERYTHING);
    FileSystem::init(path.c_str(), name);

    Log("starting");
    _config.load();

    _audio = std::make_unique<AudioSystem>();

    _input = std::make_unique<Input>();

    create_context(createWindow);
    gl::Capabilities::init();

    _resources = std::make_unique<ResourceLibrary>();

    Quit.connect(&Game::on_quit, this);
}

Game::~Game()
{
    _resources = nullptr;

    _window = nullptr;
    _context = nullptr;

    _input = nullptr;

    _audio = nullptr;

    _config.save();
    Log("exiting");

    FileSystem::done();
    SDL_Quit();
}

void Game::push_scene(std::shared_ptr<Scene> scene)
{
    auto command { [this, scene]() {
        if (!_scenes.empty()) {
            _scenes.top()->sleep();
        }
        if (scene) {
            scene->start();
            _scenes.push(scene);
        }
    } };
    _commandQueue.push(std::move(command));
}

void Game::pop_current_scene()
{
    if (!_scenes.empty()) {
        _commandQueue.push({ [this]() { pop_scene(); } });
    }
}

void Game::pop_scene()
{
    _scenes.top()->finish();
    _scenes.pop();
    if (!_scenes.empty()) {
        _scenes.top()->wake_up();
    }
}

auto Game::audio() const -> AudioSystem&
{
    return *_audio;
}

auto Game::stats() -> FPSCounter&
{
    return _fps;
}

auto Game::input() const -> Input&
{
    return *_input;
}

auto Game::resources() const -> ResourceLibrary&
{
    return *_resources;
}

auto Game::window() const -> gl::Window&
{
    return *_window;
}

void Game::loop()
{
    std::chrono::duration<f64, std::milli> now { hrc::now().time_since_epoch() };
    std::chrono::duration<f64, std::milli> nextTick { now };
    std::chrono::duration<f64, std::milli> last { now };
    const std::chrono::duration<f64, std::milli> fixedTimeSteps { FIXED_TIME_STEPS };

    PreMainLoop();

    do {
        process_events();

        while (!_commandQueue.empty()) {
            _commandQueue.front()();
            _commandQueue.pop();
        }

        _resources->update();

        // fixed update
        u8 loops { 0 };
        while (hrc::now().time_since_epoch() > nextTick && loops < MAX_FRAME_SKIP) {

            FixedUpdate(FIXED_TIME_STEPS);
            nextTick += fixedTimeSteps;
            loops++;
        }

        // update
        now = hrc::now().time_since_epoch();
        const f64 delta { (now - last).count() };
        last = now;

        PreUpdate(delta);
        Update(delta);
        PostUpdate(delta);

        // render
        if (_window) {
            _window->clear({ 0x33, 0x33, 0x33, 0 });
            Draw(*_window);
            _window->swap();
        }

        _fps.run();

    } while (!_scenes.empty());

    PostMainLoop();
}

void Game::on_quit()
{
    while (!_scenes.empty()) {
        pop_scene();
    }
}

void Game::process_events()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            Quit();
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_TEXTINPUT:
        case SDL_TEXTEDITING:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        case SDL_JOYAXISMOTION:
        case SDL_JOYHATMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
        case SDL_CONTROLLERAXISMOTION:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            _input->process_events(&ev);
            break;
        case SDL_WINDOWEVENT:
            _window->process_events(&ev);
            break;
        default:
            break;
        }
    }
}

void Game::create_context(bool createWindow)
{
    const bool fullscreen { _config["Video"]["FullScreen"] };
    const SizeU size { _config["Video"]["Resolution"] };
    const u32 aa { _config["Video"]["AntiAliasing"] };
    const bool vsync { _config["Video"]["VSync"] };

    _context = std::make_unique<gl::Context>(PointU { SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED }, size, aa);
    if (createWindow) {
        _window = std::make_unique<gl::Window>(_context->window_handle());

        _window->settings({

            .Fullscreen = fullscreen,
            .VSync = vsync,
            .Title = _name });
    }
}

////////////////////////////////////////////////////////////

FPSCounter::FPSCounter()
    : _frametimelast { SDL_GetTicks() }
{
}

void FPSCounter::run()
{
    u64 count { 0 };

    const u64 frametimesindex { _framecount % FRAME_VALUES };
    const u32 getticks { SDL_GetTicks() };
    _frametimes[frametimesindex] = getticks - _frametimelast;
    _frametimelast = getticks;

    ++_framecount;
    if (_framecount < FRAME_VALUES) {
        count = _framecount;
    } else {
        count = FRAME_VALUES;
    }

    _averageFrames = 0;
    for (u64 i { 0 }; i < count; ++i) {
        _averageFrames += _frametimes[i];
    }
    _averageFrames /= count;
    _averageFrames = 1000.f / _averageFrames;

    if (count == FRAME_VALUES) {
        _bestFrames = std::max(_bestFrames, _averageFrames);
        _worstFrames = std::min(_worstFrames, _averageFrames);
    }
}

void FPSCounter::reset()
{
    _averageFrames = 0;
    _worstFrames = std::numeric_limits<f32>::max();
    _bestFrames = 0;

    _frametimelast = SDL_GetTicks();
    _framecount = 0;
}

auto FPSCounter::average_fps() const -> f32
{
    return _averageFrames;
}

auto FPSCounter::best_fps() const -> f32
{
    return _bestFrames;
}

auto FPSCounter::worst_fps() const -> f32
{
    return _worstFrames;
}
}