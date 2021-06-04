// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <functional>
#include <queue>
#include <stack>

#include <tcob/core/Input.hpp>
#include <tcob/game/Config.hpp>
#include <tcob/thirdparty/sigslot/signal.hpp>

namespace tcob {
class FPSCounter {
    static constexpr i32 FRAME_VALUES = 100;

public:
    FPSCounter();

    void run();
    void reset();

    auto average() const -> f32;
    auto best() const -> f32;
    auto worst() const -> f32;

private:
    std::array<u32, FRAME_VALUES> _frametimes {};
    u32 _frametimelast;
    u64 _framecount { 0 };

    f32 _averageFrames { 0 };
    f32 _worstFrames { std::numeric_limits<f32>::max() };
    f32 _bestFrames { 0 };
};

////////////////////////////////////////////////////////////

class Game {
public:
    Game(i32 argc, char* argv[], const std::string& name, bool createwindow = true);
    virtual ~Game();

    Game(const Game&) = delete;
    auto operator=(const Game& other) -> Game& = delete;

    Game(const Game&&) = delete;
    auto operator=(const Game&& other) -> Game& = delete;

    sigslot::signal<> PreMainLoop;
    sigslot::signal<> PostMainLoop;
    sigslot::signal<f64> FixedUpdate;
    sigslot::signal<f64> PreUpdate;
    sigslot::signal<f64> Update;
    sigslot::signal<f64> PostUpdate;
    sigslot::signal<gl::RenderTarget&> Draw;
    sigslot::signal<> Quit;

    template <typename T>
    void start();

    void push_scene(std::shared_ptr<Scene> scene);
    void pop_current_scene();

    auto audio() const -> AudioSystem&;
    auto fps() -> FPSCounter&;
    auto input() const -> Input&;
    auto resource_library() const -> ResourceLibrary&;
    auto window() const -> gl::Window&;

private:
    void setup_window();
    void loop();
    void on_quit();
    void process_events();
    void pop_scene();

    std::string _name;

    Config _config;
    FPSCounter _fps {};
    std::unique_ptr<Input> _input;
    std::unique_ptr<ResourceLibrary> _resources {};
    std::unique_ptr<gl::Window> _window {};
    std::unique_ptr<AudioSystem> _audio {};

    std::stack<std::shared_ptr<Scene>> _scenes {};
    std::queue<std::function<void()>> _commandQueue {};
};

template <typename T>
void Game::start()
{
    push_scene(std::make_unique<T>(*this));
    loop();
}

}