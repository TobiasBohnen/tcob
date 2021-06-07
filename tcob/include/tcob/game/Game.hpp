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
#include <tcob/gfx/gl/GLContext.hpp>
#include <tcob/thirdparty/sigslot/signal.hpp>

namespace tcob {
class FPSCounter {
    static constexpr i32 FRAME_VALUES = 100;

public:
    FPSCounter();

    void run();
    void reset();

    auto average_fps() const -> f32;
    auto best_fps() const -> f32;
    auto worst_fps() const -> f32;

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
    Game(const std::string& path, const std::string& name);
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

    void start();

    template <typename T>
    void push_scene();
    void push_scene(std::shared_ptr<Scene> scene);
    void pop_current_scene();

    auto audio() const -> AudioSystem&;
    auto config() -> Config&;
    auto input() const -> Input&;
    auto resources() const -> ResourceLibrary&;
    auto stats() -> FPSCounter&;
    auto window() const -> gl::Window&;

protected:
    virtual void on_config_defaults();
    virtual void on_quit();

private:
    void create_context();
    void loop();
    void process_events();
    void pop_scene();

    std::string _name {};

    Config _config {};
    FPSCounter _fps {};
    std::unique_ptr<Input> _input {};
    std::unique_ptr<ResourceLibrary> _resources {};
    std::unique_ptr<gl::Window> _window {};
    std::unique_ptr<AudioSystem> _audio {};
    std::unique_ptr<gl::Context> _context {};

    std::stack<std::shared_ptr<Scene>> _scenes {};
    std::queue<std::function<void()>> _commandQueue {};
};

template <typename T>
inline void Game::push_scene()
{
    push_scene(std::make_shared<T>(*this));
}

}