// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <optional>
#include <stack>

#include "tcob/app/Scene.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/assets/AssetLibrary.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

class TCOB_API game : public non_copyable {
public:
    struct init {
        path                        Name {};                      //!< The name of the game.
        path                        OrgName {"tcob"};             //!< The organization name.
        path                        LogFile {"tcob.log"};         //!< The log file name.
        path                        ConfigFile {"config.ini"};    //!< The configuration file name.
        std::optional<data::object> ConfigDefaults {std::nullopt};
        std::optional<isize>        WorkerThreads {std::nullopt}; //!< The number of concurrent asynchronous threads.
    };

    explicit game(init const& gameInit);

    virtual ~game();

    signal<>                   Start;       //!< Signal emitted when the game starts.
    signal<>                   Finish;      //!< Signal emitted when the game finishes.
    signal<milliseconds const> FixedUpdate; //!< Signal emitted at a fixed time interval.
    signal<milliseconds const> PreUpdate;   //!< Signal emitted before the main update.
    signal<milliseconds const> Update;      //!< Signal emitted during the main update.
    signal<milliseconds const> PostUpdate;  //!< Signal emitted after the main update.
    signal<gfx::render_target> Draw;        //!< Signal emitted when rendering is required.

    void start();

    template <std::derived_from<scene> T>
    void push_scene();

    void push_scene(std::shared_ptr<scene> const& scene);

    void pop_current_scene();

    void queue_finish();

    auto library() -> assets::library&;

protected:
    void finish();

    void virtual on_start() { }

    void virtual on_finish() { }

private:
    void loop();
    void step();

    void pop_scene();

    void on_key_down(input::keyboard::event const& ev);

    assets::library _mainLibrary {};

    std::stack<std::shared_ptr<scene>> _scenes {};

    bool         _shouldQuit {false};
    milliseconds _nextFixedUpdate {};
    milliseconds _lastUpdate {};
};

}

#include "Game.inl"
