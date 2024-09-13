// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <stack>

#include "tcob/app/Scene.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/assets/AssetLibrary.hpp"
#include "tcob/data/ConfigTypes.hpp"

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
        path                                Path {};                      //!< The path to the game.
        path                                Name {};                      //!< The name of the game.
        path                                OrgName {"tcob"};             //!< The organization name.
        path                                LogFile {"tcob.log"};         //!< The log file name.
        path                                ConfigFile {"config.ini"};    //!< The configuration file name.
        std::optional<data::config::object> ConfigDefaults {std::nullopt};
        std::optional<i32>                  WorkerThreads {std::nullopt}; //!< The number of concurrent asynchronous threads.
    };

    //! Constructs a game instance with the specified initialization parameters.
    //! @param gameInit The initialization parameters for the game.
    explicit game(init const& gameInit);

    //! Destructor for the game instance.
    virtual ~game();

    signal<>                   Start;       //!< Signal emitted when the game starts.
    signal<>                   Finish;      //!< Signal emitted when the game finishes.
    signal<milliseconds const> FixedUpdate; //!< Signal emitted at a fixed time interval.
    signal<milliseconds const> PreUpdate;   //!< Signal emitted before the main update.
    signal<milliseconds const> Update;      //!< Signal emitted during the main update.
    signal<milliseconds const> PostUpdate;  //!< Signal emitted after the main update.
    signal<gfx::render_target> Draw;        //!< Signal emitted when rendering is required.

    prop<i32> FrameLimit;                   //!< Property to control the frame rate limit.

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

    auto get_library() -> assets::library&;

protected:
    //! Called when the game finishes.
    void finish();

    //! Hook method called on game start.
    void virtual on_start() { }

    //! Hook method called on game finish.
    void virtual on_finish() { }

private:
    //! Main game loop.
    void loop();
    void step();

    //! Pops the top scene from the scene stack.
    void pop_scene();

    void on_key_down(input::keyboard::event& ev);

    assets::library _mainLibrary {};

    milliseconds                       _frameLimit {}; //!< Frame rate limit.
    std::stack<std::shared_ptr<scene>> _scenes {};     //!< Stack of active scenes.

    bool         _shouldQuit {false};                  //!< Flag indicating if the game should quit.
    milliseconds _nextFixedUpdate {};
    milliseconds _lastUpdate {};
};

}

#include "Game.inl"
