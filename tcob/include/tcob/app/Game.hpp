// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <stack>

#include "tcob/app/Scene.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

//! Represents a game instance.
//!
//! The `game` class provides a framework for creating and managing game instances. It handles
//! scene management, rendering, updates, and event handling.
class TCOB_API game : public non_copyable {
public:
    //! Initialization parameters for the game.
    struct init {
        path Path {};                   //!< The path to the game.
        path Name {};                   //!< The name of the game.
        path ConfigFile {"config.ini"}; //!< The configuration file name.
        path OrgName {"tcob"};          //!< The organization name.
        path LogFile {"tcob.log"};      //!< The log file name.
        u32  AsyncLoadThreads {0};      //!< The number of concurrent asynchronous threads.
    };

    //! Constructs a game instance with the specified initialization parameters.
    //! @param init The initialization parameters for the game.
    explicit game(init const& init);

    //! Destructor for the game instance.
    virtual ~game();

    signal<>                   Start;       //!< Signal emitted when the game starts.
    signal<>                   Finish;      //!< Signal emitted when the game finishes.
    signal<milliseconds const> FixedUpdate; //!< Signal emitted at a fixed time interval.
    signal<milliseconds const> PreUpdate;   //!< Signal emitted before the main update.
    signal<milliseconds const> Update;      //!< Signal emitted during the main update.
    signal<milliseconds const> PostUpdate;  //!< Signal emitted after the main update.
    signal<gfx::render_target> Draw;        //!< Signal emitted when rendering is required.
    signal<path const>         DropFile;    //!< Signal emitted when a file is dropped onto the game window.

    prop<f32> FrameLimit;                   //!< Property to control the frame rate limit.

    //! Retrieves the game's window instance.
    //! @return A reference to the game's window object.
    auto get_window() const -> gfx::window&;

    //! Starts the game.
    void start();

    //! Pushes a new scene onto the scene stack.
    //! @tparam T The type of the scene to be pushed.
    template <std::derived_from<scene> T>
    void push_scene();

    //! Pushes a shared scene onto the scene stack.
    //! @param scene The shared pointer to the scene to be pushed.
    void push_scene(std::shared_ptr<scene> const& scene);

    //! Pops the current scene from the scene stack.
    void pop_current_scene();

    //! Requests to finish the game.
    void queue_finish();

    //! Retrieves the default configuration settings for the game.
    //! @return An instance of `data::config::object` containing default configuration values.
    auto virtual get_config_defaults() const -> data::config::object;

protected:
    //! Called when the game finishes.
    void finish();

    //! Hook method called on game start.
    void virtual on_start() { }

    //! Hook method called on game finish.
    void virtual on_finish() { }

private:
    //! Sets up the rendering system.
    void setup_rendersystem();

    //! Main game loop.
    void loop();

    //! Handles the key down event.
    //! @param ev The keyboard event data.
    void on_key_down(input::keyboard::event& ev);

    //! Pops the top scene from the scene stack.
    void pop_scene();

    std::unique_ptr<gfx::window>                _window;        //!< The game's window instance. TODO: move to platform?
    utf8_string                                 _name {};       //!< The name of the game.
    milliseconds                                _frameLimit {}; //!< Frame rate limit.
    std::stack<std::shared_ptr<scene>>          _scenes {};     //!< Stack of active scenes.
    std::unique_ptr<gfx::default_render_target> _defaultTarget; //!< Default render target.

    bool _shouldQuit {false};                                   //!< Flag indicating if the game should quit.
};

}

#include "Game.inl"
