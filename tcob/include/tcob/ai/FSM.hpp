// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <unordered_map>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/UserObject.hpp"

namespace tcob::ai {
////////////////////////////////////////////////////////////

class TCOB_API fsm final : public non_copyable {
public:
    using condition_func = std::function<bool(user_object const&)>;
    using action_func    = std::function<void(user_object&)>;
    using tick_func      = std::function<void(user_object&, milliseconds)>;

    struct transition {
        uid            TargetStateID {INVALID_ID};
        condition_func Condition {};
        action_func    OnTransition {};
    };

    struct state {
        uid         ID {INVALID_ID};
        action_func OnEnter {};
        action_func OnExit {};
        tick_func   OnUpdate {};

        std::vector<transition> Transitions {};
    };

    struct transition_event {
        uid From;
        uid To;
    };
    signal<transition_event const> StateChanged;

    void add_state(state const& s);
    void add_global_transition(transition const& t);

    void start(uid initialStateID, user_object data);
    void stop();
    void update(milliseconds dt);

    void force_state(uid stateID);

    auto current_state() const -> uid;
    auto previous_state() const -> uid;
    auto time_in_state() const -> milliseconds;
    auto is_running() const -> bool;

    template <typename T>
    auto data() -> T*;
    template <typename T>
    auto data() const -> T const*;

private:
    auto find_state(uid id) const -> state const*;
    void enter_state(uid id);
    void exit_current_state();

    void apply_transition(transition const& t);

    std::unordered_map<uid, state> _states {};
    std::vector<transition>        _globalTransitions {};

    uid          _current {INVALID_ID};
    uid          _previous {INVALID_ID};
    milliseconds _timeInState {0};

    user_object _data {};
    bool        _running {false};
};

////////////////////////////////////////////////////////////

template <typename T>
inline auto fsm::data() -> T*
{
    return _data.get<T>();
}

template <typename T>
inline auto fsm::data() const -> T const*
{
    return _data.get<T>();
}

}
