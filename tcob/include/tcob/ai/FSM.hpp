// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <vector>

#include "tcob/core/UserObject.hpp"

namespace tcob::ai {
////////////////////////////////////////////////////////////

class TCOB_API fsm final {
public:
    using condition_func = std::function<bool(user_object const&)>;
    using action_func    = std::function<void(user_object&)>;
    using tick_func      = std::function<void(user_object&, milliseconds)>;

    struct state {
        uid         ID {};
        string      Title {};
        action_func OnEnter {};
        action_func OnExit {};
        tick_func   OnTick {};
    };

    struct transition {
        uid            ID {};
        uid            FromID {};
        uid            ToID {};
        condition_func Condition {};
        action_func    OnTransition {};
    };

    void add_state(state const& s);
    void add_transition(transition const& t);

    void start(uid initalStateID, user_object data = {});
    void stop();
    void tick(milliseconds dt);

    auto current_state() const -> uid;
    auto is_running() const -> bool;
    auto data() const -> user_object const&;
    auto data() -> user_object&;

private:
    auto find_state(uid id) const -> state const*;
    void enter_state(uid id);
    void exit_state(uid id);

    std::vector<state>      _states {};
    std::vector<transition> _transitions {};
    uid                     _current {};
    user_object             _data {};
    bool                    _running {false};
};

}
